#include "bcm2835_defines.h"
#include "util.h"
#include "gui_defs.h"
#include "mbox.h"
#include "sig_anz_gui.h"

#define RXBUFMASK 0x3FF //max buffer size
#define SYNC_WORD 0x5A0FBE66

volatile unsigned int rxhead;
volatile unsigned int rxtail;
volatile unsigned char uart_rx_buf[RXBUFMASK+1];

volatile unsigned int  __attribute__((aligned(16))) mbox[36]; //mailbox message buffer
uint32_t fb_base,fb_size; //Frame buffer params
int phase;
int phase_offset;
int image_size;
int horiz_words;
uint8_t pkt_type;

uint8_t win_data_buf[MAX_PAK_LEN] ; //PKT_DATA_LEN+PKT_HEADER_LEN];

//uint8_t win_1_buf[FFT_WIN_WIDTH+PKT_HEADER_LEN];
int win_1_width;
int win_1_height;
int win_1_left;
int win_1_right;
int win_1_top;
int win_1_bot;


//uint8_t win_2_buf[FFT_WIN_WIDTH+PKT_HEADER_LEN];
int win_2_width;
int win_2_height;
int win_2_left;
int win_2_right;
int win_2_top;
int win_2_bot;


//uint8_t win_3_buf[FFT_WIN_WIDTH+PKT_HEADER_LEN];
int win_3_width;
int win_3_height;
int win_3_left;
int win_3_right;
int win_3_top;
int win_3_bot;









//FFT levels
int last_lev;
int db_lev;
int pix_lev;
int j;

//++++++++++

//Function declarations:
void uart_init_interrupt(void);
void enable_irq(void);
void clear_frame(void);
void toggle_up(void);
void toggle_down(void);
void toggle_tog(void);
//==================

void draw_plot_1(int y_scale)
{
int x_pos;
int y_pos;

for(int ix = 1; ix<win_1_width;ix++)
    {
    db_lev= win_data_buf[ix+16] *y_scale; //the plus 16 steps over the header

    if(last_lev == db_lev)
        {
        pix_lev = last_lev;
        x_pos = fb_base + phase_offset + win_1_left + ix;
        y_pos = (pix_lev*IMAGE_WIDTH) + (win_1_top*IMAGE_WIDTH);
        PUT8(x_pos + y_pos,YELLOW);
        }

    else
        {
        while(last_lev != db_lev)
            {
            if(last_lev < db_lev) 
                last_lev++;
            else 
                last_lev--;
            pix_lev = last_lev;
            x_pos = fb_base + phase_offset + win_1_left + ix;
            y_pos = (pix_lev*IMAGE_WIDTH) + (win_1_top*IMAGE_WIDTH);
            PUT8(x_pos + y_pos,YELLOW);
            }
        last_lev = db_lev;
        }
    }
}
//---

void draw_grid_1()
{
int i;

int xxx = win_1_height / 8;
int horiz_space = FFT_HORIZ_SPACE;
int horiz_centre = fb_base+phase_offset+ (win_1_width/2) + win_1_left;

for(i=1;i<8;i++) //horiz grid lines
    MEMSET(0x00080000 ,fb_base+phase_offset+(win_1_top*IMAGE_WIDTH)+ (i*xxx * IMAGE_WIDTH) + win_1_left,horiz_words); //grid horiz

for(i=0;i<6;i++) //vert grid lines centre to right
    for(int y=win_1_top;y< win_1_bot;y+=4) //ignore 4 lines to give dot-gaps
        MEMSET(0x08000000 ,horiz_centre + (y * IMAGE_WIDTH) + (i*horiz_space),1);  //grid vert  

for(i=1;i<6;i++) //vert grid lines centre to left
    for(int y=win_1_top;y< win_1_bot;y+=4) //ignore 4 lines to give dot-gaps
        MEMSET(0x08000000 ,horiz_centre + (y * IMAGE_WIDTH) - (i*horiz_space),1);  //grid vert  


//Horizontal Framing Lines   
MEMSET(0x03030303 ,fb_base+phase_offset+(win_1_top*IMAGE_WIDTH)+win_1_left,horiz_words); //top horiz
MEMSET(0x03030303 ,fb_base+phase_offset+(win_1_bot*IMAGE_WIDTH)+win_1_left,horiz_words); //bot horiz

//Vertical Framimg Lines
for(int y=0;y<win_1_height;y++)
    MEMSET(0x03030303 ,fb_base+phase_offset+((y+win_1_top) * IMAGE_WIDTH)+win_1_left,1);  //left vert
for(int y=0;y<win_1_height;y++)
    MEMSET(0x03030303 ,fb_base+phase_offset+((y+win_1_top) * IMAGE_WIDTH)+win_1_right-2,1); //right vert  


}

