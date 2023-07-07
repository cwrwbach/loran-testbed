#pragma once
//This file will contain all the necessary periperal register address'

/*

Groups of peripherals:

Timers
Interrupt controller
GPIO
USB
PCB/I2C
DMA Controller
I2C
I2C / SPI Slave
SPI0 SPI1 SPI2
PWM
UART0 UART1
*/




#define MMIO_BASE 0x20000000                        //this for a Pi 1


//AUX Page 8
//Auxiliary peripherals Register Map (offset = 0x7E21 5000) 

#define AUX_IRQ              MMIO_BASE+0x215000         	// Auxiliary Interrupt status 3
#define AUX_ENABLES          MMIO_BASE+0x215004             // Auxiliary enables 3
#define AUX_MU_IO_REG        MMIO_BASE+0x215040             // Mini Uart I/O Data 8
#define AUX_MU_IER_REG       MMIO_BASE+0x215044             // Mini Uart Interrupt Enable 8
#define AUX_MU_IIR_REG       MMIO_BASE+0x215048             // Mini Uart Interrupt Identify 8
#define AUX_MU_LCR_REG       MMIO_BASE+0x21504C             // Mini Uart Line Control 8
#define AUX_MU_MCR_REG       MMIO_BASE+0x215050             // Mini Uart Modem Control 8
#define AUX_MU_LSR_REG       MMIO_BASE+0x215054             // Mini Uart Line Status 8
#define AUX_MU_MSR_REG       MMIO_BASE+0x215058             // Mini Uart Modem Status 8
#define AUX_MU_SCRATCH       MMIO_BASE+0x21505C             // Mini Uart Scratch 8
#define AUX_MU_CNTL_REG      MMIO_BASE+0x215060             // Mini Uart Extra Control 8
#define AUX_MU_STAT_REG      MMIO_BASE+0x215064             // Mini Uart Extra Status 32
#define AUX_MU_BAUD_REG      MMIO_BASE+0x215068             // Mini Uart Baudrate 16
#define AUX_SPI0_CNTL0_REG   MMIO_BASE+0x215080             // SPI 1 Control register 0 32
#define AUX_SPI0_CNTL1_REG   MMIO_BASE+0x215084             // SPI 1 Control register 1 8
#define AUX_SPI0_STAT_REG    MMIO_BASE+0x215088             // SPI 1 Status 32
#define AUX_SPI0_IO_REG      MMIO_BASE+0x215090             // SPI 1 Data 32
#define AUX_SPI0_PEEK_REG    MMIO_BASE+0x215094             // SPI 1 Peek 16
#define AUX_SPI1_CNTL0_REG   MMIO_BASE+0x2150C0             // SPI 2 Control register 0 32
#define AUX_SPI1_CNTL1_REG   MMIO_BASE+0x2150C4             // SPI 2 Control register 1 8 
#define AUX_SPI1_STAT_REG    MMIO_BASE+0x2150C8             // SPI 2 Status 32
#define AUX_SPI1_IO_REG      MMIO_BASE+0x2150D0             // SPI 2 Data 32
#define AUX_SPI1_PEEK_REG    MMIO_BASE+0x2150D4             // SPI 2 Peek 16 

                  
#define GPFSEL0         MMIO_BASE+0x200000  //GPIO  Function Select 0 32 R/W
#define GPFSEL0         MMIO_BASE+0x200000  //GPIO  Function Select 0 32 R/W
#define GPFSEL1         MMIO_BASE+0x200004  //GPIO  Function Select 1 32 R/W
#define GPFSEL2         MMIO_BASE+0x200008  //GPIO  Function Select 2 32 R/W
#define GPFSEL3         MMIO_BASE+0x20000C  //GPIO  Function Select 3 32 R/W
#define GPFSEL4         MMIO_BASE+0x200010  //GPIO  Function Select 4 32 R/W
#define GPFSEL5         MMIO_BASE+0x200014  //GPIO  Function Select 5 32 R/W
// - Reserved - -  MMIO_BASE+0x200018
#define GPSET0          MMIO_BASE+0x20001C //GPIO  Pin Output Set 0 32 W
#define GPSET1          MMIO_BASE+0x200020 //GPIO  Pin Output Set 1 32 W
// - Reserved - -  MMIO_BASE+0x200024
#define GPCLR0          MMIO_BASE+0x200028 //GPIO  Pin Output Clear 0 32 W
#define GPCLR1          MMIO_BASE+0x20002C //GPIO  Pin Output Clear 1 32 W
// - Reserved - -  MMIO_BASE+0x200030
#define GPLEV0          MMIO_BASE+0x200034 //GPIO  Pin Level 0 32 R
#define GPLEV1          MMIO_BASE+0x200038 //GPIO  Pin Level 1 32 R
// - Reserved - -  MMIO_BASE+0x20003C
#define GPEDS0          MMIO_BASE+0x200040 //GPIO  Pin Event Detect Status 0 32 R/W
#define GPEDS1          MMIO_BASE+0x200044 //GPIO  Pin Event Detect Status 1 32 R/W
// - Reserved - -  MMIO_BASE+0x200048
#define GPREN0          MMIO_BASE+0x20004C //GPIO  Pin Rising Edge Detect Enable 0 32 R/W
#define GPREN1          MMIO_BASE+0x200050 //GPIO  Pin Rising Edge Detect Enable 1 32 R/W
// - Reserved - -  MMIO_BASE+0x200054
#define GPFEN0          MMIO_BASE+0x200058 //GPIO  Pin Falling Edge Detect Enable 0 32 R/W
#define GPFEN1          MMIO_BASE+0x20005C //GPIO  Pin Falling Edge Detect Enable 1 32 R/W
// - Reserved - -  MMIO_BASE+0x200060
#define GPHEN0          MMIO_BASE+0x200064 //GPIO  Pin High Detect Enable 0 32 R/W
#define GPHEN1          MMIO_BASE+0x200068 //GPIO  Pin High Detect Enable 1 32 R/W
// - Reserved - -  MMIO_BASE+0x20006C
#define GPLEN0          MMIO_BASE+0x200070 //GPIO  Pin Low Detect Enable 0 32 R/W
#define GPLEN1          MMIO_BASE+0x200074 //GPIO  Pin Low Detect Enable 1 32 R/W
// - Reserved - -  MMIO_BASE+0x200078
#define GPAREN0         MMIO_BASE+0x20007C  //GPIO  Pin Async. Rising Edge Detect 0 32 R/W
#define GPAREN1         MMIO_BASE+0x200080  //GPIO  Pin Async. Rising Edge Detect 1 32 R/W
 //- Reserved - -  MMIO_BASE+0x200084
