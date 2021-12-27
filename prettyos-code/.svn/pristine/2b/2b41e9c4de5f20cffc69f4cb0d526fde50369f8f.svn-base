#ifndef VIDEOUTILS_H
#define VIDEOUTILS_H

#include "videomanager.h"
#include "util/util.h"


// RenderBuffer
typedef struct
{
    videoDevice_t* device;
    void*          buffer;
    void*          buffer2;
} renderBuffer_t;


// BMP
typedef struct
{
    uint16_t Type;            // File type. Set to "BM".
    uint32_t Size;            // Size in DWORDs of the file
    uint32_t Reserved;        // Reserved. Set to zero.
    uint32_t Offset;          // Offset to the data.
    uint32_t headerSize;      // Size of rest of header. Set to 40.
    uint32_t Width;           // Width of bitmap in pixels.
    uint32_t Height;          // Height of bitmap in pixels.
    uint16_t Planes;          // Number of Planes. Set to 1.
    uint16_t BitsPerPixel;    // Number of bits per pixel.
    uint32_t Compression;     // Compression. Usually set to 0.
    uint32_t SizeImage;       // Size in bytes of the bitmap.
    uint32_t XPixelsPerMeter; // Horizontal pixels per meter.
    uint32_t YPixelsPerMeter; // Vertical pixels per meter.
    uint32_t ColorsUsed;      // Number of colors used.
    uint32_t ColorsImportant; // Number of "important" colors.
} __attribute__((packed)) BMPHeader_t;


// Pre-defined colors
extern const BGRA_t black;
extern const BGRA_t white;


// Color conversions. Inlined to improve performance
static inline uint16_t BGRAtoBGR16(BGRA_t bgr)
{
    return (((bgr.red & 0xF8U)<<8) | ((bgr.green & 0xFCU)<<3) | (bgr.blue >> 3));
}
static inline uint16_t BGRAtoBGR15(BGRA_t bgr)
{
    return (((bgr.red & 0xF8U)<<7) | ((bgr.green & 0xF8U)<<2) | (bgr.blue >> 3));
}
static inline uint8_t BGRAtoBGR8(BGRA_t bgr)
{
    uint8_t r = (min((uint16_t)bgr.red + 16, 255) & 0xE0U);
    uint8_t g = (min((uint16_t)bgr.green + 16, 255) & 0xE0U) >> 3;
    uint8_t b = (min((uint16_t)bgr.blue + 32, 255) >> 6);
    return (r | g | b);
}

static inline BGRA_t BGR16toBGRA(uint16_t pixel)
{
    BGRA_t color = { (pixel & 0x3) << 6, (pixel & 0x1C) << 3, pixel & 0xE0, 0 };
    return color;
}
static inline BGRA_t BGR15toBGRA(uint16_t pixel)
{
    BGRA_t color = { (pixel & 0x3) << 6, (pixel & 0x1C) << 3, pixel & 0xE0, 0 };
    return color;
}
static inline BGRA_t BGR8toBGRA(uint8_t pixel)
{
    BGRA_t color =
    {
        2 + ((pixel & 0x3) << 6)  + (pixel & 0x3) * 20,
        2 + ((pixel & 0x1C) << 3) + ((pixel & 0x1C) >> 2) * 4,
        2 + (pixel & 0xE0)        + ((pixel & 0xE0) >> 5) * 4,
        0
    };
    return color;
}
BGRA_t YUVtoRGB(YUV_t yuv);
YUV_t  RGBtoYUV(BGRA_t rgb);

// Advanced and formatted drawing functionality
void video_drawLine(videoDevice_t* buffer, uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2, BGRA_t color);                 // Draws a line
void video_drawRect(videoDevice_t* buffer, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, BGRA_t color);       // Draws a rectancle
void video_drawRectFilled(videoDevice_t* buffer, uint16_t left, uint16_t top, uint16_t right, uint16_t bottom, BGRA_t color); // Draw a rectancle filled
void video_drawCartesianCircle(videoDevice_t* buffer, uint16_t xm, uint16_t ym, uint16_t radius, BGRA_t color);               // Draws a circle using cartesian coordinates algorithm
void video_drawChar(videoDevice_t* buffer, char c);                                                                           // Draws a character using font.h
void video_drawString(videoDevice_t* buffer, const char* text, uint16_t xpos, uint16_t ypos);                                 // Draws a string using video_drawChar
void video_drawBitmap(videoDevice_t* buffer, uint16_t xpos, uint16_t ypos, const BMPHeader_t* bitmap);                              // Draws a bitmap
void video_drawBitmapTransparent(videoDevice_t* buffer, uint16_t xpos, uint16_t ypos, const BMPHeader_t* bitmap, BGRA_t colorKey);  // Draws a bitmap, colorKey is handled as transparency
void video_drawScaledBitmap(videoDevice_t* buffer, uint16_t newSizeX, uint16_t newSizeY, const BMPHeader_t* bitmap);                // Scales a bitmap and draws it
void video_printPalette(videoDevice_t* device);                                                                               // Draws the palette used by device on device

renderBuffer_t* renderBuffer_create(uint16_t width, uint16_t height, uint8_t bitsPerPixel); // Creates a renderBuffer. It is recommended to use 32 bits per Pixel as color depth to increase performance
void renderBuffer_free(renderBuffer_t* buffer);
void renderBuffer_render(videoDevice_t* destination, renderBuffer_t* buffer, uint16_t x, uint16_t y); // Draw the content of the given renderBuffer to destination at the given position.

void   renderBuffer_setPixel(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t color);
void   renderBuffer_fillPixels(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t color, size_t num);
void   renderBuffer_copyPixels(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t* colors, size_t num);
void   renderBuffer_clear(videoDevice_t* device, BGRA_t color);
BGRA_t renderBuffer_getPixel(const videoDevice_t* device, uint16_t x, uint16_t y);
void   renderBuffer_flipScreen(videoDevice_t* device);

void showYUYV(videoDevice_t* device, uint16_t height, uint16_t width, uint8_t* data, size_t dataSize);


#endif
