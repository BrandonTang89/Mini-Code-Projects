#include "hardware.h"
#include "lib.h"

/* Pins to use for serial communication */
#define TX USB_TX
#define RX USB_RX
#define NBUF 128                   /* Buffer size */

static volatile int txidle;       /* Whether UART is idle */
static volatile int bufcnt = 0;   /* Number of chars in buffer */
static unsigned bufin = 0;        /* Index of first free slot */
static unsigned bufout = 0;       /* Index of first occupied slot */
static volatile char txbuf[NBUF]; /* The buffer */

/* buf_put -- add character to buffer */
void buf_put(char ch)
{
    txbuf[bufin] = ch;
    bufcnt++;
    bufin = (bufin+1) % NBUF;
}

/* buf_get -- fetch character from buffer */
char buf_get(void)
{
    char ch = txbuf[bufout];
    bufcnt--;
    bufout = (bufout+1) % NBUF;
    return ch;
}

/* serial_init -- set up UART connection to host */
void serial_init(void)
{
    UART_ENABLE = UART_ENABLE_Disabled;
    UART_BAUDRATE = UART_BAUDRATE_9600; /* 9600 baud */
    UART_CONFIG = FIELD(UART_CONFIG_PARITY, UART_PARITY_None);
                                        /* format 8N1 */
    UART_PSELTXD = TX;                  /* choose pins */
    UART_PSELRXD = RX;
    UART_ENABLE = UART_ENABLE_Enabled;
    UART_TXDRDY = 0;
    UART_STARTTX = 1;

    /* Interrupt for transmit only */
    UART_INTENSET = BIT(UART_INT_TXDRDY);
    enable_irq(UART_IRQ);
    txidle = 1;
}

/* uart_handler -- interrupt handler for UART */
void uart_handler(void)
{
    if (UART_TXDRDY) {
        UART_TXDRDY = 0;
        if (bufcnt == 0)
            txidle = 1;
        else
            UART_TXD = buf_get();
    }
}

/* serial_putc -- send output character */
void serial_putc(char ch)
{
    while (bufcnt == NBUF) pause();

    intr_disable();
    if (txidle) {
        UART_TXD = ch;
        txidle = 0;
    } else {
        buf_put(ch);
    }
    intr_enable();
}

/* print_buf -- output routine for use by printf */
void print_buf(char *buf, int n)
{
    for (int i = 0; i < n; i++) {
        char c = buf[i];
        if (c == '\n') serial_putc('\r');
        serial_putc(c);
    }
}

/* start_timer -- Start a timer */
void start_timer(void)
{
    TIMER0_MODE = TIMER_MODE_Timer;
    TIMER0_BITMODE = TIMER_BITMODE_32Bit;
    TIMER0_PRESCALER = 4; /* Count at 1MHz */
    TIMER0_START = 1;
}

/* stop_timer -- Print timer result */
void stop_timer(void)
{
    TIMER0_CAPTURE[0] = 1;
    unsigned time1 = TIMER0_CC[0];
    printf("%d millisec\n", (time1+500)/1000);
}

/** Timer 1*/
#define TICK 5  // One timer interrupt per 5 millisec (0.005 sec)
#define BAUD 12 // 10 bits per second
#define SAMPLES_PER_BIT 7 // 100 samples per bit
#define SAMPLEBUFSIZE 1024
static volatile unsigned timeStepsPerOutput = (1000) / (BAUD * TICK * SAMPLES_PER_BIT);
static volatile unsigned timeCounter = 0;

static int volatile sampleBufIn = 0;
static int volatile sampleBufOut = 0;
static int volatile sampleBufCnt = 0;

void timer1_handler(void) {
    if (TIMER1_COMPARE[0]) {
        if (!ADC_BUSY){ // If ADC is not busy, start a new conversion
            if (timeCounter >= timeStepsPerOutput && sampleBufCnt < SAMPLEBUFSIZE){ // try to start new conversion whenever possible
                GPIO_OUT = 0x0ff0; // set rows low and cols high
                ADC_START = 1;
                timeCounter = 0;
                sampleBufIn++;
                if (sampleBufIn == SAMPLEBUFSIZE){
                    sampleBufIn = 0;
                }
                sampleBufCnt++;
            }
            timeCounter++;
        }
        TIMER1_COMPARE[0] = 0;
    }
}

void init_timer(void) {
    TIMER1_STOP = 1;
    TIMER1_MODE = TIMER_MODE_Timer;
    TIMER1_BITMODE = TIMER_BITMODE_16Bit;
    TIMER1_PRESCALER = 4;      // 1MHz = 16MHz / 2^4
    TIMER1_CLEAR = 1;
    TIMER1_CC[0] = 1000 * TICK;
    TIMER1_SHORTS = BIT(TIMER_COMPARE0_CLEAR);
    TIMER1_INTENSET = BIT(TIMER_INT_COMPARE0);
    TIMER1_START = 1;
    enable_irq(TIMER1_IRQ);
}

