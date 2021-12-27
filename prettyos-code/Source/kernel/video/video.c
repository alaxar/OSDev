/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "video.h"
#include "console.h"
#include "util/util.h"
#include "util/array.h"
#include "paging.h"
#include "kheap.h"
#include "filesystem/fsmanager.h"
#include "storage/devicemanager.h"
#include "tasking/synchronisation.h"
#include "vbe.h"
#include "pci.h"

#define SCREENSHOT_BYTES 4100


VIDEOMODES videomode = VM_TEXT;

static uint16_t* vidmem = (uint16_t*)TEXTMODUS_BASE;
static char infoBar[3][81] = {}; // Infobar with 3 lines and 80 columns

static position_t cursor = {0, 0};

static mutex_t videoLock = mutex_init();


bool vga_install(bool PrettyBL)
{
    vidmem = paging_allocMMIO(TEXTMODUS_BASE, 2);
    paging_setPhysMemCachingBehaviour(VIDEORAM_BASE, VIDEORAM_SIZE/PAGESIZE, MTRR_WRITECOMBINING);
    refreshScreen();

    if (!PrettyBL)
    {
        // Other bootloaders than PrettyBL do not set 80*50 text mode. Use the same function that is also used when leaving VBE modes.
        vbe_detect();
        vbe_leaveVideoMode(0);
    }

    return (vidmem != 0);
}

void vga_setPixel(uint8_t x, uint8_t y, uint16_t value)
{
    mutex_lock(&videoLock);
    vidmem[y*COLUMNS + x] = value;
    mutex_unlock(&videoLock);
}

void vga_clearScreen(void)
{
    mutex_lock(&videoLock);
    memsetw(vidmem, 0, COLUMNS * LINES);
    mutex_unlock(&videoLock);
}

void vga_fillLine(char c, uint8_t attr, uint16_t line)
{
    mutex_lock(&videoLock);
    uint16_t val = (uint16_t)c | (uint16_t)(attr << 8);
    memsetw(vidmem + line * COLUMNS, val, COLUMNS);
    mutex_unlock(&videoLock);
}

// http://de.wikipedia.org/wiki/Codepage_437
uint8_t AsciiToCP437(uint8_t ascii)
{
    switch (ascii)
    {
        case 0xE4:  return (0x84);  // ä
        case 0xF6:  return (0x94);  // ö
        case 0xFC:  return (0x81);  // ü
        case 0xDF:  return (0xE1);  // ß
        case 0xA7:  return (0x15);  // §
        case 0xB0:  return (0xF8);  // °
        case 0xC4:  return (0x8E);  // Ä
        case 0xD6:  return (0x99);  // Ö
        case 0xDC:  return (0x9A);  // Ü
        case 0xB2:  return (0xFD);  // ²
        case 0xB3:  return (0x00);  // ³ <-- not available
        case 0x80:  return (0xEE);  // € <-- Greek epsilon used
        case 0xB5:  return (0xE6);  // µ
        case 0xB6:  return (0x14);  // ¶
        case 0xC6:  return (0x92);  // æ
        case 0xE6:  return (0x91);  // Æ
        case 0xF1:  return (0xA4);  // ñ
        case 0xD1:  return (0xA5);  // Ñ
        case 0xE7:  return (0x87);  // ç
        case 0xC7:  return (0x80);  // Ç
        case 0xBF:  return (0xA8);  // ¿
        case 0xA1:  return (0xAD);  // ¡
        case 0xA2:  return (0x0B);  // ¢
        case 0xA3:  return (0x9C);  // £
        case 0xBD:  return (0xAB);  // ½
        case 0xBC:  return (0xAC);  // ¼
        case 0xA5:  return (0x9D);  // ¥
        case 0xF7:  return (0xF6);  // ÷
        case 0xAC:  return (0xAA);  // ¬
        case 0xAB:  return (0xAE);  // «
        case 0xBB:  return (0xAF);  // »
        default:    return ascii; // to be checked for more deviations
    }
}

static void kputch(char c, uint8_t attrib)
{
    uint8_t uc = AsciiToCP437((uint8_t)c); // no negative values

    uint16_t att = attrib << 8;

    switch (uc)
    {
        case 0x09: // tab: increment csr_x (divisible by 8)
            cursor.x = alignUp(cursor.x+1, 8);
            break;
        case '\r': // cr: cursor back to the margin
            cursor.x = 0;
            break;
        case '\n': // newline: like 'cr': cursor to the margin and increment csr_y
            cursor.x = 0; ++cursor.y;
            break;
        default:
        {
            uint16_t* pos = vidmem + (cursor.y * COLUMNS + cursor.x);
            *pos = uc | att; // character AND attributes: color
            ++cursor.x;
            break;
        }
    }

    if (cursor.x >= COLUMNS) // cursor reaches edge of the screen's width, a new line is inserted
    {
        cursor.x = 0;
        ++cursor.y;
    }
}

static void kputs(const char* text, uint8_t attrib)
{
    for (; *text; kputch(*text, attrib), ++text);
}

