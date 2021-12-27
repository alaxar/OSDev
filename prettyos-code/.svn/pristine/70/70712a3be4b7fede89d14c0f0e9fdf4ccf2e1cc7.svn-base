/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/


#include "ac97_intel.h"
#include "wav.h"
#include "util/util.h"
#include "video/console.h"
#include "paging.h"
#include "timer.h"
#include "irq.h"
#include "kheap.h"


static void AC97_handler(registers_t* r, pciDev_t* device);

void ac97Intel_install(pciDev_t* device)
{
    ac97Intel_t* ac97 = malloc(sizeof(ac97Intel_t), 0, "ac97Intel");
    ac97->device = device;

    ac97->device->data = ac97;

    // ICH6 specification requires Bit 0 to be set before PCI_CMD_IO can be set
    uint8_t CFG = pci_configRead(device, 0x41, 1);
    if (!(CFG & BIT(0)))
        pci_configWrite_byte(device, 0x41, CFG|BIT(0));

    uint16_t pciCommandRegister = pci_configRead(device, PCI_COMMAND, 2);
    pci_configWrite_word(device, PCI_COMMAND, pciCommandRegister | PCI_CMD_BUSMASTER | PCI_CMD_IO);

    irq_installPCIHandler(device->irq, AC97_handler, device);

    // first and second address room
    ac97->nambar = (uint16_t)device->bar[0].baseAddress;  // NAM-BAR (Mixer)
    ac97->nabmbar = (uint16_t)device->bar[1].baseAddress; // NABM-BAR (Player)
    printf("\nnambar: %X nabmbar: %X  ", ac97->nambar, ac97->nabmbar);

    // reset
    outportw(ac97->nambar + PORT_NAM_RESET, 42);         // Each value is possible
    outportb(ac97->nabmbar + PORT_NABM_PICONTROL, 0x02); // 0x02 enforces reset
    outportb(ac97->nabmbar + PORT_NABM_POCONTROL, 0x02); // 0x02 enforces reset
    outportb(ac97->nabmbar + PORT_NABM_MCCONTROL, 0x02); // 0x02 enforces reset
    sleepMilliSeconds(100);

    // volume
    uint8_t volume = 0; //Am lautesten!
    outportw(ac97->nambar + PORT_NAM_MASTER_VOLUME, (volume << 8) | volume);  // General volume left and right
    outportw(ac97->nambar + PORT_NAM_MONO_VOLUME, volume);                    // Volume for Mono
    outportw(ac97->nambar + PORT_NAM_PC_BEEP_VOLUME, volume);                 // Volume for PC speaker
    outportw(ac97->nambar + PORT_NAM_PCM_OUT_VOLUME, (volume << 8) | volume); // Volume for PCM left and right
    sleepMilliSeconds(10);

    // sample rate
    if (!(inportw(ac97->nambar + PORT_NAM_EXT_AUDIO_ID) & 1))
    {
        // sample rate is fixed to 48 kHz
    }
    else
    {
        outportw(ac97->nambar + PORT_NAM_EXT_AUDIO_STS_CTRL, inportw(ac97->nambar + PORT_NAM_EXT_AUDIO_STS_CTRL) | 1); // Activate variable rate audio
        sleepMilliSeconds(10);
        outportw(ac97->nambar + PORT_NAM_FRONT_DAC_RATE, 44100); // General sample rate
        outportw(ac97->nambar + PORT_NAM_LR_ADC_RATE, 44100); // Stereo  sample rate
        sleepMilliSeconds(10);
    }

    // Actual sample rate can be read here:
    printf("sample rate: %u Hz\n", inportw(ac97->nambar + PORT_NAM_FRONT_DAC_RATE));

    bool tick = false;

    // Generate beep (test)
    ac97->NUM = 6553;
    ac97->buffer = malloc(sizeof(*ac97->buffer) * ac97->NUM, 64 | HEAP_CONTINUOUS, "AC97 sample buffer");
    for (size_t i = 0; i < ac97->NUM; i++)
    {
        if (i % 100 == 0)
            tick = !tick;
        if (tick)
            ac97->buffer[i] = 0x7FFF;
        else
            ac97->buffer[i] = rand();
    }

    ac97->numDesc = 3;
    ac97->descs = malloc(sizeof(ac97Intel_bufDesc_t) * ac97->numDesc, 64 | HEAP_CONTINUOUS, "AC97 buffer descriptor");
}

void ac97Intel_start(ac97Intel_t* ac97)
{
    for (uint32_t i = 0; i < ac97->numDesc; i++)
    {
        ac97->descs[i].buf = paging_getPhysAddr(ac97->buffer);
        ac97->descs[i].len = ac97->NUM;
        ac97->descs[i].ioc = 1;
        ac97->descs[i].bup = 0;
    }
    ac97->descs[ac97->numDesc - 1].bup = 1;

    outportl(ac97->nabmbar + PORT_NABM_POBDBAR, paging_getPhysAddr(ac97->descs));
    outportb(ac97->nabmbar + PORT_NABM_POLVI, ac97->numDesc - 1);
    outportb(ac97->nabmbar + PORT_NABM_POCONTROL, 0x15); // play and generate interrupt afterwards
}

void ac97Intel_stop(ac97Intel_t* ac97)
{
    for (int i=0; i<ac97->numDesc; i++)
    {
        ac97->descs[i].buf = 0;
        ac97->descs[i].len = 0;
        ac97->descs[i].ioc = 0;
        ac97->descs[i].bup = 0;
    }

    outportl(ac97->nabmbar + PORT_NABM_POBDBAR, paging_getPhysAddr(ac97->descs));
    outportb(ac97->nabmbar + PORT_NABM_POLVI, ac97->numDesc - 1);
    outportb(ac97->nabmbar + PORT_NABM_POCONTROL, 0);
}

static void AC97_handler(registers_t* r, pciDev_t* device)
{
    ac97Intel_t* ac97 = (ac97Intel_t*)device->data;

    uint8_t pi = inportb(ac97->nabmbar + PORT_NABM_PISTATUS) & 0x1C;
    uint8_t po = inportb(ac97->nabmbar + PORT_NABM_POSTATUS) & 0x1C;
    uint8_t mc = inportb(ac97->nabmbar + PORT_NABM_MCSTATUS) & 0x1C;

    outportb(ac97->nabmbar + PORT_NABM_PISTATUS, pi);
    outportb(ac97->nabmbar + PORT_NABM_POSTATUS, po);
    outportb(ac97->nabmbar + PORT_NABM_MCSTATUS, mc);
}


/*
* Copyright (c) 2012-2016 The PrettyOS Project. All rights reserved.
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
