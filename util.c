#include "bcm2835_defines.h"
#include "util.h"
#include "mbox.h"
#include "sig_anz_gui.h"
#include "gui_defs.h"

#include "fb2-font.h"
#include "ArialNum32x50.h"


//https://jsandler18.github.io/
#define VIDEOCORE_MBOX  (MMIO_BASE+0x0000B880)
#define MBOX_READ       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x0))
#define MBOX_POLL       ((volatile unsigned int*)(VIDEOCORE_MBOX+0x10))
#define MBOX_SENDER     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x14))
#define MBOX_STATUS     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x18))
#define MBOX_CONFIG     ((volatile unsigned int*)(VIDEOCORE_MBOX+0x1C))
#define MBOX_WRITE      ((volatile unsigned int*)(VIDEOCORE_MBOX+0x20))
#define MBOX_RESPONSE   0x80000000
#define MBOX_FULL       0x80000000
#define MBOX_EMPTY      0x40000000

#define PI1

#ifdef PI1
#define MMIO_BASE       0x20000000 //has to be !!!
#else
#define MMIO_BASE       0x3F000000
#endif

#define AUX_SPI0_CS     0x20204000
#define AUX_SPI0_FIFO   0x20204004
#define AUX_SPI0_CLK    0x20204008
#define AUX_SPI0_DLEN   0x2020400C
#define AUX_SPI0_LTOH   0x20204010
#define AUX_SPI0_DC     0x20204014

//#define TIMEOUT 400000
#define TEST 7
//-----
/*
extern struct surface
{
uint16_t * data;
int sz_x;
int sz_y;
int pos_x;
int pos_y;
}specanz;
*/
extern unsigned int fb_base;
extern unsigned int fb_size;
extern unsigned int phase_offset;
extern unsigned int image_size;

extern volatile unsigned int  __attribute__((aligned(16))) mbox[36];

uint8_t toggle_count;
void toggle_(void);
void toggle_down(void);
void toggle_tog(void);
//=============================


void plot_small_character(struct surface * dest, int x, int y,uint8_t char_num,uint16_t colour)
{
int horiz,vert;
uint8_t test_byte;

char_num = '6';


int location = (300*IMAGE_WIDTH) + 640;
for(vert=0; vert<16;vert++)
    {
    test_byte = defaultFontData[(char_num * 16)+vert];

    for(horiz = 0; horiz<8;horiz++)
        {
        if(test_byte & 0x80) 

PUT8(fb_base + phase_offset + location + horiz + (vert*IMAGE_WIDTH),YELLOW);



          //   set_pixel(dest,x+horiz,y+vert,colour);
        test_byte <<=1; 
        }
    }
}

void plot_huge_numeral(struct surface * dest, int x, int y,uint8_t numeral,uint16_t colour)
{
//This uses characters which are 32 X 50 (only does 0 to 9 and colon)
//The colon character is 0x3a
int horiz,vert,byt,bn;
int xx,yy;
unsigned char test_byte;

numeral -= 0x30; //ascii to digit
numeral *=4; //4 bytes per row, 

for(vert=0,yy=0; vert<50;vert++,yy+=1) //50 rows *4 bytes = 200 bytes per numeral
    {
    for(byt=0;byt<4;byt++)
        {
        bn=(numeral*50)+byt+(vert*4);
        test_byte = ArialNumFontPlus[bn]; 
        for(horiz =0,xx=0; horiz<8;horiz++,xx++)
            {
            set_pixel(dest,x+xx+(byt*8),y+yy,BLACK);
            if(test_byte & 0x80) //MSB only
                { 
                set_pixel(dest,x+xx+(byt*8),y+yy,colour); 
                }
            test_byte <<=1; //shifts 8 bits per byte
            }
        }
    }
}

void plot_large_string(int x,int y,char * string,char colour)
{
int c;
c=0;

while(*string) 
    {
    plot_large_character(x+(c*16),y,string++,colour);
    c++;
    }
}



