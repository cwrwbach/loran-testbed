#include "/pitaya_info.h" //Must be in System top Level directory
#include <stdlib.h>
#include <termios.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
//#include <pthread.h>
#include <sys/ioctl.h>
#include <linux/kd.h>
#include "pitaya_serv.h"

#define CENTRE_FREQ 100000 //100000;
#define SAMPLE_RATE 100000 //(Span width)
#define FILTER_WIDTH 0.1 
//#define SCOPE

#define SYNC_WORD 0x5A0FBE66
#define PKT_HEADER_LEN 16
#define PKT_DATA_LEN 1280
#define PKT_LEN 1280+16

//Global buffers
_Complex float resamp_out_buf[2560];
_Complex float pit_cpx_buf[2560];
_Complex float fft_out[1280];
float hamming_win[1280];




fftplan liq_fftq;
unsigned int fft_rate;

int fft_flag;
int scope_flag;

//Resampling
msresamp2_crcf decim_q;
int rso;
int lq_decim_stages;
int pitaya_master_freq;
int pitaya_sample_rate;

//Pointers to Pitaya FPGA registers
volatile uint64_t *rx_data, *tx_data;
volatile uint32_t *rx_freq, *tx_freq;
volatile uint16_t *rx_rate, *rx_cntr, *tx_rate, *tx_cntr;
volatile uint8_t *gpio, *rx_rst, *rx_sync, *tx_rst, *tx_sync;


//LPF coeffs
#define H_LEN 57
float h[H_LEN];
firfilt_crcf q; // = firfilt_crcf_create(h,h_len);
liquid_float_complex x;    // input sample
liquid_float_complex y;    // output sample

//misc
unsigned int timeout;
int update_flag;
static int do_exit = 0;
struct timespec now_time;
time_t start_time;

int fdk;
typedef struct input_event EV;
EV ev;

int shifter;

int spi_fd;
int tty_fd;
int hex_fd;

char uart_buf[PKT_LEN];
char * pkt_data_buf;

int d_cnt;

//Function prototypes
int set_interface_attribs(int, int);

int init_spi(void); 
void reset_pi(void);
void init_tty(void);
void do_header();
//===

int finito(char * msg)
    {
    printf(" %s \n",msg);
    return -1;
    }

//---

int open_keypad()
{
char dev_name[256];
printf(" Keypad open \n");

//open the keypad device
//strcpy(dev_name,"/dev/input/by-id/usb-SEM_HCT_Keyboard-event-if01");
strcpy(dev_name,"/dev/input/by-id/usb-SEM_HCT_Keyboard-event-kbd");

//usb-SEM_HCT_Keyboard-event-if01
//usb-SEM_HCT_Keyboard-event-kbd

fdk = open(dev_name, O_RDONLY| O_NONBLOCK);
if (fdk < 0) 
	{
	printf(" Failed to open keypad device\n");
	return -1;
    }            
// Flag it as exclusive access
if(ioctl( fdk, EVIOCGRAB, 1 ) < 0) 
	{
    printf( "evgrab not exclusive ioctl\n" );
	return -2;
    }

// if we get to here, we're connected to 
printf("Keypad device connected. dv: %d\n",fdk);
return 0;
}

//---

