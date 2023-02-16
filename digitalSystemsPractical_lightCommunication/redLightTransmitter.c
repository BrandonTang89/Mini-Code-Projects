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
    UART_RXDRDY = 0;
    UART_STARTRX = 1;
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

/* serial_getc -- wait for input character and return it */
int serial_getc(void)
{
    while (! UART_RXDRDY) { }
    char ch = UART_RXD;
    UART_RXDRDY = 0;
    GPIO_OUT = 0xe000;
    return ch;
}

/* serial_getline -- input a line of text into buf with line editing */
void serial_getline(const char *prompt, char *buf, int nbuf)
{
    char *p = buf;

    printf(prompt);

    while (1) {
        char x = serial_getc();

        switch (x) {
        case '\b':
        case 0177:
            if (p > buf) {
                p--;
                serial_putc('\b');
                serial_putc(' ');
                serial_putc('\b');
            }
            break;

        case '\r':
            *p = '\0';
            serial_putc('\r');
            serial_putc('\n');
            return;

        default:
            /* Ignore other non-printing characters */
            if (x >= 040 && x < 0177 && p < &buf[nbuf]) {
                *p++ = x;
                serial_putc(x);
            }
        }
    }
}

char message[100];
volatile int bits_to_send[1500];
volatile int num_bits_to_send;
volatile int cur_bit = 0;
volatile int done = 0;
volatile int clock_counter = 0;

#define BAUD 12
#define TICK 5                // One timer interrupt per 5 millisec
int ticksPerBit = 1000 / (BAUD*TICK);
void advance(void) {
    if (clock_counter >= ticksPerBit){
        clock_counter = 0;
        printf("%d", bits_to_send[cur_bit]);
        GPIO_OUT = bits_to_send[cur_bit] ? 0xe000 : 0x0000;
        cur_bit++;
        if (cur_bit >= num_bits_to_send) {
            done = 1;
        }
    }
    clock_counter++;
}

void timer1_handler(void) {
    if (TIMER1_COMPARE[0]) {
        advance();
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


void init(void)
{
    GPIO_DIR = 0xfff0;
    serial_init();
    delay_loop(10000);
    printf("\nHello Red Light Transmitter!\n\n");
    init_timer();
    while (1) {
        serial_getline("Enter a message to send: ", message, 100);
        printf("Sending message...\n");
        
        for (int i=0; i<50; i++) bits_to_send[i] = 1; // White space at the start
        for (int i=0; message[i] != '\0'; i++) {
            int c = message[i];
            bits_to_send[50 + 11*i] = 0; // Start bit
            for (int j=7; j>=0; j--) {
                bits_to_send[50 + 11*i + 1 + j] = (c >> j) & 1;
            }
            bits_to_send[50 + 11*i + 8] = 1; // Stop bit
            bits_to_send[50 + 11*i + 9] = 1; // Stop bit
            bits_to_send[50 + 11*i + 10] = 1; // Stop bit
            num_bits_to_send = 50 + 11*i + 11;
        }
        cur_bit = 0;
        clock_counter = 0;
        done = 0;
        TIMER1_START = 1;
        while (!done){
            pause();
        }
        TIMER1_STOP = 1;
        printf("\nDone!\n");
        
        GPIO_OUT = 0x0000;
    }
}