void kprintf(const char* message, uint32_t line, int attribute, ...)
{
    cursor.x = 0; cursor.y = line;

    va_list ap;
    va_start(ap, attribute);
    char buffer[32]; // Larger is not needed at the moment

    mutex_lock(&videoLock);
    for (; *message; message++)
    {
        switch (*message)
        {
            case '%':
                switch (*(++message))
                {
                    case 'u':
                        utoa(va_arg(ap, uint32_t), buffer);
                        kputs(buffer, attribute);
                        break;
                    case 'f':
                        ftoa(va_arg(ap, double), buffer);
                        kputs(buffer, attribute);
                        break;
                    case 'i': case 'd':
                        itoa(va_arg(ap, int32_t), buffer);
                        kputs(buffer, attribute);
                        break;
                    case 'X': /// TODO: make it standardized
                        i2hex(va_arg(ap, uint32_t), buffer, 8);
                        kputs(buffer, attribute);
                        break;
                    case 'x':
                        i2hex(va_arg(ap, uint32_t), buffer, 4);
                        kputs(buffer, attribute);
                        break;
                    case 'y':
                        i2hex(va_arg(ap, uint32_t), buffer, 2);
                        kputs(buffer, attribute);
                        break;
                    case 's':
                        kputs(va_arg(ap, char*), attribute);
                        break;
                    case 'c':
                        kputch((int8_t)va_arg(ap, int32_t), attribute);
                        break;
                    case 'v':
                        kputch(*(++message), (attribute >> 4) | (attribute << 4));
                        break;
                    case '%':
                        kputch('%', attribute);
                        break;
                    default:
                        --message;
                        break;
                }
                break;
            default:
                kputch(*message, attribute);
                break;
        }
    }
    mutex_unlock(&videoLock);
    va_end(ap);
}

static void refreshInfoBar(void)
{
    memset(vidmem + (USER_BEGIN + LINES - 7) * COLUMNS, 0, 3 * COLUMNS * 2); // Clearing info-area
    cursor.x = 0; cursor.y = 45;
    kputs(infoBar[0], YELLOW);
    cursor.x = 0; cursor.y = 46;
    kputs(infoBar[1], YELLOW);
    cursor.x = 0; cursor.y = 47;
    kputs(infoBar[2], YELLOW);
}

void writeInfo(uint8_t line, const char* args, ...)
{
    va_list ap;
    va_start(ap, args);
    vsnprintf(infoBar[line], 81, args, ap);
    va_end(ap);

    if (console_displayed->properties & CONSOLE_SHOWINFOBAR)
    {
        refreshInfoBar();
    }
}

void refreshUserScreen(void)
{
    mutex_lock(&videoLock);

    if (console_displayed->properties & CONSOLE_FULLSCREEN)
    {
        // copying content of visible console to the video-ram
        memcpy(vidmem, (void*)console_displayed->vidmem, COLUMNS*LINES*sizeof(uint16_t));
    }
    else if (console_displayed->properties & CONSOLE_SHOWINFOBAR)
    {
        // copying content of visible console to the video-ram
        memcpy(vidmem + USER_BEGIN * COLUMNS, (void*)console_displayed->vidmem, COLUMNS * (USER_END-USER_BEGIN-4) * 2);
        vga_fillLine('-', LIGHT_GRAY, 44); // Separation
        refreshInfoBar();
    }
    else
    {
        // copying content of visible console to the video-ram
        memcpy(vidmem + USER_BEGIN * COLUMNS, (void*)console_displayed->vidmem, COLUMNS * (USER_END-USER_BEGIN)*2);
    }

    mutex_unlock(&videoLock);
}

void refreshScreen(void)
{
    mutex_lock(&videoLock);

    refreshUserScreen(); // Reprint user area
    if (!(console_displayed->properties & CONSOLE_FULLSCREEN))
    {
        // Printing titlebar
        vga_fillLine(' ', WHITE, 0);
        kprintf("PrettyOS [Version %s]", 0, TITLEBAR, version);

        if (console_displayed->ID == KERNELCONSOLE_ID)
        {
            cursor.x = COLUMNS - 5;
            cursor.y = 0;
            kputs("Shell", TITLEBAR);
        }
        else
        {
            char Buffer[70];
            size_t length = snprintf(Buffer, 70, "Console %u: %s", console_displayed->ID + ((consoleStack_t*)console_currentStack->data)->base, console_displayed->name);
            cursor.x = COLUMNS - length;
            cursor.y = 0;
            kputs(Buffer, TITLEBAR);
        }
        vga_fillLine('-', LIGHT_GRAY, 1); // Separation on top
        vga_fillLine('-', LIGHT_GRAY, 48); // Separation on bottom
    }
    mutex_unlock(&videoLock);
}

void vga_updateCursor(void)
{
    uint16_t position = (console_displayed->cursor.y+2) * COLUMNS + console_displayed->cursor.x;
    // cursor HIGH port to vga INDEX register
    outportb(CRTC_ADDR_REGISTER, CURSOR_LOCATION_HI_REGISTER);
    outportb(CRTC_DATA_REGISTER, BYTE2(position));
    // cursor LOW port to vga INDEX register
    outportb(CRTC_ADDR_REGISTER, CURSOR_LOCATION_LO_REGISTER);
    outportb(CRTC_DATA_REGISTER, BYTE1(position));
}