void do_fft()
{
int i;
double see_p, see_q;
double log_out;
static double log_fft[1024];
double hyp;

//Hamming window
for(int i=0;i<FFT_POINTS;i++) 
    pit_cpx_buf[i] *= hamming_win[i] * 4; //This is really just tweaking to calibrate gain of HW

fft_execute(liq_fftq);//data is from 'pit_cpx_buf'

for(i=0;i<FFT_POINTS; i++)
    {			
	see_p = creal(fft_out[i]/1024); //that is normalised to N
	see_q = cimag(fft_out[i]/1024);	
	hyp = hypot(see_p,see_q); // sqrt sum of sqrs
    if(hyp <0.0000005) //that is -126dB
        hyp = 0.0000005; //remove negative log spikess

    log_out = 20*log10(hyp);
    log_fft[i] -= 0.4f * (log_fft[i] - log_out );// 0.2 good, Smaller factor increase the average time
    //log_fft[i] = log_out; //BYPASS AVERAGE
	}		

//reorder the FFT. Units are 0.5*dB. 'Abs' because negative is assumed,
// sending e.g. 250 is actually neg125, therefore minimum level is 127dB below dBFS 
for(int i = 0;i<FFT_POINTS/2;i++)
  //  fft_xfer_buf[i] = (uint8_t) abs(log_fft[(FFT_POINTS/2)+i])*2; //0.5dB units
pkt_data_buf[i] = (uint8_t) abs(log_fft[(FFT_POINTS/2)+i])*2; //0.5dB units

for(int i = FFT_POINTS/2,j=0; i<FFT_POINTS;i++,j++)
   // fft_xfer_buf[i] = (uint8_t)abs(log_fft[j])*2; //0.5dB units, negative assumed
pkt_data_buf[i] = (uint8_t)abs(log_fft[j])*2; //0.5dB units, negative assumed

//temporary testing stuff...
pkt_data_buf[511] = 10;
pkt_data_buf[512] = 230;
}

//---

void do_scope()
{
int i;
double see_p, see_q;
//double log_out;
//static double log_fft[1024];
double hyp;

int val;


for(i=0;i<1280; i++) //Number of samples to plot
    {			
	see_p = creal(pit_cpx_buf[i]); //that is normalised to N
	see_q = cimag(pit_cpx_buf[i]);	
	hyp = hypot(see_p,see_q); // sqrt sum of sqrs
val = (int) (hyp*16384);
val*=-1;
val +=240;
    pkt_data_buf[i] = (uint8_t) val;

//float fred = cabsf(pit_cpx_buf[1]);
//float tom = cargf(pit_cpx_buf[1]);
 //  printf(" * %f - %d \n",hyp,val);
	}	

}




uart_send()
{
int err=write(tty_fd,&uart_buf, 1040); 
}
//---

void *rx_data_handler(void *arg) //Reads data from Pitaya FPGA 
{
int fft_timer;
union f8
    {
    uint64_t wide;
    float_t iq[2];
    _Complex float  cpx;
    } pit_rx;

fft_timer = 0;
fft_flag = 0;
//do register reset
*rx_rst |= 1;
*rx_rst &= ~1;

while(1) //loop forever
    {
   
    if(1) //timeout ==1) 
        { 
        if(*rx_cntr >= 8192)
            {
            *rx_rst |= 1;
            *rx_rst &= ~1;
            }
       // timeout++;
      //  if(timeout ==0)
       //         printf("\n *** TIMED OUT *** \n"); 

        while(*rx_cntr < 4096) 
            usleep(500); //loop waiting for a full register (double buffered in fpga)

        for(int i = 0; i < FFT_POINTS; ++i) 
            {
            pit_rx.wide = *rx_data; //this data is 64bit wide 
            pit_cpx_buf[i] = pit_rx.cpx ; //via the union (*4096 now done in audio)
           // timeout++;
            }
    
      //  do_audio(); // do audio before FFT

        //call FFT AFTER audio
        fft_timer++;
        if(fft_timer > 1) //q&d fft timer test (maybe better within 'do_fft()'? )
            {
            fft_flag = 0;
            fft_timer = 0;
            do_header();





#ifdef SCOPE

//+++++++
for(int nf = 0; nf < FFT_POINTS;nf++)
    {

    x=pit_cpx_buf[nf];
    firfilt_crcf_push(q, x);    // push input sample
    firfilt_crcf_execute(q,&y); // compute output
    pit_cpx_buf[nf] =  y;
    }
//+++++++++




            do_scope();
          
#else

            do_fft();
#endif
            uart_send();
            }
        }
    else 
        {
        sleep(1);
        }

    }

//should never get here
printf(" RX DATA ERROR Line %d \n",__LINE__);
return NULL;
}