void draw_grid_2()
{
int i;

int xxx = win_2_height / 8;
int horiz_space = FFT_HORIZ_SPACE;
int horiz_centre = fb_base+phase_offset+ (win_2_width/2) + win_2_left;

for(i=1;i<8;i++) //horiz grid lines
    MEMSET(0x00080000 ,fb_base+phase_offset+(win_2_top*IMAGE_WIDTH)+ (i*xxx * IMAGE_WIDTH) + win_2_left,horiz_words); //grid horiz

for(i=0;i<5;i++) //vert grid lines centre to right
    for(int y=win_2_top;y< win_2_bot;y+=4) //ignore 4 lines to give dot-gaps
        MEMSET(0x08000000 ,horiz_centre + (y * IMAGE_WIDTH) + (i*horiz_space),1);  //grid vert  

for(i=1;i<5;i++) //vert grid lines centre to left
    for(int y=win_2_top;y< win_2_bot;y+=4) //ignore 4 lines to give dot-gaps
        MEMSET(0x08000000 ,horiz_centre + (y * IMAGE_WIDTH) - (i*horiz_space),1);  //grid vert  


//Horizontal Framing Lines   
MEMSET(0x03030303 ,fb_base+phase_offset+(win_2_top*IMAGE_WIDTH)+win_2_left,horiz_words); //top horiz
MEMSET(0x03030303 ,fb_base+phase_offset+(win_2_bot*IMAGE_WIDTH)+win_2_left,horiz_words); //bot horiz

//Vertical Framimg Lines
for(int y=0;y<win_2_height;y++)
    MEMSET(0x03030303 ,fb_base+phase_offset+((y+win_2_top) * IMAGE_WIDTH)+win_2_left,1);  //left vert
for(int y=0;y<win_2_height;y++)
    MEMSET(0x03030303 ,fb_base+phase_offset+((y+win_2_top) * IMAGE_WIDTH)+win_2_right-2,1); //right vert  
}



void draw_grid_3()
{
int i;

int xxx = win_3_height / 8;
int horiz_space = FFT_HORIZ_SPACE;
int horiz_centre = fb_base+phase_offset+ (win_3_width/2) + win_3_left;


for(i=1;i<8;i++) //horiz grid lines
    MEMSET(0x00080000 ,fb_base+phase_offset+(win_3_top*IMAGE_WIDTH)+ (i*xxx * IMAGE_WIDTH) + win_3_left,horiz_words); //grid horiz

for(i=0;i<5;i++) //vert grid lines centre to right
    for(int y=win_3_top;y< win_3_bot;y+=4) //ignore 4 lines to give dot-gaps
        MEMSET(0x08000000 ,horiz_centre + (y * IMAGE_WIDTH) + (i*horiz_space),1);  //grid vert  

for(i=1;i<5;i++) //vert grid lines centre to left
    for(int y=win_3_top;y< win_3_bot;y+=4) //ignore 4 lines to give dot-gaps
        MEMSET(0x08000000 ,horiz_centre + (y * IMAGE_WIDTH) - (i*horiz_space),1);  //grid vert  


//Horizontal Framing Lines   
MEMSET(0x03030303 ,fb_base+phase_offset+(win_3_top*IMAGE_WIDTH)+win_3_left,horiz_words); //top horiz
MEMSET(0x03030303 ,fb_base+phase_offset+(win_3_bot*IMAGE_WIDTH)+win_3_left,horiz_words); //bot horiz

//Vertical Framimg Lines
for(int y=0;y<win_3_height;y++)
    MEMSET(0x03030303 ,fb_base+phase_offset+((y+win_3_top) * IMAGE_WIDTH)+win_3_left,1);  //left vert
for(int y=0;y<win_3_height;y++)
    MEMSET(0x03030303 ,fb_base+phase_offset+((y+win_3_top) * IMAGE_WIDTH)+win_3_right-2,1); //right vert  
}


