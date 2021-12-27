/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "string.h"
#include "stdio.h"
#include "ctype.h"
#include "stdlib.h"


bool enabledEvents = false;


void event_flush(EVENT_t filter)
{
    EVENT_t ev;
    do
    {
        ev = event_poll(0, 0, filter);
    } while(ev != EVENT_NONE);
}

void sleep(uint32_t milliseconds)
{
    wait(BL_TIME, 0, milliseconds);
}

bool waitForTask(uint32_t pid, uint32_t timeout)
{
    return (wait(BL_TASK, (void*)pid, timeout));
}

void iSetCursor(uint16_t x, uint16_t y)
{
    position_t temp = {x, y};
    setCursor(temp);
}

char getchar(void)
{
    char ret = 0;
    EVENT_t ev = event_poll(&ret, 1, enabledEvents ? EVENT_TEXT_ENTERED : EVENT_NONE);

    while (ev != EVENT_TEXT_ENTERED)
    {
        if (ev == EVENT_NONE)
        {
            waitForEvent(0);
        }
        ev = event_poll(&ret, 1, enabledEvents ? EVENT_TEXT_ENTERED : EVENT_NONE);
    }
    return (ret);
}

uint32_t getCurrentSeconds(void)
{
    return (getCurrentMilliseconds()/1000);
}

int strncicmp(const char* s1, const char* s2, size_t n)
{
    if (n == 0) return (0);

    for (; *s1 && n > 1 && tolower(*s1) == tolower(*s2); n--)
    {
        ++s1;
        ++s2;
    }
    return (tolower(*s1) - tolower(*s2));
}

