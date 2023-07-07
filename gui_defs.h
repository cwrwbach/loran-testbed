#pragma once

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;

#define IMAGE_WIDTH 1280 
#define IMAGE_HEIGHT 768
#define IMAGE_SIZE (IMAGE_WIDTH*IMAGE_HEIGHT)

#define MAX_PAK_LEN 4095

#define FFT_WIN_WIDTH 1280
#define FFT_WIN_HEIGHT 250 
#define FFT_WIN_LEFT   0 
#define FFT_WIN_TOP 10 
#define FFT_HORIZ_SPACE 100//space between each grid line

#define PKT_HEADER_LEN 16
#define PKT_DATA_LEN 1024

#define BLACK   0x00
#define RED     0x01
#define GREEN   0x02
#define BLUE    0x03
#define MAGENTA 0x04
#define YELLOW  0x05
#define CYAN    0x06
#define DARK_GREEN 0x07
#define GRID_COL    0x08

//#define HIGH_WHITE   0xff
