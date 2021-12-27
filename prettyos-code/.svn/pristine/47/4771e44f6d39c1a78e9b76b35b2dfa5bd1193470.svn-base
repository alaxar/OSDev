/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "log.h"
#include "video/console.h"
#include "util/list.h"
#include "tasking/synchronisation.h"
#include "serial.h"
#include "ipc.h"

#define LOG_BUFFER_SIZE 4096

// Configure screen log
bool log_screen = true;
static LOG_LEVEL log_screenLevel = LOG_IMPORTANT;
static LOG_FOCUS log_screenFocus = FOCUS_NONE;
static mutex_t log_screenMutex = mutex_init();

// Configure secondary log
bool log_secondary = false;
static LOG_LEVEL log_secondaryLevel = LOG_NORMAL;
static LOG_FOCUS log_secondaryFocus = FOCUS_NONE;
static LOG_MODE log_secondaryMode = MODE_BUFFERED;
LOG_DEVICE log_secondaryDevice = LOG_DEVICE_SERIAL;
static list_t log_secondaryBuffers = list_init();
static size_t log_secondaryPos = LOG_BUFFER_SIZE;
static mutex_t log_secondaryMutex = mutex_init();
static file_t* log_file = 0;

static LOG_FOCUS charToFocus(char c)
{
    switch (c)
    {
    case 'u':
        return FOCUS_USB;
    case 'h':
        return FOCUS_USB_HC;
    case 'f':
        return FOCUS_FS;
    case 'd':
        return FOCUS_DISK;
    default:
        return FOCUS_NONE;
    }
}
static char focusToChar(LOG_FOCUS focus)
{
    switch (focus)
    {
    case FOCUS_USB:
        return 'u';
    case FOCUS_USB_HC:
        return 'h';
    case FOCUS_FS:
        return 'f';
    case FOCUS_DISK:
        return 'd';
    case FOCUS_NONE: default:
        return '-';
    }
}

void log_incrementScreenLevel(void)
{
    mutex_lock(&log_screenMutex);
    log_screenLevel++;
    if (log_screenLevel == __LOG_LEVEL_END)
        log_screenLevel = 0;
    mutex_unlock(&log_screenMutex);
    log_showScreenStatus();
}
void log_decrementScreenLevel(void)
{
    mutex_lock(&log_screenMutex);
    if (log_screenLevel == 0)
        log_screenLevel = __LOG_LEVEL_END;
    log_screenLevel--;
    mutex_unlock(&log_screenMutex);
    log_showScreenStatus();
}
void log_setScreenFocus(char c)
{
    log_screenFocus = charToFocus(c);
    log_showScreenStatus();
}

void log_incrementSecondaryLevel(void)
{
    mutex_lock(&log_secondaryMutex);
    log_secondaryLevel++;
    if (log_secondaryLevel == __LOG_LEVEL_END)
        log_secondaryLevel = 0;
    mutex_unlock(&log_secondaryMutex);
    log_showSecondaryStatus();
}
void log_decrementSecondaryLevel(void)
{
    mutex_lock(&log_secondaryMutex);
    if (log_secondaryLevel == 0)
        log_secondaryLevel = __LOG_LEVEL_END;
    log_secondaryLevel--;
    mutex_unlock(&log_secondaryMutex);
    log_showSecondaryStatus();
}
void log_toggleSecondaryMode(void)
{
    mutex_lock(&log_secondaryMutex);
    if (log_secondaryMode == MODE_BUFFERED)
        log_secondaryMode = MODE_REALTIME;
    else
        log_secondaryMode = MODE_BUFFERED;
    mutex_unlock(&log_secondaryMutex);
    log_showSecondaryStatus();
}
void log_setSecondaryFocus(char c)
{
    log_secondaryFocus = charToFocus(c);
    log_showSecondaryStatus();
}