void do_header()
{
//PKT_HEADER_LEN
//Setup the header buffer only one sync pulse needed, this to experiment  offset ???
uart_buf[0] = 0x5a;
uart_buf[1] = 0x0f;
uart_buf[2] = 0xbe;
uart_buf[3] = 0x66;


//Packet type + pak length
uart_buf[4] =0x33; //0x33 is temp code for fft win
uart_buf[5] =0x00;
uart_buf[6] = 0x04; //packet len...
uart_buf[7] =0x0f;  //of 1024

//Stamps time and sequence
uart_buf[8] = 0x38;
uart_buf[9] = 0x39;
uart_buf[10] = 0x3a;
uart_buf[11] = 0x3b; 

//Chek sums and the like
uart_buf[12] = 0x00;
uart_buf[13] = 0x00;
uart_buf[14] = 0x00;
uart_buf[15] = 0x00;
}


int init_spi()
{
spi_fd = open("/dev/spidev1.0", O_RDWR | O_NOCTTY);
if(spi_fd < 0)
    {
	printf("Error opening spidev1.0 \n");
	return -1;
	}
//int spi_speed = 4; //???
return 0;
}

void init_tty()
{
char *portname = "/dev/ttyPS1";

int wlen;
char *xstr = "Hello!\n";
int xlen = strlen(xstr);

tty_fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
if (tty_fd < 0) 
    {
    printf("Error opening tty\n");
//    return -1;
    }
    
//baudrate 115200, 8 bits, no parity, 1 stop bit
set_interface_attribs(tty_fd, B921600);
  //  set_mincount(fd, 10);                /* set to pure timed read */

//simple output */
wlen = write(tty_fd, xstr, xlen);

    if (wlen != xlen) {
        printf("Error from write \n");
    }
    tcdrain(tty_fd);  //WOTS ALL THIS THEN ?  //delay for output 
}

void reset_pi(void)
{
int err;
char dataz = 0x00;

if(init_spi() < 0)
	printf("Initialization of SPI failed \n");
err=write(spi_fd, &dataz, 1);
err=err;
close(spi_fd);
}

// ---Serial TTY--->
int set_interface_attribs(int fd, int speed)
{
struct termios tty;

if (tcgetattr(fd, &tty) < 0) 
    {
    printf("Error from tcgetattr \n");
    return -1;
    }

cfsetospeed(&tty, (speed_t)speed);
//cfsetispeed(&tty, (speed_t)speed);

tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
tty.c_cflag &= ~CSIZE;
tty.c_cflag |= CS8;         /* 8-bit characters */
tty.c_cflag &= ~PARENB;     /* no parity bit */
tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

// setup for non-canonical mode 
tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
tty.c_oflag &= ~OPOST;

// fetch bytes as they become available 
tty.c_cc[VMIN] = 1;
tty.c_cc[VTIME] = 1;

if (tcsetattr(fd, TCSANOW, &tty) != 0) 
    {
    printf("Error from tcsetattr \n");
    return -1;
    }
return 0;
}

void * keypad_event(void *keypad_thread_id)
{
int n;
//int *pstatus;
//int dn;
//char fred;

//dn = 0;

while(1)
    {
    usleep(10000); //anti cpu hogger
    //printf("looping \n");
    n=read(fdk,&ev,sizeof (ev));
    //printf (" n= %d\n",n);
  //  if(n > 0)
    if(0)
	    {
    printf(" Data is rxd from the Keypad N: %d \n",n);
    printf(" Data recd: *** %d \n",n);
    printf(" Type: %d Code: %d  Value: %d \n\n",ev.type,ev.code,ev.value);
        }
 

    if(ev.type == 1 && ev.value ==1)
        {
//printf(" Type: %d Code: %d  Value: %d \n\n",ev.type,ev.code,ev.value);

        switch (ev.code)
            {

            case 77 :
            printf("right\n");
            timeout=1;
            shifter +=100; //temp only lob
            break;

            case 75 :
            printf("left\n");
            timeout =0;
            shifter -=100; //temp only lob
            break;

            case 72 :
            printf("up\n");
            break;

            case 80 :
            printf("down\n");
            break;

            case 73 :
            printf("pg up\n");
            shifter +=1000; //temp only lob
            break;

            case 81 :
            printf("pg dwn\n");
            shifter -=1000; //temp only lob
            break;


            }
        printf(" Shifter: %d \n",shifter);
        }
    }
}