void vga_installPCIDevice(pciDev_t* dev)
{
    uint16_t pciCommandRegister = pci_configRead(dev, PCI_COMMAND, 2);
    pci_configWrite_word(dev, PCI_COMMAND, pciCommandRegister | PCI_CMD_BUSMASTER);

    uint8_t cap;
    if (pci_getExtendedCapability(dev, 2, &cap)) // AGP capability
    {
        uint32_t agp_identifier = pci_configRead(dev, cap, 4);
        uint32_t agp_status = pci_configRead(dev, cap+4, 4);
        uint32_t agp_command = pci_configRead(dev, cap+8, 4);
      #ifdef _DIAGNOSIS_
        printf("\nAGP %u.%u:", (agp_identifier >> 20) & 0xF, (agp_identifier >> 16) & 0xF);
        printf("\n\tBefore initialization: status=%X, command=%X", agp_status, agp_command);
      #endif

        if (((agp_identifier >> 20) & 0xF) == 1 || ((agp_identifier >> 20) & 0xF) == 2 || (((agp_identifier >> 20) & 0xF) == 3 && !(agp_status & BIT(3))))
        {
            // We enable the lowest supported mode for compatibility, unless we find a way to find out what the mainboard supports
            if (agp_status & BIT(0)) // Enable 1X
                agp_command = (agp_command & 0xFFFFFFF8) | BIT(0);
            else if (agp_status & BIT(1)) // Enable 2X
                agp_command = (agp_command & 0xFFFFFFF8) | BIT(1);
            else if (agp_status & BIT(2)) // Enable 4X
                agp_command = (agp_command & 0xFFFFFFF8) | BIT(2);
            else
                return;
        }
        else if (((agp_identifier >> 20) & 0xF) == 3) // AGP 3.0
        {
            // We enable the lowest supported mode for compatibility, unless we find a way to find out what the mainboard supports
            if (agp_status & BIT(0)) // Enable 4X
                agp_command = (agp_command & 0xFFFFFFF8) | BIT(0);
            else if (agp_status & BIT(1)) // Enable 8X
                agp_command = (agp_command & 0xFFFFFFF8) | BIT(1);
            else
                return;
        }
        //if (agp_status & BIT(4)) // Enable Fast-Write
        //    agp_command |= BIT(4);
        if (((agp_identifier >> 20) & 0xF) == 3 && agp_status & BIT(3)) // Enable Side-Band adressing only (and always) in AGP 3.0
        //if (agp_status & BIT(9)) // Enable Side-Band adressing
                agp_command |= BIT(9);

        agp_command |= agp_status & 0xFF000000; // Set Request queue depth
        agp_command |= BIT(8); // Enable AGP

        pci_configWrite_dword(dev, cap + 8, agp_command);

      #ifdef _DIAGNOSIS_
        agp_command = pci_configRead(dev, cap + 8, 4);
        printf("\n\tAfter initialization: status=%X, command=%X", agp_status, agp_command);
      #endif
    }
}

static uint8_t* screenCache = 0;
void takeScreenshot(void)
{
    uint16_t NewLine = 0;

    mutex_lock(&videoLock);
    if (screenCache == 0) // If necessary, allocate screen cache
        screenCache = malloc(SCREENSHOT_BYTES, 0, "screenCache");
    for (uint16_t i=0; i<4000; i++)
    {
        uint16_t j=i+2*NewLine;
        screenCache[j] = *(const uint8_t*)(vidmem+i); // only signs, no attributes

        if (i % 80 == 79)
        {
            screenCache[j+1]= 0xD; // CR
            screenCache[j+2]= 0xA; // LF
            NewLine++;
        }
    }
    mutex_unlock(&videoLock);

    puts("\nTake screenshot. ");
}

extern array(disk_t*) disks; // HACK
void saveScreenshot(const diskType_t* destinationType)
{
    if (screenCache == 0)
        return; // No screenshot in memory

    char Pfad[20] = {0};

    for (int i = 0; i < disks.size; i++) // HACK
    {
        if (disks.data[i] && disks.data[i]->type == destinationType && (disks.data[i]->partition[0]->subtype == FS_FAT12 || disks.data[i]->partition[0]->subtype == FS_FAT16 || disks.data[i]->partition[0]->subtype == FS_FAT32))
        {
            snprintf(Pfad, 20, "%u:/screen.txt", i+1);
            break;
        }
    }

    if (Pfad[0] == 0)
        return; // No destination

    file_t* file = fopen(Pfad, "a+");

    if (file) // check for NULL pointer, otherwise #PF
    {
        fwrite(screenCache, 1, SCREENSHOT_BYTES, file);
        fclose(file);
    }
    else
    {
        puts("\nError: file could not be opened!");
    }
    free(screenCache);
    screenCache = 0;
}


/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
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