void plot_large_character(int x, int y,char *val,char colour)
{
int horiz,vert;
int xx,yy;
unsigned short test_word;

char char_num = * val -32;

int location = (y*IMAGE_WIDTH) + x;

for(vert=0,yy=0; vert<24;vert++,yy+=1)
    {
    test_word = ASCII_16x24[((char_num) * 24)+vert]; 
    for(horiz =0,xx=0; horiz<16;horiz++,xx++)
        {
        if(test_word & 0x0001)
            {
            PUT8(fb_base + image_size + location + horiz + (vert*IMAGE_WIDTH),colour);
            PUT8(fb_base + location + horiz + (vert*IMAGE_WIDTH),colour);
            }        
        else
            {
            PUT8(fb_base + image_size + location + horiz + (vert*IMAGE_WIDTH),BLACK);
            PUT8(fb_base + location + horiz + (vert*IMAGE_WIDTH),BLACK);
            }
        test_word >>=1; 
        }
    }
}

















//=====
void init_gpio()
{
uint32_t ra;

ra=GET32(GPFSEL0);
    ra&=~(7<<21);
    ra|=1<<21;
    PUT32(GPFSEL0,ra);


    ra=GET32(GPFSEL1);
    ra&=~(7<<18);
    ra|=1<<18;
    PUT32(GPFSEL1,ra);

    PUT32(ARM_TIMER_CTL,0x00F90000);
    PUT32(ARM_TIMER_CTL,0x00F90200);
}

void spi_init ( void )
{
    unsigned int ra;

    ra=GET32(AUX_ENABLES);
    ra|=2; //enable spi0
    PUT32(AUX_ENABLES,ra);

    ra=GET32(GPFSEL0);
    ra&=~(7<<27); //gpio9
    ra|=4<<27;    //alt0
    ra&=~(7<<24); //gpio8
    ra|=4<<24;    //alt0
    ra&=~(7<<21); //gpio7
    ra|=4<<21;    //alt0
    PUT32(GPFSEL0,ra);
    ra=GET32(GPFSEL1);
    ra&=~(7<<0); //gpio10/
    ra|=4<<0;    //alt0
    ra&=~(7<<3); //gpio11/
    ra|=4<<3;    //alt0
    PUT32(GPFSEL1,ra);
    ra=GET32(GPFSEL2);
    ra&=~(7<<15); //gpio25/
    ra|=1<<15;    //output
    PUT32(GPFSEL2,ra);

    PUT32(AUX_SPI0_CS,0x0000030);
//    PUT32(AUX_SPI0_CLK,0x0000); //slowest possible, could probably go faster here
    PUT32(AUX_SPI0_CLK,26);

}

void spi_one_byte ( unsigned int x )
{
    PUT32(AUX_SPI0_CS,0x000000B0); //TA=1 cs asserted
    while(1)
        {
        if(GET32(AUX_SPI0_CS)&(1<<18)) break; //TXD
        }
    PUT32(AUX_SPI0_FIFO,x&0xFF);
    while(1) if(GET32(AUX_SPI0_CS)&(1<<16)) break;
    //while(1) if(GET32(AUX_SPI0_CS)&(1<<17)) break; //should I wait for this?
    PUT32(AUX_SPI0_CS,0x00000000); //cs0 comes back up
}

void spi_command ( unsigned int cmd )
{
    PUT32(GPCLR0,1<<25); //D/C = 0 for command
    spi_one_byte(cmd);
}
//------------------------------------------------------------------------
void spi_data ( unsigned int data )
{
    PUT32(GPSET0,1<<25); //D/C = 1 for data
    spi_one_byte(data);
}


uint32_t spi_read()
{
uint32_t read_data;

read_data = GET32(AUX_SPI0_FIFO); 
return read_data;
}


void delay(int del_mS)
{
#define ONE_MS 1000
uint32_t start,stop;

start=GET32(ARM_TIMER_CNT);

while(1)
    {
    stop=GET32(ARM_TIMER_CNT);
    if((stop-start)>=del_mS * ONE_MS) 
        break;
    }
}
//---

void blinker()
{ 
//while(1)
 //   {
    PUT32(GPSET0,1<<16);
    PUT32(GPSET0,1<<TEST);
      
    delay(200);
  
    PUT32(GPCLR0,1<<16);
    PUT32(GPCLR0,1<<TEST);
      
    delay(200);
 //  }
}

