/* lab3/primes.c */
/* Copyright (c) 2018 J. M. Spivey */

#include "hardware.h"
#include "lib.h"

/* Pins to use for serial communication */
#define TX USB_TX
#define RX USB_RX
#define TICK 5                // One timer interrupt per 5 millisec (0.005 sec)

/* Characters that have been generated by printf and not yet output
are stored in a circular buffer. */

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

/* start_timer -- light an LED and start a timer */
void start_timer(void)
{
    TIMER0_MODE = TIMER_MODE_Timer;
    TIMER0_BITMODE = TIMER_BITMODE_32Bit;
    TIMER0_PRESCALER = 4; /* Count at 1MHz */
    TIMER0_START = 1;
}

/* stop_timer -- turn off LED and print timer result */
void stop_timer(void)
{
    TIMER0_CAPTURE[0] = 1;
    unsigned time1 = TIMER0_CC[0];
    printf("%d millisec\n", (time1+500)/1000);
}

/** Timer 1*/
static volatile unsigned numOut = 0; // number of outputs we have done so far
static volatile unsigned timeStepsPerOutput = 2; // 10 ms between samples
static volatile unsigned timeCounter = 0;
#define MAXOUT 1500
void timer1_handler(void) {
    if (TIMER1_COMPARE[0]) {
        if (!ADC_BUSY && numOut < MAXOUT + 1){ // If ADC is not busy, start a new conversion
            if (timeCounter >= timeStepsPerOutput){
                ADC_START = 1;
                timeCounter = 0;
                numOut++;
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
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_RES, ADC_RES_8Bit);
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_INPSEL, ADC_INPSEL_AIn_1_1);
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_REFSEL, ADC_REFSEL_BGap);
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_PSEL, AIN5 + AIN6 + AIN7); // cols 1 to 3 of the LED Array
    SET_FIELD(ADC_CONFIG, ADC_CONFIG_EXTREFSEL, ADC_EXTREFSEL_Ref0);
    ADC_ENABLE = 1;

    ADC_INTENSET = 1; // Enable ADC interrupt
    enable_irq(ADC_IRQ);
}
unsigned samples[MAXOUT];
unsigned bitstream[MAXOUT];
void adc_handler(void) {
    if (ADC_END) {
        ADC_END = 0;
        ADC_BUSY = 0;
        int result = ADC_RESULT;
        if (numOut < MAXOUT)
        samples[numOut] = result;
        // printf("ADC: %d\n", result);
        // printf("numOut: %d\n", numOut);
    }
}


void init(void)
{
    serial_init();
    delay_loop(10000);
    start_timer();
    init_timer();
    init_adc();
    while (numOut < MAXOUT){
        pause();
    }
    printf("Done Recording\n");

    static unsigned threshold;
    
    for (int i = 0; i < 10; i++){
        threshold += samples[i];
    }
    threshold /= 10;
    threshold -= 2;
    for (int i = 1; i < MAXOUT; i++){
        if (samples[i] > threshold){
            printf("1");
            bitstream[i] = 1;
        } else {
            printf("0");
            bitstream[i] = 0;
        }
    }
    printf("\n");

    // detect falling edge and determine if it is a start bit (next 3 or so bits are 0)
    
    int baud = 10; // 10 bits per second
    int timePerData = 1000 / baud; // 100 ms per data bit
    int dt = TICK * timeStepsPerOutput; // time between samples
    for (int i = 1; i < MAXOUT; i++){
        if (bitstream[i] == 0 && bitstream[i-1] == 1){ // falling edge
            int j = i + 1;
            int count = 0;
            // take the average of the next timePerdata / dt samples
            while (j < MAXOUT && (j - i) * dt < timePerData){
                count += bitstream[j];
                j++;
            }
            if (count <= timePerData / (2*dt)){
                printf("Start bit detected at %d\n: ", i);
                // move i to the middle of the start bit
                i = (i + j) / 2;
                // read the center of the next 7 data bits
                int start_index = i;

                char c = 0;
                for (int k=1; k <= 7; k++){
                    int dataCenterTime =  timePerData * k; // k*(10 ms) = time since the center of start bit
                    int dataCenterIndex = start_index + dataCenterTime/dt;
                    if (bitstream[dataCenterIndex] + bitstream[dataCenterIndex-1] +bitstream[dataCenterIndex+1] >= 2){
                        printf("1");
                        c += 1 << (7-k);
                    } else {
                        printf("0");
                    }
                }

                
                printf(" = %c\n", c);
                printf("\n");
                i += 8 * timePerData / dt;
            }
        }
    }
    
    
    stop_timer();
}