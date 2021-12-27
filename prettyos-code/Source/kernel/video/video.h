#ifndef VIDEO_H
#define VIDEO_H

#include "os.h"
#include "storage/devicemanager.h"

#define COLUMNS 80
#define LINES   50
#define USER_BEGIN 2 // Reserving  Titlebar + Separation
#define USER_END   48 // Reserving Statusbar + Separation

//CRTC Address Register and the CRTC Data Register
#define CRTC_ADDR_REGISTER           0x3D4
#define CRTC_DATA_REGISTER           0x3D5

// Cursor Location Registers
#define CURSOR_LOCATION_HI_REGISTER  0xE
#define CURSOR_LOCATION_LO_REGISTER  0xF

#define VIDEORAM_BASE  0xA0000
#define VIDEORAM_SIZE  0x20000
#define TEXTMODUS_BASE 0xB8000

typedef struct
{
    uint16_t x, y;
} position_t;

typedef enum
{
    VM_TEXT, VM_VBE
} VIDEOMODES;


// These alias for colors are mandatory
#define ERROR          LIGHT_RED
#define SUCCESS        GREEN
#define HEADLINE       CYAN
#define TABLE_HEADING  LIGHT_GRAY
#define DATA           BROWN
#define IMPORTANT      YELLOW
#define TEXT           WHITE
#define FOOTNOTE       LIGHT_RED
#define TITLEBAR       LIGHT_RED


enum COLORS
{
    BLACK, BLUE,        GREEN,       CYAN,       RED,       MAGENTA,       BROWN,  LIGHT_GRAY,
    GRAY,  LIGHT_BLUE,  LIGHT_GREEN, LIGHT_CYAN, LIGHT_RED, LIGHT_MAGENTA, YELLOW, WHITE
};


struct diskType; // HACK
struct pciDev;


extern VIDEOMODES videomode;
extern bool autoRefresh;

bool vga_install(bool PrettyBL);
void vga_setPixel(uint8_t x, uint8_t y, uint16_t value);
void vga_fillLine(char c, uint8_t attr, uint16_t line);
void refreshScreen(void);
void refreshUserScreen(void);
void vga_clearScreen(void);
void kprintf(const char* message, uint32_t line, int attribute, ...);
void vga_updateCursor(void);
void vga_installPCIDevice(struct pciDev* dev);
uint8_t AsciiToCP437(uint8_t ascii);
void writeInfo(uint8_t line, const char* args, ...);
void saveScreenshot(const struct diskType* destinationType);
void takeScreenshot(void);


#endif
