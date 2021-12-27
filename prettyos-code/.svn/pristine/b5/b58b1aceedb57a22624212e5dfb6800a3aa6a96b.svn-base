#ifndef CONSOLE_H
#define CONSOLE_H

#include "video.h"
#include "tasking/synchronisation.h"
#include "util/list.h"


#define KERNELCONSOLE_ID 11


typedef enum
{
    CONSOLE_FULLSCREEN  = 1,
    CONSOLE_SHOWINFOBAR = 2,
    CONSOLE_AUTOREFRESH = 4,
    CONSOLE_AUTOSCROLL  = 8
} console_properties_t;

typedef struct // Defines the User-Space of the display
{
    dlelement_t* stack; // Stack this console is part of
    char*      name;
    console_properties_t properties;
    position_t cursor;
    mutex_t    mutex;
    list_t     tasks;
    uint8_t    ID; // Number of the console in stack
    uint8_t    scrollBegin;
    uint8_t    scrollEnd;
    uint16_t   vidmem[LINES*COLUMNS]; // Memory that stores the content of this console. Size is USER_LINES*COLUMNS
} console_t;

typedef struct {
    console_t* array[10];
    uint16_t   base;
} consoleStack_t;


extern console_t* volatile console_displayed;
extern console_t*          console_current;
extern console_t           kernelConsole;
extern dlelement_t*        console_currentStack;


void kernel_console_init(void);
void console_init(console_t* console, const char* name);
void console_exit(console_t* console);
void console_cleanup(task_t* task);
bool console_display(uint8_t ID);
void console_incrementStack(void);
void console_decrementStack(void);
void console_foldStack(void);

void console_clear(uint8_t backcolor);
void textColor(uint8_t color); // bit 4-7: background; bit 1-3: foreground
uint8_t getTextColor(void);
void console_setProperties(console_properties_t properties);
void setScrollField(uint8_t begin, uint8_t end);
void console_setPixel(uint8_t x, uint8_t y, uint16_t value);
void putch(char c);
void puts(const char* text);
size_t printf (const char* args, ...);
size_t printfe(const char* args, ...);
size_t vprintf(const char* args, va_list ap);
size_t cprintf(const char* message, uint16_t line, int attribute, ...);
void setCursor(position_t pos);
void getCursor(position_t* pos);


#endif