/*
case PAD_3 :
    digit_display[8] = '3';
    break;
case PAD_6 :
    dn--;
    if(dn < 0) dn=0;
    break;

case PAD_5 :
    digit_display[8] = '5';
    break;

case PAD_4 :
    dn++;
    if(dn >8) dn=8;

    digit_colour[dn] = RED;
    break;

case PAD_7 :
    digit_display[8] = '7';
    break;

case PAD_8 :
    digit_display[dn] ++;
    if(digit_display[dn] <0x30 ) digit_display[dn] = 0x30;

    break;

case PAD_9 :
    digit_display[8] = '9';

    break;

    }

*/
//==================================

void low_pass_filter(float width) 
{
    // parameters and simulation options
    unsigned int h_len =   57;  // filter length (samples)
    float        fc    = width ;  // normalized cutoff frequency
    unsigned int nfft  =  800;  // 'FFT' size (actually DFT)

    unsigned int i;
    unsigned int k;

    // design filter
 //   float h[h_len];
    for (i=0; i < h_len; i++) {
        // generate time vector, centered at zero
        float t = (float)i + 0.5f - 0.5f*(float)h_len;

        // generate sinc function (time offset in 't' prevents divide by zero)
        float s = sinf(2*M_PI*fc*t + 1e-6f) / (2*M_PI*fc*t + 1e-6f);

        // generate Hamming window
        float w = 0.53836 - 0.46164*cosf((2*M_PI*(float)i)/((float)(h_len-1)));

        // generate composite filter coefficient
        h[i] = s * w;
    }
}

//==================================