void fft_fill(uint32_t col)
{
for(int y= win_1_top;y<win_1_bot;y++)
    {
    MEMSET(col,(fb_base+phase_offset) +(IMAGE_WIDTH*y) + win_1_left,win_1_right/4);
    }
}

void  fill_frame_block (uint8_t colour ,int pos_x,int pos_y,int sz_x,int sz_y)
{
for(int y=0;y <sz_y;y++)
    for(int x=0;x<sz_x;x++)
        {
        PUT8(fb_base + ((pos_y+y) * IMAGE_WIDTH) + pos_x+x,colour); //alpha
        PUT8((fb_base + image_size)+ ((pos_y+y) * IMAGE_WIDTH) + pos_x+x,colour); //bravo
        }
}

//-------

void packet_select(int type)
{

switch(type)
    {
    case 0x42:
    draw_plot_1(1);
    break;


    default:
    break;
    
    }


}

//-------

void c_irq_handler ( void )
{
unsigned int rb,rc;
//an IRQ has occurred, find out why
while(1) //resolve all interrupts to uart
    {
 
    rb=GET32(AUX_MU_IIR_REG); //Interrupt identify (bok page.12)
    if((rb&1)==1) 
        break; //no more interrupts

    if((rb&6)==4)
        {
        //receiver holds a valid byte
        rc=GET32(AUX_MU_IO_REG); //read byte from rx fifo
        uart_rx_buf[rxhead++]=rc&0xFF;
        rxhead&=RXBUFMASK; //MAX size
        } 
    }
}

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++