void toggle()
{
PUT32(GPSET0,1<<TEST);

PUT32(GPCLR0,1<<TEST);
}

void toggle_tog()
{
toggle_count++;
if(toggle_count & 0x01)
    toggle_up();
else toggle_down();
}

void toggle_up()
{
PUT32(GPSET0,1<<TEST);
}


void toggle_down()
{
PUT32(GPCLR0,1<<TEST);
}
//---

void init_frame_buffer()
{
PUT32(0x40040000, IMAGE_WIDTH); //* #0 Physical Width of the screen
PUT32(0x40040004, IMAGE_HEIGHT); //* #4 Physical Height ditto
PUT32(0x40040008, IMAGE_WIDTH); //* #8 Virtual Width of buffer memory
PUT32(0x4004000C, IMAGE_HEIGHT * 2); //* #12 Virtual Height ditto
PUT32(0x40040010, 0); //* #16 GPU - Pitch will be returned
PUT32(0x40040014, 8); //* #8 Bit Depth 
PUT32(0x40040018, 0); //* #24 X ?
PUT32(0x4004001C, 0); //* #28 Y ?
PUT32(0x40040020, 0); //* #32 GPU - Pointer 
PUT32(0x40040024, 0); //* #36 GPU - Size 

//write the data to the Frame Buffer mailbox (1)
uint32_t mailbox=0x2000B880;
uint32_t ra;
while(1)
    {
    if((GET32(mailbox+0x18)&0x80000000)==0) break;
    }
PUT32(mailbox+0x20,0x40040001);

//Read back the FB Mailbox reply 
while(1)
    {
    while(1)
        {
        ra=GET32(mailbox+0x18);
        if((ra&0x40000000)==0) break;
        }
    ra=GET32(mailbox+0x00);
        
    if((ra&0xF) == 1) break;
    }

fb_base=GET32(0x40040020);
fb_size=GET32(0x40040024);
	
 mbox[0] = 17;                  //Message size - increase for addtional pallete entries
 mbox[1]=	0x0,                    // clear response
 mbox[2]=	0x0004800b,             // set palette cmd
 mbox[3]=	0,                     //len 24...1032 8 + (3 pallete entries * 16)
 mbox[4]=	0,                      // response size
 mbox[5]=	0,                    // offset 0
 mbox[6]=	9,                    //  palette entries. increase for addtional pallete entries

mbox[7]=    0x00000000; //BLACK
mbox[8]=	0xff0000ff,  //RED           // Entry 1 Full red
mbox[9]=	0xff00ff00,  //GREEN                  // Entry 1 0 green
mbox[10]=	0xffff0000,                    //  Entry 1 0 blue

mbox[11]=	0xffff00ff, //MAGENTA            // Entry 1 Full red
mbox[12]=	0xff00ffff, //YELLOW                   // Entry 1 0 green
mbox[13]=	0xffffff00, //CYAN
mbox[14]=   0xff000f00, //DARK_GREEN
mbox[15]=   0xff7f7f7f, //GRID
mbox[16] = 0;

properties_mbox_call(MBOX_CH_PROP);
}

void byte_copy(uint8_t *from, uint32_t *to,uint32_t n_bytes)
{
uint32_t temp;
int n,t;
for(n=0,t=0; n< n_bytes/4;t++)
    {
    temp = 0x0;
    temp |=from[n++];
    temp |=from[n++]<<8;
    temp |=from[n++]<<16;
    temp |=from[n++]<<24;
    to[t] = temp;
    }    
}
//----------------------

int abs(int val)
{
if (val < 0)
    val *=-1;
return val;
}

void set_pixel(uint8_t * dest, int x, int y, int sz_x,uint8_t colour)
{
int location;

location = x + (y*sz_x) ; 
dest[location] = colour;
}