/** Enable the Analog to Digital Converter*/
#define AIN5 32
#define AIN6 64
#define AIN7 128
void init_adc(void) {
    GPIO_DIR = 0xfff0;

    SET_FIELD(ADC_CONFIG, ADC_CONFIG_RES, ADC_RES_8Bit);
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_INPSEL, ADC_INPSEL_AIn_1_1);
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_REFSEL, ADC_REFSEL_BGap);
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_PSEL, AIN5 + AIN6 + AIN7); // cols 1 to 3 of the LED Array
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_EXTREFSEL, ADC_EXTREFSEL_Ref0);
    ADC_ENABLE = 1;

    ADC_INTENSET = 1; // Enable ADC interrupt
    enable_irq(ADC_IRQ);
}


static volatile int threshold = 0;
static volatile unsigned samples[SAMPLEBUFSIZE];
static volatile unsigned bitstream[SAMPLEBUFSIZE];

void adc_handler(void) {
    if (ADC_END) {
        ADC_END = 0;
        ADC_BUSY = 0;
        int result = ADC_RESULT;
        samples[sampleBufIn] = result;
        if (result < threshold){ // if the result is bright
            bitstream[sampleBufIn] = 1;
        } else {
            bitstream[sampleBufIn] = 0;
        }
        // printf("ADC: %d\n", result);
    }
}


void init(void)
{
    serial_init();
    delay_loop(10000);
    start_timer();
    init_timer();
    init_adc();

    int timePerData = 1000 / BAUD; // in ms per data bit
    int dt = TICK * timeStepsPerOutput; // time between samples
    
    printf("Calculating threshold...");
    while (sampleBufCnt < SAMPLEBUFSIZE/2) pause();
    int maxi = 0;
    int mini = 255;
    for (int i = 10; i < SAMPLEBUFSIZE/2; i++){
        threshold += samples[i];
        if (samples[i] > maxi) maxi = samples[i];
        if (samples[i] < mini) mini = samples[i];
    }
    /*
    threshold /= 10;
    threshold += 2;
    */
    threshold = (maxi + mini) / 2;
    printf("Threshold: %d\n", threshold);

    // Reset the Buffer
    sampleBufCnt = 0;
    sampleBufOut = 0;
    sampleBufIn = 0;

    // detect falling edge and determine if it is a start bit (next 3 or so bits are 0)
    while (1){
        while (sampleBufCnt < 2) pause();
        int next = (sampleBufOut + 1)%SAMPLEBUFSIZE;
        // printf("%d %d %d %d %d\n", samples[sampleBufOut], samples[next], bitstream[sampleBufOut], bitstream[next], sampleBufCnt);
        int x = bitstream[sampleBufOut] == 1 && bitstream[next] == 0;
        
        intr_disable();
        sampleBufCnt--;
        intr_enable();
        sampleBufOut = next;
        // printf("%d", bitstream[sampleBufOut]);

        if (x){ // falling edge
            // printf("Falling edge detected\n");
            int j = next;
            int count = 0;
            while (sampleBufCnt < 2*SAMPLES_PER_BIT) pause(); 
            // take the average of the next timePerdata / dt samples
            for (int k = 0; k < SAMPLES_PER_BIT; k++){
                count += bitstream[j];
                j++;
                if (j == SAMPLEBUFSIZE) j = 0;
            }
            intr_disable();
            sampleBufCnt -= SAMPLES_PER_BIT / 2;
            intr_enable();
            sampleBufOut += SAMPLES_PER_BIT/2;

            if (count <= SAMPLEBUFSIZE/2 -1){ // start bit detected
                // printf("Start bit detected\n");
                // printf("bufcnt: %d\n", sampleBufCnt);
                while (sampleBufCnt < SAMPLES_PER_BIT*9) pause();
                // read the center of the next 7 data bits
                char c = 0;
                for (int k=1; k <= 7; k++){
                    int dataCenterIndex = sampleBufOut + (timePerData * k)/dt;
                    dataCenterIndex %= SAMPLEBUFSIZE;
                    if (bitstream[(dataCenterIndex - 1)%SAMPLEBUFSIZE] + bitstream[dataCenterIndex] + bitstream[(dataCenterIndex + 1)%SAMPLEBUFSIZE] >= 2){
                        printf("1");
                        c += 1 << (7-k);
                    } else {
                        printf("0");
                    }
                }

                printf(" = %c\n", c);
                sampleBufOut += 8 * SAMPLES_PER_BIT;
                if (sampleBufOut >= SAMPLEBUFSIZE) sampleBufOut -= SAMPLEBUFSIZE;
                intr_disable();
                sampleBufCnt -= 8 * SAMPLES_PER_BIT;
                intr_enable();
            }
        }
    }
    
    stop_timer();
}
