/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "audio.h"
#include "ac97_intel.h"
#include "ac97_via.h"
#include "sb16.h"
#include "video/console.h"
#include "wav.h"


#ifdef _AUDIO_ENABLE_
enum {AD_AC97_INTEL, AD_AC97_VIA, AD_SB16, AD_END};

static const audio_driver_t drivers[AD_END] =
{
    { &ac97Intel_install },
    { &ac97Via_install },
    { &install_SB16 }
};

static pciDev_t* firstDev = 0;
#endif


void audio_installDevice(pciDev_t* device)
{
  #ifdef _AUDIO_ENABLE_
    if (!firstDev)
        firstDev = device;

    if(device->vendorID == 0x1102)                                     // SB16. (TODO: We check only vendor here. Verify that its really an SB16)
    {
        drivers[AD_SB16].install(device);
    }
    else if (device->vendorID == 0x8086 /* && device->deviceID ==  */) // Intel (TODO: We check only vendor here. Verify that its really an AC97)
    {
        drivers[AD_AC97_INTEL].install(device);
    }
    else if (device->vendorID == 0x1106 && device->deviceID == 0x3058) // Via
    {
        drivers[AD_AC97_VIA].install(device);
    }
  #endif
}

void audio_test(void)
{
  #ifdef _AUDIO_ENABLE_
    if (firstDev)
    {
        // HACK. Support other devices than Intel-AC97 here.
        ac97Intel_start(firstDev->data);
        textColor(IMPORTANT);
        printf("Press key to stop ac97 test sound!");
        textColor(TEXT);
        waitForKeyStroke();
        ac97Intel_stop(firstDev->data);

        wav_playStartWav(firstDev->data); // start sound
    }
    else
    {
        printfe("No audio device found.");
    }
    waitForKeyStroke();
  #endif
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
