// The raspberry pi firmware at the time this was written defaults
// loading at address 0x8000.  Although this bootloader could easily
// load at 0x0000, it loads at 0x8000 so that the same binaries built
// for the SD card work with this bootloader.



#define ARM_TIMER_CTL 0x2000B408
#define ARM_TIMER_CNT 0x2000B420


/*

#define GPFSEL0 0x20200000
#define GPFSEL1 0x20200004
#define GPFSEL2 0x20200008
#define GPFSEL3 0x2020000C
#define GPFSEL4 0x20200010
#define GPFSEL5 0x20200014

#define GPSET0  0x2020001C
#define GPSET1  0x20200020


#define GPCLR0  0x20200028
#define GPCLR1  0x2020002C



#define GPPUD       0x20200094
#define GPPUDCLK0   0x20200098

#define AUX_ENABLES     0x20215004
#define AUX_MU_IO_REG   0x20215040
#define AUX_MU_IER_REG  0x20215044
#define AUX_MU_IIR_REG  0x20215048
#define AUX_MU_LCR_REG  0x2021504C
#define AUX_MU_MCR_REG  0x20215050
#define AUX_MU_LSR_REG  0x20215054
#define AUX_MU_MSR_REG  0x20215058
#define AUX_MU_SCRATCH  0x2021505C
#define AUX_MU_CNTL_REG 0x20215060
#define AUX_MU_STAT_REG 0x20215064
#define AUX_MU_BAUD_REG 0x20215068
*/
//GPIO14  TXD0 and TXD1
//GPIO15  RXD0 and RXD1


typedef unsigned int uint32_t;
typedef unsigned short uint16_t;
typedef unsigned char uint8_t;


void PUT32 ( unsigned int, unsigned int );
void PUT16 ( unsigned int, unsigned int );
void PUT8 ( unsigned int, unsigned int );
unsigned int GET32 ( unsigned int );
unsigned int GETPC ( void );
void BRANCHTO ( unsigned int );
void dummy ( unsigned int );

void uart_init ( void );
unsigned int uart_lcr ( void );
void uart_flush ( void );
void uart_send ( char);
void uart_putch ( char );
unsigned int uart_recv ( void );
unsigned int uart_check ( void );
void uart_puthex ( unsigned int );
void uart_puteol( void);

void uart_puts(char *) ;

void MEMSET(uint32_t,uint32_t,uint32_t);

unsigned int MailboxWrite ( unsigned int, unsigned int);
unsigned int MailboxRead ( unsigned int);
void init_frame_buffer(void);

void byte_copy(uint8_t *, uint32_t *,uint32_t );

void timer_init ( void );
unsigned int timer_tick ( void );

void timer_init ( void );
unsigned int timer_tick ( void );


void toggle(void);

//void plot_line (uint8_t* ,int, int, int, int,uint8_t );
//void plot_dotted_line (uint8_t* ,int, int, int, int,uint8_t);
//void test_bars(void);