//Make a mailbox call. Returns 0 on failure, non-zero on success
int properties_mbox_call(unsigned char ch)
{
    unsigned int r = (((unsigned int)((unsigned long)&mbox)&~0xF) | (ch&0xF));
    /* wait until we can write to the mailbox */
    do
        {
        asm volatile("nop");
        }
    while(*MBOX_STATUS & MBOX_FULL);
    
    /* write the address of our message to the mailbox with channel identifier */
    *MBOX_WRITE = r;
    /* now wait for the response */

    while(1) {
        /* is there a response? */
        do{asm volatile("nop");}while(*MBOX_STATUS & MBOX_EMPTY);
        /* is it a response to our message? */
        if(r == *MBOX_READ)
            /* is it a valid successful response? */
            return mbox[1]==MBOX_RESPONSE;
    }
    return 0;
}

//------------------------------------------------------------------------
unsigned int uart_lcr ( void )
{
    return(GET32(AUX_MU_LSR_REG));
}
//------------------------------------------------------------------------
void uart_putch(char c)
{
uart_send(c);
}

void uart_puts(char *s) 
{
while(*s) 
    {
    //convert newline to carriage return + newline 
    if(*s=='\n')
        uart_send('\r');
    uart_send(*s++);
    }
}

//------------------------------------------------------
unsigned int uart_recv ( void )
{
    while(1)
    {
        if(GET32(AUX_MU_LSR_REG)&0x01) break;
    }
    return(GET32(AUX_MU_IO_REG)&0xFF);
}
//------------------------------------------------------------------------
unsigned int uart_check ( void )
{
    if(GET32(AUX_MU_LSR_REG)&0x01) return(1);
    return(0);
}
//------------------------------------------------------------------------


void uart_send (char c )
{

//do{asm volatile("nop");} while(!(*AUX_MU_LSR&0x20));
  while(1)
    {
    asm volatile("nop");
        if(GET32(AUX_MU_LSR_REG)&0x20) break;
//if ( !(*AUX_MU_LSR_REG &0x20)) 
    //break;

   }
//
    //*AUX_MU_IO=c;
    PUT32(AUX_MU_IO_REG,c);
}
//------------------------------------------------------------------------
void uart_flush ( void )
{
    while(1)
    {
        if((GET32(AUX_MU_LSR_REG)&0x100)==0) break;
    }
}
//------------------------------------------------------------------------
void uart_puthex( unsigned int d )
{
unsigned int rb;
unsigned int rc;

uart_send('0');
uart_send('x');

rb=32;
while(1)
    {
    if(rb==16)
        uart_send('-');
    rb-=4;
    rc=(d>>rb)&0xF;
    if(rc>9) rc+=0x37; else rc+=0x30;
    uart_send(rc);
    if(rb==0) 
        break;
    }
    uart_send(0x20);
}

void uart_puteol (void)
{
uart_send(0x0D);
uart_send(0x0A);
}

void uart_init_no_int ( void )
{
    unsigned int ra;

    PUT32(AUX_ENABLES,1);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_CNTL_REG,0);
    PUT32(AUX_MU_LCR_REG,3);
    PUT32(AUX_MU_MCR_REG,0);
    PUT32(AUX_MU_IER_REG,0);
 //   PUT32(AUX_MU_IER_REG,0x5); //enable rx interrupts
    PUT32(AUX_MU_IIR_REG,0xC6);
    PUT32(AUX_MU_BAUD_REG,33); // 33 = 981600, 270 = 115200);
    ra=GET32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    ra&=~(7<<15); //gpio15
    ra|=2<<15;    //alt5
    PUT32(GPFSEL1,ra);
    PUT32(GPPUD,0);
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,(1<<14)|(1<<15));
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,0);
    PUT32(AUX_MU_CNTL_REG,3);
}

void uart_init_interrupt ( void )
{
    unsigned int ra;

    PUT32(AUX_ENABLES,1);
    PUT32(AUX_MU_IER_REG,0);
    PUT32(AUX_MU_CNTL_REG,0);
    PUT32(AUX_MU_LCR_REG,3);
    PUT32(AUX_MU_MCR_REG,0);
    PUT32(AUX_MU_IER_REG,0x5); //enable rx interrupts
    PUT32(AUX_MU_IIR_REG,0xC6);
    PUT32(AUX_MU_BAUD_REG,33); //=921600 270 = 115200);

    ra=GET32(GPFSEL1);
    ra&=~(7<<12); //gpio14
    ra|=2<<12;    //alt5
    ra&=~(7<<15); //gpio15
    ra|=2<<15;    //alt5
    PUT32(GPFSEL1,ra);

    PUT32(GPPUD,0);
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,(1<<14)|(1<<15));
    for(ra=0;ra<150;ra++) dummy(ra);
    PUT32(GPPUDCLK0,0);

    PUT32(AUX_MU_CNTL_REG,3);
}

