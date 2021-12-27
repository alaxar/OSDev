/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cmos.h"
#include "util/util.h"
#include "tasking/synchronisation.h"

#define CMOS_ADDRESS  0x70
#define CMOS_DATA     0x71

#define BIT_6_TO_0    0x7F

static mutex_t mutex = mutex_init();

uint8_t cmos_read(uint8_t offset) // Read byte from CMOS
{
    mutex_lock(&mutex);
    uint8_t tmp = inportb(CMOS_ADDRESS) & BIT(7);
    outportb(CMOS_ADDRESS, (tmp | (offset & BIT_6_TO_0))); // do not change bit7 (NMI mask)
    uint8_t retVal = inportb(CMOS_DATA);
    mutex_unlock(&mutex);
    return retVal;
}

void cmos_write(uint8_t offset, uint8_t val) // Write byte to CMOS
{
    mutex_lock(&mutex);
    uint8_t tmp = inportb(CMOS_ADDRESS) & BIT(7);
    outportb(CMOS_ADDRESS, (tmp | (offset & BIT_6_TO_0))); // do not change bit7 (NMI mask)
    outportb(CMOS_DATA, val);
    mutex_unlock(&mutex);
}


/*
* Copyright (c) 2009-2014 The PrettyOS Project. All rights reserved.
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