int main(int argc, char *argv[])
{
int flags=0;
pthread_t thread;
pthread_t keypad_thread_id;
volatile void *cfg, *sts;
int fd;
uint32_t pitaya_sr_command; 


int chan_number;
float in,out;
int err;



pkt_data_buf = & uart_buf[16];

//pkt_data_buf = & uart_buf + PKT_HEADER_LEN;

printf(" Starting Pitaya - Pi \n");

shifter = 50000; //temporary to test keyboard stuff.

err = open_keypad();
    pthread_create(&keypad_thread_id, NULL, keypad_event, NULL);

init_tty();
reset_pi(); //reset by sending an SPI byte, get sign-on message

// simple noncanonical input 
unsigned char buf[256];
int rdlen;

rdlen = read(tty_fd, buf, sizeof(buf) - 1);
if (rdlen > 0) 
    {
    buf[rdlen] = 0;
    printf("%s",buf);
    } 
else if (rdlen < 0) 
    {
    printf("Error from read: %d: %s\n", rdlen, strerror(errno));
    } 
sleep(1); //or whatever to let the Pi get ready

printf("\n get file \n");

hex_fd = open("aaa.hex",O_RDWR); //the target GUI file
if(hex_fd <0)
    printf("Image File not found \n");

char xxxccc;
int xxx_len;
int count = 0;

do
    {
    xxx_len = read(hex_fd,&xxxccc,1);
    count ++;
    err=write(tty_fd, & xxxccc, 1);
    } while( xxx_len > 0);

usleep(100000);

xxxccc= 'g';
err=write(tty_fd,&xxxccc, 1);
printf(" Done boot %d \n",count);

usleep(100000); //PI now running
in = 0.0; 
out= 0.0;
usleep(50000); //80000 good
 
for(d_cnt = 0;d_cnt<1000;d_cnt++)
        {
        out = sinf(in);
        in = in + (0.05 * M_PI);
        if(in > 2* M_PI) in = 0;
    
        pkt_data_buf[d_cnt] = (unsigned char) (out * 32)+ 96;
        pkt_data_buf[d_cnt] += rand() & 0x0f; //sdd noise
        }

for(int n = 0; n <200;n++)
        pkt_data_buf[n] = 0;    

for(int n = 200; n <300;n++)
        pkt_data_buf[n] = 75;  

for(int n = 300; n <400;n++)  
        pkt_data_buf[n] = 150;  

for(int n = 400; n <500;n++)
        pkt_data_buf[n] = 225;  

for(int n = 500; n <600;n++)
        pkt_data_buf[n] = 250;  

//err=write(tty_fd,pkt_data_buf, 1000);

if(argc == 1) 
    chan_number = CHAN_NUMBER; //use default if no argument

else 
    {
    if(atoi(argv[1]) == 1) chan_number = 1;
    else if(atoi(argv[1]) == 2) chan_number = 2;
    else chan_number = CHAN_NUMBER; //default
    }

printf("PITAYA server using chan: %d \n",chan_number);
rso = 0; //resample output pointer
update_flag = 0;

for(int i=0;i<FFT_POINTS;i++) //make a Hamming window for fft
    hamming_win[i] = liquid_hamming(i,FFT_POINTS);

liq_fftq = fft_create_plan(FFT_POINTS, pit_cpx_buf, fft_out, LIQUID_FFT_FORWARD, flags);

#ifdef PIT16
printf("This is a 16 bit Red Pitaya\n");
//audio_rate = 8000; //audio_rate = 12000;
pitaya_sample_rate = 512000; //768000;
pitaya_master_freq = 61440000;
#else
printf("This is a 14 bit Red Pitaya\n");
//audio_rate = 7812;
pitaya_sample_rate = SAMPLE_RATE; //20000; //250000; //500000;
pitaya_master_freq = 62500000;
#endif


//make coeffs
low_pass_filter(FILTER_WIDTH);

unsigned int h_len=H_LEN;
   // create filter object
//firfilt_crcf 
q = firfilt_crcf_create(h,h_len);
//liquid_float_complex x;    // input sample
//liquid_float_complex y;    // output sample




//set sample rate in pitaya FPGA
pitaya_sr_command = pitaya_master_freq/pitaya_sample_rate; 

fft_rate=0;

if((fd = open("/dev/mem", O_RDWR)) < 0)
    {
    perror("open");
    return EXIT_FAILURE;
    }

switch(chan_number)
    {
    case 1:
      cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40001000);
      sts = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40002000);
      rx_data = mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40010000);
      tx_data = mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40018000);
      break;
    case 2:
      cfg = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40003000);
      sts = mmap(NULL, sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40004000);
      rx_data = mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40020000);
      tx_data = mmap(NULL, 8*sysconf(_SC_PAGESIZE), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0x40028000);
      break;
    }

rx_rst  = ((uint8_t *)(cfg + 0));
rx_freq = ((uint32_t *)(cfg + 4));
rx_sync = ((uint8_t *)(cfg + 8));
rx_rate = ((uint16_t *)(cfg + 10));
rx_cntr = ((uint16_t *)(sts + 0));

*rx_rate = pitaya_sr_command & 0x0000ffff;

float pitaya_xtal;
pitaya_xtal = (float) pitaya_master_freq*2 ; //Master F is divided by 2 in FPGA

float fstartup_freq = CENTRE_FREQ; //5000000;
*rx_freq = (uint32_t)floor(fstartup_freq/pitaya_xtal*(1<<30)+0.5);

timeout = 0 ;// 000; //Needed - sets a keep alive

//start the rx thread
if(pthread_create(&thread, NULL,rx_data_handler , NULL) < 0)
   printf("pthread_create");

while( 1)
    {
    //my goood stuff here
        usleep(100);
  //  usleep(shifter*10);
    fft_flag =1;
    }

err=err;
while (!do_exit) 
    {
    sleep(1);
    } //while no exit
firfilt_crcf_destroy(q);
} //end main

//=======================================================
