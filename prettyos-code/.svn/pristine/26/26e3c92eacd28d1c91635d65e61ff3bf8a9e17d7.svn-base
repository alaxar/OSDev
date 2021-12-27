/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

// http://wiki.osdev.org/Serial_ports

#include "serial.h"
#include "video/console.h"
#include "util/util.h"
#include "tasking/synchronisation.h"


static struct {
    mutex_t mutex;
    uint16_t port;  // Contains the port number used to access the interface
} COMports[4] =
{
    { mutex_init(), 0 },
    { mutex_init(), 0 },
    { mutex_init(), 0 },
    { mutex_init(), 0 }
};


void serial_init(void)
{
    textColor(LIGHT_GRAY);
    printf("\n   => Serial ports:");
    textColor(TEXT);

    // cf. http://www.lowlevel.eu/wiki/BDA
    uint8_t serialPorts = min(4, ((*(uint16_t*)BDA_EQUIPMENTWORD)>>9)&0x7); // Read from BIOS Data Area (BDA), bits 11-9: number of COM ports
    COMports[0].port = *((uint16_t*)BDA_COM1_BASE);
    COMports[1].port = *((uint16_t*)BDA_COM2_BASE);
    COMports[2].port = *((uint16_t*)BDA_COM3_BASE);
    COMports[3].port = *((uint16_t*)BDA_COM4_BASE);

    const uint32_t baudrate = 115200;
    const uint16_t divisor = 115200/baudrate;

    for (uint8_t i = 0; i < serialPorts; i++)
    {
        if (!COMports[i].port)
            continue;

        outportb(COMports[i].port + INTERRUPTENABLE, 0x00);             // Disable all interrupts
        outportb(COMports[i].port + LINECONTROL,     DLAB_BIT);         // Enable DLAB (set baud rate divisor)
        outportb(COMports[i].port + 0,               BYTE1(divisor));   // Set divisor lo byte (DO NOT TRY 0!!!)
        outportb(COMports[i].port + 1,               BYTE2(divisor));   // Set divisor hi byte
        outportb(COMports[i].port + LINECONTROL,     0x03);             // 8 bits (bit1:0: 8-5=3), no parity (bit5:3), one stop bit (bit2: 0), disable DLAB (bit7)
        outportb(COMports[i].port + FIFOCONTROL,     0xC7);             // Enable FIFO, clear them, with 14-byte threshold
        outportb(COMports[i].port + MODEMCONTROL,    0x0B);             // set OUT2, RTS, DSR

      #ifdef _DIAGNOSIS_
        textColor(LIGHT_GRAY);
        printf("\n     => COM %d:", i+1);
        printf("\n       => IO-port: ");
        textColor(TEXT);
        printf("%xh", COMports[i].port);
      #else
        printf(" COM%u", i + 1);
      #endif

    }

    putch('\n');
}

static bool received_atomic(uint8_t com)
{
    return inportb(COMports[com - 1].port + 5) & 1;
}

bool serial_received(uint8_t com)
{
    if (com <= 4 && COMports[com - 1].port)
    {
        mutex_lock(&COMports[com - 1].mutex);
        bool retVal = received_atomic(com);
        mutex_unlock(&COMports[com - 1].mutex);
        return retVal;
    }
    return false;
}

static char read_atomic(uint8_t com)
{
    while (received_atomic(com));
    return inportb(COMports[com - 1].port);
}

char serial_read(uint8_t com)
{
    if (com <= 4 && COMports[com - 1].port)
    {
        mutex_lock(&COMports[com - 1].mutex);
        char retVal = read_atomic(com);
        mutex_unlock(&COMports[com - 1].mutex);
        return retVal;
    }
    return (0);
}

static bool isTransmitEmpty_atomic(uint8_t com)
{
    return inportb(COMports[com - 1].port + 5) & 0x20;
}

bool serial_isTransmitEmpty(uint8_t com)
{
    if (com <= 4 && COMports[com - 1].port)
    {
        mutex_lock(&COMports[com - 1].mutex);
        bool retVal = isTransmitEmpty_atomic(com);
        mutex_unlock(&COMports[com - 1].mutex);
        return retVal;
    }
    return (true);
}

static void write_atomic(uint8_t com, char a)
{
    while (!isTransmitEmpty_atomic(com));
    outportb(COMports[com - 1].port, a);
}

void serial_write(uint8_t com, char a)
{
    if (com <= 4 && COMports[com - 1].port)
    {
        mutex_lock(&COMports[com - 1].mutex);
        write_atomic(com, a);
        mutex_unlock(&COMports[com - 1].mutex);
    }
}

void serial_vprintf(uint8_t com, const char* msg, va_list ap)
{
    size_t length = strlen(msg) + 100;
    char array[length]; // HACK: Should be large enough.
    vsnprintf(array, length, msg, ap);
    mutex_lock(&COMports[com - 1].mutex);
    for (size_t i = 0; i < length && array[i] != 0; i++)
    {
        if (array[i] == '\n') // Replace \n by \r\n for Windows compatibility
            write_atomic(com, '\r');
        write_atomic(com, array[i]);
    }
    mutex_unlock(&COMports[com - 1].mutex);
}

void serial_printf(uint8_t com, const char* msg, ...)
{
    va_list ap;
    va_start(ap, msg);
    serial_vprintf(com, msg, ap);
    va_end(ap);
}


/*
* Copyright (c) 2010-2015 The PrettyOS Project. All rights reserved.
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
