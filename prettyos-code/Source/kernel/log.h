#ifndef LOG_H
#define LOG_H

#include "os.h"


typedef enum
{
    LOG_DETAIL, LOG_NORMAL, LOG_IMPORTANT, LOG_ERROR,
    __LOG_LEVEL_END
} LOG_LEVEL;

typedef enum
{
    FOCUS_NONE, FOCUS_USB, FOCUS_USB_HC, FOCUS_FS, FOCUS_DISK,
    __LOG_FOCUS_END
} LOG_FOCUS;

typedef enum
{
    MODE_BUFFERED, MODE_REALTIME,
    __LOG_MODE_END
} LOG_MODE;

typedef enum
{
    LOG_DEVICE_FILE, LOG_DEVICE_SERIAL, LOG_DEVICE_TCP
} LOG_DEVICE;


// Configure screen log
extern bool log_screen;
void log_incrementScreenLevel(void);
void log_decrementScreenLevel(void);
void log_setScreenFocus(char c);

// Configure secondary log
extern bool log_secondary;
extern LOG_DEVICE log_secondaryDevice;

void log_incrementSecondaryLevel(void);
void log_decrementSecondaryLevel(void);
void log_toggleSecondaryMode(void);
void log_setSecondaryFocus(char c);
void log_flushSecondary(void);

// Show log status
void log_showScreenStatus(void);
void log_showSecondaryStatus(void);

// Write to log
// These functions will print, store or forget the string depending on level and focus.
// If focus does not match or if level is not sufficient, information will be dropped.
void log_vprintf(LOG_LEVEL level, LOG_FOCUS focus, const char* format, va_list args);
void log_printf(LOG_LEVEL level, LOG_FOCUS focus, const char* format, ...);
void log_cprintf(LOG_LEVEL level, LOG_FOCUS focus, uint8_t attr, const char* format, ...);


#endif