/// http://en.wikipedia.org/wiki/Itoa
void reverse(char* s)
{
    for (size_t i = 0, j = strlen(s)-1; i < j; i++, j--)
    {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

char* itoa(int32_t n, char* s)
{
    if (n < 0)
    {
        s[0] = '-';
        utoa(-n, s + 1);
        return s;
    }
    return utoa(n, s);
}

char* utoa(uint32_t n, char* s)
{
    uint32_t i = 0;
    do // generate digits in reverse order
    {
        s[i++] = n % 10 + '0'; // get next digit
    }
    while ((n /= 10) > 0);     // delete it
    s[i] = '\0';
    reverse(s);
    return (s);
}

void ftoa(float f, char* buffer)
{
    if (f < 0)
        *(buffer++) = '-';
    int32_t i = (int32_t)f;
    itoa(i < 0 ? -i : i, buffer);

    if (f < 0.0)
        f = -f;

    buffer += strlen(buffer);
    *buffer = '.';
    ++buffer;

    *buffer++ = ((uint32_t)(f * 10.f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 100.f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 1000.f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 10000.f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 100000.f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 1000000.f) % 10) + '0';
    *buffer   = '\0';
}

void i2hex(uint32_t val, char* dest, uint32_t len)
{
    char* cp = &dest[len];
    while (cp > dest)
    {
        char x = val & 0xF;
        val >>= 4;
        *--cp = x + ((x > 9) ? 'A' - 10 : '0');
    }
    dest[len]='\0';
}

static void scrollInfoLine(char* line1, char* line2, char* line3)
{
    char temp1 = line1[79];
    char temp2 = line2[79];
    char temp3 = line3[79];

    for (uint8_t i=79; i>0; --i)
    {
        line1[i] = line1[i-1];
        line2[i] = line2[i-1];
        line3[i] = line3[i-1];
    }
    line1[0] = temp1;
    line2[0] = temp2;
    line3[0] = temp3;
    printLine(line1,43,0xE);
    printLine(line2,44,0xE);
    printLine(line3,45,0xE);
}

void showInfo(uint8_t val)
{
    switch (val)
    {
        case 1:
        {
            static char* line1 = "   _______                __________      <>_<>                                 ";
            static char* line2 = "  (_______) |_|_|_|_|_|_|| [] [] [] | .---|'\"`|---.                             ";
            static char* line3 = " `-oo---oo-'`-oo-----oo-'`-oo----oo-'`o\"O-OO-OO-O\"o'                            ";
            scrollInfoLine(line1, line2, line3);
            break;
        }
        case 2:
        {
            static char* line1 = "       ___    ___    ___    ___                                                 ";
            static char* line2 = " ______\\  \\___\\  \\___\\  \\___\\  \\__________                                      ";
            static char* line3 = " \\ =  : : : : : : : : PrettyOS : : : : : /                                      ";
            scrollInfoLine(line1, line2, line3);
            break;
        }
    }
}


IP4_t stringToIP(char* str)
{
    IP4_t IP;
    for (uint8_t i_start = 0, i_end = 0, byte = 0; byte < 4; i_end++)
    {
        if (str[i_end] == 0)
        {
            IP.IP4[byte] = atoi(str+i_start);
            break;
        }
        if (str[i_end] == '.')
        {
            str[i_end] = 0;
            IP.IP4[byte] = atoi(str+i_start);
            i_start = i_end+1;
            byte++;
        }
    }
    return (IP);
}

// Should do: Uniformize path separators, simplify 1:0:/ to 1:/, simplify ../ sequences
bool squashPath(char* path)
{
    return true; // TODO
    /*char* oldSep = strpbrk(path, "/|\\");
    if (!oldSep)
    {
        free(path);
        return false;
    }
    char* sep = strpbrk(oldSep + 1, "/|\\");
    while (sep)
    {
        if (strncmp(sep + 1, "..", 2) == 0)
        {

        }
        oldSep = sep;
        sep = strpbrk(sep + 1, "/|\\");
    }*/
}

static bool isExistingDirectory(const char* path)
{
    folder_t* folder = folderAccess(path, FOLDER_OPEN);
    if (!folder)
        return false;

    folderClose(folder);
    return true;
}

bool changeWorkingDir(const char* subpath)
{
    if (strchr(subpath, ':') != 0) // Absolute path
    {
        if (!isExistingDirectory(subpath))
            return false;
        ipc_setString("~|workingDir", subpath);
    }
    else
    {
        // Check for special folders . or ..
        if (strcmp(subpath, ".") == 0)
            return true;

        else
        {
            size_t subpathLength = strlen(subpath);
            // Get old working dir
            char* path = malloc(20 + subpathLength + 2);
            size_t length = 20;
            IPC_ERROR err = ipc_getString("~|workingDir", path, &length);
            if (err == IPC_NOTENOUGHMEMORY)
            {
                size_t required = length;
                free(path);
                path = malloc(required + subpathLength + 2);
                err = ipc_getString("~|workingDir", path, &required);
            }

            if (err == IPC_SUCCESSFUL)
            {
                size_t pathLen = strlen(path);
                if (strcmp(subpath, "..") == 0)
                {
                    // Change into parent directory
                    if (pathLen == 0)
                        return false;

                    size_t separatorPos = pathLen - 1; //the last char may be a separator or not
                    while (separatorPos > 0)
                    {
                        if (strchr("/|\\", path[separatorPos]) != 0)
                        {
                            path[separatorPos + 1] = '\0';
                            break;
                        }
                        separatorPos--;
                    }
                    if (separatorPos == 0)
                        return false;
                }
                else
                {
                    if (strchr("/|\\", path[pathLen - 1]) == 0)
                        strcat(path, "|");
                    strcat(path, subpath);
                }

                if (squashPath(path))
                {
                    if (!isExistingDirectory(path))
                        return false;
                    ipc_setString("~|workingDir", path);
                }
                free(path);
            }
        }
    }
    return true;
}

char* getWorkingDir(void)
{
    char* path = 0;
    size_t length = 0;
    IPC_ERROR err = ipc_getString("~|workingDir", 0, &length);
    if (err == IPC_SUCCESSFUL)
    {
        path = malloc(length);
        err = ipc_getString("~|workingDir", path, &length);
    }

    if (err == IPC_SUCCESSFUL)
        return (path);
    else
        free(path);

    return (0);
}


/*
* Copyright (c) 2009-2016 The PrettyOS Project. All rights reserved.
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