void log_flushSecondary(void)
{
    mutex_lock(&log_secondaryMutex);

    // Write buffer content
    if (log_secondaryDevice == LOG_DEVICE_SERIAL)
    {
        for (dlelement_t* e = log_secondaryBuffers.head; e != log_secondaryBuffers.tail; e = e->next)
        {
            for(size_t i = 0; i < LOG_BUFFER_SIZE; i++)
                serial_write(SER_LOG, ((uint8_t*)e->data)[i]);
        }
        for (size_t i = 0; i < log_secondaryPos; i++)
            serial_write(SER_LOG, ((uint8_t*)log_secondaryBuffers.tail->data)[i]);
    }
    else if (log_secondaryDevice == LOG_DEVICE_FILE)
    {
        if (log_file == 0)
        {
            const char* filename = "1:|log.txt";
            ipc_getConstString("/PrettyOS/Log/Filename", &filename);
            log_file = fopen(filename, "w");
        }

        for (dlelement_t* e = log_secondaryBuffers.head; e != log_secondaryBuffers.tail; e = e->next)
            fwrite(e->data, LOG_BUFFER_SIZE, 1, log_file);
        fwrite(log_secondaryBuffers.tail->data, log_secondaryPos, 1, log_file);

        fclose(log_file);
        log_file = 0;
    }
    else if (log_secondaryDevice == LOG_DEVICE_TCP)
        ; // TODO: Buffered output to TCP

    // Release old buffers
    log_secondaryPos = LOG_BUFFER_SIZE;
    list_destruct(&log_secondaryBuffers);

    mutex_unlock(&log_secondaryMutex);
}

void log_showScreenStatus(void)
{
    if (log_screen)
        writeInfo(2, "screen: level=%u, focus=%c", log_screenLevel, focusToChar(log_screenFocus));
    else
        writeInfo(2, "screen: off");
}

void log_showSecondaryStatus(void)
{
    if (log_secondary)
        writeInfo(2, "secondary: level=%u, focus=%c, mode=%c, device=%u", log_secondaryLevel, focusToChar(log_secondaryFocus), (log_secondaryMode == MODE_REALTIME ? 'r' : 'b'), log_secondaryDevice);
    else
        writeInfo(2, "secondary: off");
}

void log_vprintf(LOG_LEVEL level, LOG_FOCUS focus, const char* format, va_list args)
{
    // Screen log
    if (log_screen && ((level <= log_screenLevel && (log_screenFocus == FOCUS_NONE || focus == log_screenFocus)) || level+2 <= log_screenLevel))
    {
        mutex_lock(&log_screenMutex);
        vprintf(format, args); // Forward to console
        mutex_unlock(&log_screenMutex);
    }

    // Secondary log
    if (log_secondary && ((level <= log_secondaryLevel && (log_secondaryFocus == FOCUS_NONE || focus == log_secondaryFocus)) || level + 2 <= log_secondaryLevel))
    {
        mutex_lock(&log_secondaryMutex);

        if (log_secondaryMode == MODE_REALTIME)
        {
            if (log_secondaryDevice == LOG_DEVICE_SERIAL)
                serial_vprintf(SER_LOG, format, args);
            else if (log_secondaryDevice == LOG_DEVICE_FILE)
            {
                if (log_file == 0)
                {
                    const char* filename = "1:|log.txt";
                    ipc_getConstString("/PrettyOS/Log/Filename", &filename);
                    log_file = fopen(filename, "w");
                }

                if (log_file != 0)
                {
                    size_t length = strlen(format) + 1000;
                    char array[length]; // HACK: Should be large enough. Better implement and use vfprintf
                    vsnprintf(array, length, format, args);
                    fputs(array, log_file);
                    fflush(log_file);
                }
            }
            else if (log_secondaryDevice == LOG_DEVICE_TCP)
                ; // TODO: Realtime output to tcp
        }
        else // buffered mode
        {
            size_t length = strlen(format) + 1000;
            char array[length]; // HACK: Should be large enough.
            length = vsnprintf(array, length, format, args);
            size_t pos = 0;

            while (length != 0)
            {
                if (log_secondaryPos >= LOG_BUFFER_SIZE)
                {
                    list_append_elem(&log_secondaryBuffers, list_alloc_elem(LOG_BUFFER_SIZE, "log buffer"));
                    log_secondaryPos = 0;
                }
                size_t count = min(length, LOG_BUFFER_SIZE - log_secondaryPos);
                memcpy(log_secondaryBuffers.tail->data, array + pos, count);
                length -= count;
                pos += count;
            }
        }

        mutex_unlock(&log_secondaryMutex);
    }
}

void log_printf(LOG_LEVEL level, LOG_FOCUS focus, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    log_vprintf(level, focus, format, args);
    va_end(args);
}

void log_printfc(LOG_LEVEL level, LOG_FOCUS focus, uint8_t attr, const char* format, ...)
{
    uint8_t oldAttr = getTextColor();
    textColor(attr);
    va_list args;
    va_start(args, format);
    log_vprintf(level, focus, format, args);
    va_end(args);
    textColor(oldAttr);
}


/*
* Copyright (c) 2015-2016 The PrettyOS Project. All rights reserved.
*
* http://www.prettyos.de
*
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice,
*    this list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright
*    notice, this list of conditions and the following disclaimer in the
*    documentation and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
* TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
* PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
* CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
* PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
* OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
* OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
* ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
