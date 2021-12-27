#ifndef VBE_H
#define VBE_H

#include "videoutils.h"

// http://www.petesqbsite.com/sections/tutorials/tuts/vbe3.pdf
// http://poli.cs.vsb.cz/misc/rbint/text/1005.html

// SuperVGA information block
typedef struct
{
    char      VESASignature[4];
    uint16_t  VESAVersion;
    uintptr_t OEMStringPtr;
    uint8_t   Capabilities[4];
    uint16_t* VideoModes;
    uint16_t  TotalMemory;
    uint16_t  OemSoftwareRev;
    uintptr_t OemVendorNamePtr;
    uintptr_t OemProductNamePtr;
    uintptr_t OemProductRevPtr;
    //uint8_t   Reserved1[222];   // VgaInfoBlock is padded to 512 Bytes, but PrettyOS neither needs this padding nor OEM data
    //uint8_t   OemData[256];
} __attribute__((packed)) VgaInfoBlock_t;

// SuperVGA mode information block
typedef struct
{
    uint16_t   ModeAttributes;         // Mode attributes
    uint8_t    WinAAttributes;         // Window A attributes
    uint8_t    WinBAttributes;         // Window B attributes
    uint16_t   WinGranularity;         // Window granularity in k
    uint16_t   WinSize;                // Window size in k
    uint16_t   WinASegment;            // Window A segment
    uint16_t   WinBSegment;            // Window B segment

    void*      WinFuncPtr;             // Pointer to window function
    uint16_t   BytesPerScanLine;       // Bytes per scanline
    uint16_t   XResolution;            // Horizontal resolution
    uint16_t   YResolution;            // Vertical resolution
    uint8_t    XCharSize;              // Character cell width
    uint8_t    YCharSize;              // Character cell height
    uint8_t    NumberOfPlanes;         // Number of memory planes
    uint8_t    BitsPerPixel;           // Bits per pixel
    uint8_t    NumberOfBanks;          // Number of CGA style banks
    uint8_t    MemoryModel;            // Memory model type
    uint8_t    BankSize;               // Size of CGA style banks
    uint8_t    NumberOfImagePages;     // Number of images pages
    uint8_t    Reserved1;
    uint8_t    RedMaskSize;            // Size of direct color red mask
    uint8_t    RedFieldPosition;       // Bit posn of lsb of red mask
    uint8_t    GreenMaskSize;          // Size of direct color green mask
    uint8_t    GreenFieldPosition;     // Bit posn of lsb of green mask
    uint8_t    BlueMaskSize;           // Size of direct color blue mask
    uint8_t    BlueFieldPosition;      // Bit posn of lsb of blue mask
    uint8_t    RsvdMaskSize;           // Size of direct color res mask
    uint8_t    RsvdFieldPosition;      // Bit posn of lsb of res mask
    uint8_t    DirectColorModeInfo;    // Direct color mode attributes
    uintptr_t  PhysBasePtr;            // 32-bit physical memory address
    uint32_t   OffScreenMemOffset;     // pointer to start of off screen memory
    uint16_t   OffScreenMemSize;       // amount of off screen memory in 1k units
    //uint8_t   Reserved2[206];          // ModeInfoBlocked is padded to 512 Bytes, but PrettyOS does not need those padding bytes.
} __attribute__((packed)) ModeInfoBlock_t;

typedef struct
{
    videoDevice_t* device;
    void*          memory;
    uint8_t*       buffer1;
    uint8_t*       buffer2;
    uint8_t        paletteBitsPerColor;
} vbe_videoDevice_t;


size_t vbe_detect(void);
void   vbe_createDevice(videoDevice_t* device);
void   vbe_freeDevice(videoDevice_t* device);
void   vbe_createModeList(videoDevice_t* device, list_t* list);
void   vbe_freeMode(videoMode_t* mode);
void   vbe_enterVideoMode(videoMode_t* mode);
void   vbe_leaveVideoMode(videoDevice_t* device);
void   vbe_debug(void);

// Basic drawing functionality
void   vbe_setPixel(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t color);                 // Sets a single pixel on the screen
void   vbe_fillPixels(videoDevice_t* device, uint16_t x, uint16_t y, BGRA_t color, size_t num);   // Sets pixels on the screen
void   vbe_clear(videoDevice_t* device, BGRA_t color);
BGRA_t vbe_getPixel(const videoDevice_t* device, uint16_t x, uint16_t y);                         // Returns the color of a single pixel on the screen
void   vbe_flipScreen(videoDevice_t* device);


#endif
