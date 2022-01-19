#ifndef _FONTS_H_
#define _FONTS_H_

#include "../include/types.h"

#define PSF1_MAGIC0     0x36
#define PSF1_MAGIC1     0x04


// the size of the header is always fixed to 32 bytes long
typedef struct bmp_header {
    unsigned short type;
    unsigned int size;
    unsigned int reserved;
    unsigned int offset;
} __attribute__((packed)) bmp_header;


typedef struct dib_header {
    unsigned int headerSize;
    int width;
    int height;
    unsigned short color_planes;
    unsigned short colorDepth;
    unsigned int compression;
    unsigned int image_size;
    int horizontal_resolution;
    int vertical_resolution;
    unsigned int color_num;
    unsigned int importantCol;
} __attribute__((packed)) dib_header;

int psf_init();
#endif