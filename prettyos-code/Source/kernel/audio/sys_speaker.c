/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

// Enable pc-speaker-simulation in qemu: -soundhw pcspk

#include "sys_speaker.h"
#include "util/util.h"
#include "timer.h"
#include "pit.h"
#include "tasking/task.h"


void sound(uint32_t frequency)
{
    // Select channel, Access mode, Operating mode, BCD/Binary mode
    outportb(COMMANDREGISTER, COUNTER_2 | RW_LO_HI_MODE | SQUAREWAVE); // x86 offers only binary mode (no BCD)

    // calculate our divisor
    uint16_t divisor = TIMECOUNTER_i8254_FREQU / frequency; //divisor must fit into 16 bits; PIT (programable interrupt timer)

    // send divisor
    outportb(COUNTER_2_DATAPORT, BYTE1(divisor));
    outportb(COUNTER_2_DATAPORT, BYTE2(divisor));

    // sound on
    uint8_t temp = inportb(COUNTER_2_CONTROLPORT);
    if (temp != (temp | (AUX_GATE_2 | AUX_OUT_2)))
    {
        outportb(COUNTER_2_CONTROLPORT, temp | (AUX_GATE_2 | AUX_OUT_2));
    }

    currentTask->speaker = true;
}

void noSound(void)
{
    outportb(COUNTER_2_CONTROLPORT, inportb(COUNTER_2_CONTROLPORT) & ~(AUX_GATE_2 | AUX_OUT_2));
    currentTask->speaker = false;
}

void beep(uint32_t freq, uint32_t duration)
{
    sti();
    sound(freq);
    sleepMilliSeconds(duration);
    noSound();
}

void msgbeep(void)
{
    beep(440, 1000);
}

/*
* Copyright (c) 2009-2013 The PrettyOS Project. All rights reserved.
*
* http://www.c-plusplus.de/forum/viewforum-var-f-is-62.html
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
