/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "wav.h"
#include "util/util.h"
#include "video/console.h"
#include "hid/keyboard.h"
#include "kheap.h"


bool wav_parseWAV(ac97Intel_t* ac97_currentDevice, void* wav)
{
    wavHeaderPCM_t* wavHeader = (wavHeaderPCM_t*)wav;

    if (wavHeader->riffMarker[0] == 'R' && wavHeader->riffMarker[1] == 'I' && wavHeader->riffMarker[2] == 'F' && wavHeader->riffMarker[3] == 'F')
    {
        textColor(SUCCESS);
        printf("\nRIFF: ok");
        textColor(TEXT);
    }

    printf("\tfileLength: %u", wavHeader->fileLength);

    if (wavHeader->fileTypeHeader[0] == 'W' && wavHeader->fileTypeHeader[1] == 'A' && wavHeader->fileTypeHeader[2] == 'V' && wavHeader->fileTypeHeader[3] == 'E')
    {
        textColor(SUCCESS);
        printf("\tWAVE: ok");
        textColor(TEXT);
    }

    if (wavHeader->fmtHeader.marker[0] == 'f' && wavHeader->fmtHeader.marker[1] == 'm' && wavHeader->fmtHeader.marker[2] == 't' && wavHeader->fmtHeader.marker[3] == ' ')
    {
        textColor(SUCCESS);
        printf("\tfmt: ok");
        textColor(TEXT);
    }

    printf("\nfmtLength: %u",       wavHeader->fmtHeader.length);
    printf("\taudioFormat: %u",     wavHeader->audioFormat);
    printf("\tnumChannels: %u",     wavHeader->numChannels);

    printf("\tsampleRate: %u",      wavHeader->sampleRate);
    outportw(ac97_currentDevice->nambar + PORT_NAM_FRONT_DAC_RATE, wavHeader->sampleRate); // General sample rate
    outportw(ac97_currentDevice->nambar + PORT_NAM_LR_ADC_RATE,    wavHeader->sampleRate); // Stereo  sample rate

    printf("\nbytesPerSecond: %u",  wavHeader->bytesPerSecond);
    printf("\tbytesPerSample: %u",  wavHeader->bytesPerSample);
    printf("\tbitsPerSample: %u\n", wavHeader->bitsPerSample);

    uint8_t i=0;
    while (wavHeader->byte[i + 0] != 'd' || wavHeader->byte[i + 1] != 'a' || wavHeader->byte[i + 2] != 't' || wavHeader->byte[i + 3] != 'a')
    {
        i++;
        if (i>49)
        {
            printfe("\ndata chunk not found.");
            return false;
        }
    }

    subChunkHeader_t* dataChunk = (subChunkHeader_t*)&wavHeader->byte[i];
    printf("\tdataLength: %u", dataChunk->length);

    waitForKeyStroke();

    free(ac97_currentDevice->buffer);
    ac97_currentDevice->NUM = dataChunk->length/wavHeader->bytesPerSample;
    ac97_currentDevice->buffer = malloc(dataChunk->length, 64 | HEAP_CONTINUOUS, "ac97 startWAV");

    void* dataStart = dataChunk + 1;
    memcpy(ac97_currentDevice->buffer, dataStart, dataChunk->length);

    free(ac97_currentDevice->descs);
    ac97_currentDevice->numDesc = 2;
    ac97_currentDevice->descs = malloc(sizeof(ac97Intel_bufDesc_t)*ac97_currentDevice->numDesc, 64 | HEAP_CONTINUOUS, "AC97 buffer descriptor");

    ac97Intel_start(ac97_currentDevice);
    textColor(IMPORTANT);
    printf("\nPress key to stop sound...");
    textColor(TEXT);
    getch();
    ac97Intel_stop(ac97_currentDevice);

    return true;
}

void wav_playStartWav(ac97Intel_t* ac97_currentDevice)
{
    file_t* file = fopen("1:|START.WAV", "r");

    if (file)
    {
        void* wav_start = malloc(file->size, 0, "start WAV");
        fread(wav_start, file->size, 1, file);
        fclose(file);

        wav_parseWAV(ac97_currentDevice, wav_start);
    }
    else
    {
        printfe("\nSTART.WAV could not be opened.");
    }
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