#define GPAFEN0         MMIO_BASE+0x200088  //GPIO  Pin Async. Falling Edge Detect 0 32 R/W
#define GPAFEN1         MMIO_BASE+0x20008C  //GPIO  Pin Async. Falling Edge Detect 1 32 R/W
// - Reserved - -  MMIO_BASE+0x200090
#define GPPUD           MMIO_BASE+0x200094  //GPIO  Pin Pull-up/down Enable 32 R/W
#define GPPUDCLK0       MMIO_BASE+0x200098    //GPIO  Pin Pull-up/down Enable Clock 0 32 R/W
#define GPPUDCLK1       MMIO_BASE+0x20009C    //GPIO  Pin Pull-up/down Enable Clock 1 32 R/W
// - Reserved - -  MMIO_BASE+0x2000A0
// - Test 4 R/W    MMIO_BASE+0x2000B0


#define IRQ_BASIC           MMIO_BASE+0x00B200
#define IRQ_PEND1           MMIO_BASE+0x00B204
#define IRQ_PEND2           MMIO_BASE+00B208
#define IRQ_FIQ_CONTROL     MMIO_BASE+0x00B210
#define IRQ_ENABLE1         MMIO_BASE+0x00B210
#define IRQ_ENABLE2         MMIO_BASE+0x00B214
#define IRQ_ENABLE_BASIC    MMIO_BASE+0x00B218
#define IRQ_DISABLE1        MMIO_BASE+0x00B21C
#define IRQ_DISABLE2        MMIO_BASE+0x00B220
#define IRQ_DISABLE_BASIC   MMIO_BASE+0x00B224