void uart_putdec(int input)
{
//int neg_flag = 0;
unsigned int decimal[10]; //thou=0,hun=0,ten=0,unit=0;
int divisor;
int dv;

for(int n=0;n<10;n++)
    decimal[n] = 0;

//if(input < 0 ) 
//    {
//    input *=-1;
//    neg_flag = 1;
//    }

dv=9;
divisor = 1000000000;

while(dv >= 0)
    {
    while(input >= divisor)
        {
        input -= divisor;
        decimal[dv]++;
        }
    divisor = divisor /10;
    dv --;
    }

for(int n=9;n>=0;n--)
    uart_putch(0x30+decimal[n]); 
}


void plot_puthex( unsigned int d )
{
unsigned int rb;
unsigned int rc;

char show_hex[16];
int digit=0;

show_hex[digit++]= '0';
show_hex[digit++]= 'x';

rb=32;
while(1)
    {
    if(rb==16)
        show_hex[digit++] = ('-');
    rb-=4;
    rc=(d>>rb)&0xF;
    if(rc>9) rc+=0x37; else rc+=0x30;
    show_hex[digit++] = (rc);
    if(rb==0) 
        break;
    }
plot_large_string(300,700, show_hex,YELLOW);
   
}




void plot_putdec(int input)
{
//int neg_flag = 0;
unsigned int decimal[10]; //thou=0,hun=0,ten=0,unit=0;
int divisor;
int dv;



for(int n=0;n<10;n++)
    decimal[n] = 0;

//if(input < 0 ) 
//    {
//    input *=-1;
//    neg_flag = 1;
//    }

dv=9;
divisor = 1000000000;

while(dv >= 0)
    {
    while(input >= divisor)
        {
        input -= divisor;
        decimal[dv]++;
        }
    divisor = divisor /10;
    dv --;
    }

char show_dec[16];
int digit=0;
for(int n=9;n>=0;n--)
    {
    show_dec[digit++] = decimal[n]+ 0x30;
    }

plot_large_string(100,700, show_dec,YELLOW);
}

//------------------------------------------------------------------------
void  timer_init ( void )
    {
    //0xF9+1 = 250
    //250MHz/250 = 1MHz
    PUT32(ARM_TIMER_CTL,0x00F90000);
    PUT32(ARM_TIMER_CTL,0x00F90200);
    }

unsigned int timer_tick ( void )
    {
    return(GET32(ARM_TIMER_CNT));
    }

void plot_dotted_line (uint8_t * dest,int x0, int y0, int x1, int y1, int sz_x,uint8_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 
int count;
count = 0;
for (;;) //evah
    { 
    if(count++ & 0x04)// miss out spaces
        set_pixel(dest,x0,y0,sz_x,colour);

    if (x0 == x1 && y0 == y1) break;
    e2 = 2 * err;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}

void plot_line (uint8_t * dest, int x0, int y0, int x1, int y1, int sz_x,uint8_t colour)
{
int dx =  abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1; 
int err = dx + dy, e2; //error value e_xy 

for (;;) //evah
    { 
    set_pixel(dest,x0,y0,sz_x,colour);

    if (x0 == x1 && y0 == y1) break;
    e2 = err*2;
    if (e2 >= dy) { err += dy; x0 += sx; } // e_xy+e_x > 0 
    if (e2 <= dx) { err += dx; y0 += sy; } // e_xy+e_y < 0 
    }
}

void clear_frame()
{
int xray = 0x00000000; //clear screen 4 bytes per word.
MEMSET(xray,fb_base,IMAGE_HEIGHT/4 * IMAGE_WIDTH);
MEMSET(xray,fb_base+ IMAGE_SIZE,IMAGE_HEIGHT/4 * IMAGE_WIDTH);
}