void notmain ( void )
{
uint16_t rx_ptr;
//uint16_t pkt_ptr;
uint16_t sync_flag; 
uint32_t sync_word; //byte 0
uint32_t ident_word; //byte 1
uint32_t stamp_word; //byte 2
uint32_t check_word; //byte 3

int pkt_active;
uint16_t pak_len;

uint8_t uart_byte;
//unsigned int debug;
int y_scale;

image_size = IMAGE_SIZE;
rx_ptr = 0;
//pkt_ptr = 0;
sync_word = 0;
sync_flag = 0;

//setup fft window sizes
win_1_width = FFT_WIN_WIDTH;
win_1_height = FFT_WIN_HEIGHT;
win_1_left = FFT_WIN_LEFT;
win_1_right = win_1_left + win_1_width; 
win_1_top = FFT_WIN_TOP;
win_1_bot = win_1_top + win_1_height; 


win_2_width = FFT_WIN_WIDTH;
win_2_height = FFT_WIN_HEIGHT;
win_2_left = FFT_WIN_LEFT;
win_2_right = win_2_left + win_2_width; 
win_2_top = FFT_WIN_TOP +270 ;
win_2_bot = win_2_top + win_2_height; 

win_3_width = FFT_WIN_WIDTH;
win_3_height = 170;
win_3_left = FFT_WIN_LEFT;
win_3_right = win_3_left + win_3_width; 
win_3_top = FFT_WIN_TOP +540 ;
win_3_bot = win_3_top + win_3_height; 

horiz_words = win_1_width/4;                  //Because does 4 pixels per word (faster)

uart_init_interrupt();

PUT32(IRQ_DISABLE1,1<<29); //AUX INT (bok p.113)

init_gpio();

init_frame_buffer();
mbox[0] = 8*4;                  // length of the message
mbox[1] = MBOX_REQUEST;         // this is a request message
mbox[2] = TAG_SET_VIRTUAL_OFFSET;   // 
mbox[3] = 8;                    //x
mbox[4] = 0;                    //y
mbox[5] = 0;                    // clear output buffer
mbox[6] = 0;
mbox[7] = MBOX_TAG_LAST;
properties_mbox_call(MBOX_CH_PROP);

PUT32(IRQ_ENABLE1,1<<29); //AUX INT (bok p.113)

enable_irq();

delay(500);

clear_frame();

phase = 0;



//fill_frame_block(BLUE,0,0,20,20);
//fill_frame_block(GREEN,IMAGE_WIDTH-20,0,20,20);
//fill_frame_block(RED,0,IMAGE_HEIGHT-20,20,20);
//fill_frame_block(YELLOW,IMAGE_WIDTH-20,IMAGE_HEIGHT-20,20,20);

phase = 0;

    // display an ASCII string on screen with PSF
//    lfb_print(200, 300, "Hello World!");



//for(int n=0;n<10;n++)
//    {
//    char_num = 0x30+n;
//    plot_large_character(640+(n*16),400,char_num,GREEN);
    //delay(1000);
 //   }

//plot_small_character(fb_base,200,200,IMAGE_WIDTH,0x33,GREEN);

plot_large_string(104,660,"Decimal      Hex",GREEN);

int td;
td=123456789;
plot_putdec(td);

plot_puthex(td);
pkt_active = 0;
//MAIN LOOP --->>>
while(1)// main display loop
    {
    //int s;
    if(sync_flag==1)
        {
        sync_flag = 0;
        phase^= 0x01; //toggle phase
        phase_offset  = image_size * phase; //setup frame start address 600 lines * 1024 wide
        
        mbox[1] = MBOX_REQUEST;         // this is a request message

        if(phase ==1)
            mbox[6] = 0; //0; //top block ALPHA
        else 
            mbox[6] = IMAGE_HEIGHT; //600- lower block BRAVO

        properties_mbox_call(MBOX_CH_PROP); //set the correct buffer

        delay(5);

        if(pkt_type == 0x33) //temp code for a fft
            {
            pkt_type = 0x00;
            fft_fill(0x07070707); //background colour

            draw_grid_1();

       // draw_grid_2();

      //  draw_grid_3();
            y_scale =1;
            draw_plot_1(y_scale);
            }
        delay(5);

        phase_offset  = image_size * phase; //614400

        }
    else
        {

// draw_grid_2();
    }

//receive any new data in buffer
    while(rxtail!=rxhead)
        {
        uart_byte = uart_rx_buf[rxtail++] & 0x000000ff;
        rxtail &= RXBUFMASK; //ensure no over-run of buffer
        sync_word |= uart_byte;
        if(sync_word == SYNC_WORD)
            {
            rx_ptr = 3; //set to third byte of sync_word.
            sync_word = 0;
            pak_len = 0;
            pkt_active =1 ;
            }
        sync_word = (sync_word<<8);
        win_data_buf[rx_ptr++] = uart_byte;   

        if(pkt_active)
            {
            if(rx_ptr == 15) //get packet info
                {
                pkt_type = win_data_buf[4]; 
                pak_len = (win_data_buf[6] & 0xff)<< 8;
                pak_len |=(win_data_buf[7] & 0xff);           
                pkt_active = 0;
                plot_putdec(pak_len); //debug
                }
            }

        if(rx_ptr == pak_len) //only process the data length
            {
          //  for(int wd=pak_len;wd < 1024+16; wd++)
             //   win_data_buf[wd]=0; //only for debug to clear remainder of display
            sync_flag = 1;
            pak_len = 0;
            rx_ptr = 0;
            }
        }//while tail!=head
    } //while(1) main loop

//STOPS HERE !
//=========================================


while(1)
    blinker();

}





//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>





