/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "ac97_via.h"
#include "util/util.h"
#include "video/console.h"
#include "paging.h"
#include "timer.h"
#include "kheap.h"


static void codec_sendCommand(ac97Via_t* ac97, uint8_t reg, bool primary, uint16_t data)
{
    WAIT_FOR_CONDITION(!(inportl(ac97->iobase + AC97_VIA_ACCESS_CODEC) & AC97_VIA_CODEC_BUSY), 10, 10, "\nAC97: Controller is busy.");
    if (primary)
        outportl(ac97->iobase + AC97_VIA_ACCESS_CODEC, AC97_VIA_CODEC_SEL_PRIM | AC97_VIA_CODEC_WRITE | ((uint32_t)reg << 16) | data);
    else
        outportl(ac97->iobase + AC97_VIA_ACCESS_CODEC, AC97_VIA_CODEC_SEL_SEC  | AC97_VIA_CODEC_WRITE | ((uint32_t)reg << 16) | data);
}

static uint16_t codec_readStatus(ac97Via_t* ac97, uint8_t reg, bool primary)
{
    WAIT_FOR_CONDITION(!(inportl(ac97->iobase + AC97_VIA_ACCESS_CODEC) & AC97_VIA_CODEC_BUSY), 10, 10, "\nAC97: Controller is busy.");
    if (primary)
    {
        outportl(ac97->iobase + AC97_VIA_ACCESS_CODEC, AC97_VIA_CODEC_SEL_PRIM | AC97_VIA_CODEC_READ | AC97_VIA_CODEC_PRIM_VALID | ((uint32_t)reg << 16));
        WAIT_FOR_CONDITION(inportl(ac97->iobase + AC97_VIA_ACCESS_CODEC) & AC97_VIA_CODEC_PRIM_VALID, 10, 10, "\nAC97: Controller does not provide valid data.");
    }
    else
    {
        outportl(ac97->iobase + AC97_VIA_ACCESS_CODEC, AC97_VIA_CODEC_SEL_SEC | AC97_VIA_CODEC_READ | AC97_VIA_CODEC_SEC_VALID | ((uint32_t)reg << 16));
        WAIT_FOR_CONDITION(inportl(ac97->iobase + AC97_VIA_ACCESS_CODEC) & AC97_VIA_CODEC_SEC_VALID, 10, 10, "\nAC97: Controller does not provide valid data.");
    }
    return inportl(ac97->iobase + AC97_VIA_ACCESS_CODEC) & 0xFFFF;
}