/*


MMIO_BASE+0x200000 GPFSEL0           //GPIO  Function Select 0 32 R/W
MMIO_BASE+0x200000 GPFSEL0           //GPIO  Function Select 0 32 R/W
MMIO_BASE+0x200004 GPFSEL1           //GPIO  Function Select 1 32 R/W
MMIO_BASE+0x200008 GPFSEL2           //GPIO  Function Select 2 32 R/W
MMIO_BASE+0x20000C GPFSEL3           //GPIO  Function Select 3 32 R/W
MMIO_BASE+0x200010 GPFSEL4           //GPIO  Function Select 4 32 R/W
MMIO_BASE+0x200014 GPFSEL5           //GPIO  Function Select 5 32 R/W
MMIO_BASE+0x200018 - Reserved - -
MMIO_BASE+0x20001C GPSET0           //GPIO  Pin Output Set 0 32 W
MMIO_BASE+0x200020 GPSET1           //GPIO  Pin Output Set 1 32 W
MMIO_BASE+0x200024 - Reserved - -
MMIO_BASE+0x200028 GPCLR0           //GPIO  Pin Output Clear 0 32 W
MMIO_BASE+0x20002C GPCLR1           //GPIO  Pin Output Clear 1 32 W
MMIO_BASE+0x200030 - Reserved - -
MMIO_BASE+0x200034 GPLEV0           //GPIO  Pin Level 0 32 R
MMIO_BASE+0x200038 GPLEV1           //GPIO  Pin Level 1 32 R
MMIO_BASE+0x20003C - Reserved - -
MMIO_BASE+0x200040 GPEDS0           //GPIO  Pin Event Detect Status 0 32 R/W
MMIO_BASE+0x200044 GPEDS1           //GPIO  Pin Event Detect Status 1 32 R/W
MMIO_BASE+0x200048 - Reserved - -
MMIO_BASE+0x20004C GPREN0           //GPIO  Pin Rising Edge Detect Enable 0 32 R/W
MMIO_BASE+0x200050 GPREN1           //GPIO  Pin Rising Edge Detect Enable 1 32 R/W
MMIO_BASE+0x200054 - Reserved - -
MMIO_BASE+0x200058 GPFEN0           //GPIO  Pin Falling Edge Detect Enable 0 32 R/W
MMIO_BASE+0x20005C GPFEN1           //GPIO  Pin Falling Edge Detect Enable 1 32 R/W
MMIO_BASE+0x200060 - Reserved - -
MMIO_BASE+0x200064 GPHEN0           //GPIO  Pin High Detect Enable 0 32 R/W
MMIO_BASE+0x200068 GPHEN1           //GPIO  Pin High Detect Enable 1 32 R/W
MMIO_BASE+0x20006C - Reserved - -
MMIO_BASE+0x200070 GPLEN0           //GPIO  Pin Low Detect Enable 0 32 R/W
MMIO_BASE+0x200074 GPLEN1           //GPIO  Pin Low Detect Enable 1 32 R/W
MMIO_BASE+0x200078 - Reserved - -
MMIO_BASE+0x20007C GPAREN0           //GPIO  Pin Async. Rising Edge Detect 0 32 R/W
MMIO_BASE+0x200080 GPAREN1           //GPIO  Pin Async. Rising Edge Detect 1 32 R/W
MMIO_BASE+0x200084 - Reserved - -
MMIO_BASE+0x200088 GPAFEN0           //GPIO  Pin Async. Falling Edge Detect 0 32 R/W
MMIO_BASE+0x20008C GPAFEN1           //GPIO  Pin Async. Falling Edge Detect 1 32 R/W
MMIO_BASE+0x200090 - Reserved - -
MMIO_BASE+0x200094 GPPUD             //GPIO  Pin Pull-up/down Enable 32 R/W
MMIO_BASE+0x200098 GPPUDCLK0           //GPIO  Pin Pull-up/down Enable Clock 0 32 R/W
MMIO_BASE+0x20009C GPPUDCLK1           //GPIO  Pin Pull-up/down Enable Clock 1 32 R/W
MMIO_BASE+0x2000A0 - Reserved - -
MMIO_BASE+0x2000B0 - Test 4 R/W 


*/