void ac97Via_install(pciDev_t* device)
{
    ac97Via_t* ac97 = malloc(sizeof(ac97Via_t), 0, "ac97Via");
    ac97->device = device;
    ac97->device->data = ac97;

    //irq_installPCIHandler(device->irq, AC97_handler, device);

    uint16_t pciCommandRegister = pci_configRead(device, PCI_COMMAND, 2);
    pci_configWrite_word(device, PCI_COMMAND, pciCommandRegister | PCI_CMD_IO);

    ac97->iobase = (uint16_t)device->bar[0].baseAddress;

    // Ensure AC Link being enabled
    uint8_t interfaceControl = pci_configRead(device, 0x41, 1);
    if (!(interfaceControl & BIT(7)) || !(interfaceControl & BIT(2)))
        pci_configWrite_byte(device, 0x41, interfaceControl | BIT(7) | BIT(2) | BIT(6) | BIT(3));

    WAIT_FOR_CONDITION(pci_configRead(device, 0x40, 1) & BIT(0), 10, 10, "\nAC97: Codec not ready");

    /*// Reset card (Linux driver does so)?
    outportb(ac97->iobase + AC97_VIA_R_SGD_CONTROL, BIT(6)|BIT(3)|BIT(0));
    inportb(ac97->iobase + AC97_VIA_R_SGD_CONTROL);
    sleepMilliSeconds(50);
    // disable interrupts
    outportb(ac97->iobase + AC97_VIA_R_SGD_CONTROL, 0);
    // clear interrupts
    outportb(ac97->iobase + 0, 0x03);
    outportb(ac97->iobase + AC97_VIA_R_SGD_TYPE, 0x00);*/

    // Initialize AC97 codec
    codec_sendCommand(ac97, AC97_RESET, true, 42); // Reset (any value)
    sleepMilliSeconds(100); // Give it some time to reset (not sure if necessary)

    // Set volume
    uint8_t volume = 0; // As loud as possible
    codec_sendCommand(ac97, AC97_MASTER_VOLUME, true, (volume << 8) | volume);  // General volume left and right
    codec_sendCommand(ac97, AC97_MONO_VOLUME, true, volume);                    // Volume for Mono
    codec_sendCommand(ac97, AC97_PC_BEEP_VOLUME, true, volume);                 // Volume for PC speaker
    codec_sendCommand(ac97, AC97_PCM_OUT_VOLUME, true, (volume << 8) | volume); // Volume for PCM left and right

    if (!(codec_readStatus(ac97, AC97_EXT_AUDIO_ID, true) & 1))
    {
        printf("\nSample rate fixed to %u Hz.\n", codec_readStatus(ac97, AC97_FRONT_DAC_RATE, true));
    }
    else
    {
        codec_sendCommand(ac97, AC97_EXT_AUDIO_STS_CTRL, true, codec_readStatus(ac97, AC97_EXT_AUDIO_STS_CTRL, true) | 1); // Activate variable rate audio
        sleepMilliSeconds(10);
        codec_sendCommand(ac97, AC97_FRONT_DAC_RATE, true, 44100); // General sample rate
        codec_sendCommand(ac97, AC97_LR_ADC_RATE, true, 44100); // Stereo  sample rate
        sleepMilliSeconds(10);
        printf("\nSample rate set to %u Hz.\n", codec_readStatus(ac97, AC97_FRONT_DAC_RATE, true));
    }

    // Setup SGD table
    ac97->numDesc = 32;
    ac97->SGDtable = malloc(sizeof(ac97Via_SGDEntry_t) * ac97->numDesc, PAGESIZE | HEAP_CONTINUOUS, "AC97 SGD table");
    ac97->buffers = malloc(AC97_VIA_SAMPLES_PER_BUF * 2 * ac97->numDesc, PAGESIZE | HEAP_CONTINUOUS, "AC97 SGD buffers");
    for (size_t i = 0; i < ac97->numDesc; i++)
    {
        ac97->SGDtable[i].buf = paging_getPhysAddr(ac97->buffers + AC97_VIA_SAMPLES_PER_BUF * 2 * i);
        ac97->SGDtable[i].len = AC97_VIA_SAMPLES_PER_BUF;
        ac97->SGDtable[i].reserved = 0;
        ac97->SGDtable[i].flag = 0;
        ac97->SGDtable[i].stop = 0;
        if (i == ac97->numDesc-1)
            ac97->SGDtable[i].eol = 1;
        else
            ac97->SGDtable[i].eol = 0;
    }

    outportb(ac97->iobase + AC97_VIA_R_SGD_TYPE, BIT(7) | BIT(5) | BIT(4));
    //outportb(ac97->iobase + AC97_VIA_W_SGD_TYPE, BIT(7) | BIT(5) | BIT(4));
    outportl(ac97->iobase + AC97_VIA_R_SGD_TABLE_BASE, paging_getPhysAddr(ac97->SGDtable));
    //outportl(ac97->iobase + AC97_VIA_W_SGD_TABLE_BASE, paging_getPhysAddr(ac97->SGDtable)); // Set SGD Table Pointer Base
}

void ac97Via_start(ac97Via_t* ac97)
{
    bool tick = false;
    for (size_t i = 0; i < AC97_VIA_SAMPLES_PER_BUF * ac97->numDesc; i++)
    {
        if (i % 100 == 0)
            tick = !tick;
        if (tick)
            ((uint16_t*)ac97->buffers)[i] = 0x7FFF;
        else
            ((uint16_t*)ac97->buffers)[i] = 0xFFFF;
    }

    outportb(ac97->iobase + AC97_VIA_R_SGD_CONTROL, BIT(7)); // Start
    //outportb(ac97->iobase + AC97_VIA_W_SGD_CONTROL, BIT(7)); // Start
}


void ac97Via_stop(ac97Via_t* ac97)
{
    outportb(ac97->iobase + AC97_VIA_R_SGD_CONTROL, BIT(6)); // Terminate
    //outportb(ac97->iobase + AC97_VIA_W_SGD_CONTROL, BIT(6)); // Terminate
}


/*
* Copyright (c) 2016 The PrettyOS Project. All rights reserved.
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
