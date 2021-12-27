#ifndef SB16_H
#define SB16_H

#include "os.h"
#include "pci.h"

/* Informationen zur Soundblaster16 Treiber Programmierung
    http://www.inversereality.org/tutorials/sound%20programming/examples/soundblaster16example2.html
    http://homepages.cae.wisc.edu/~brodskye/other/code.html#sb16snd


CT1335    Sound Blaster 2.0 CD Interface Card
CT1345    Sound Blaster Pro
CT1745    Sound Blaster 16

The Sound Blaster 16 DSP I/O Ports

The SB16's DSP chip is programming using several I/O ports at a base I/O address
determined by jumper settings. On the SB16, there are 16 I/O ports which are
used for FM synthesized music, mixer settings, DSP programming and CD-ROM access.
Five of these ports are used in programming the DSP. They are listed below.

    * 2x6h - DSP Reset
    * 2xAh - DSP Read
    * 2xCh - DSP Write (Command/Data), DSP write-buffer status (Bit 7)
    * 2xEh - DSP Read-buffer status (Bit 7), DSP interrupt acknowledge
    * 2xFh - DSP 16-bit interrupt acknowledge

Resetting the DSP

You have to reset the DSP before you can program it. The DSP can be reset using
the following procedure:

   1. Write a 1 to the reset port (2x6)
   2. Wait for 3 microseconds
   3. Write a 0 to the reset port (2x6)
   4. Poll the read-buffer status port (2xE) until bit 7 is set
   5. Poll the read data port (2xA) until you receive an AA

The DSP usually takes about 100 microseconds to initialized itself.
After this period of time, if the return value is not AA or there is no data at
all, then the SB card may not be installed or an incorrect I/O address is being used.


Writing to the DSP

To write a byte to the SB16, the following procedure should be used:

   1. Read the write-buffer status port (2xC) until bit 7 is cleared
   2. Write the value to the write port (2xC)

Reading from the DSP
To read a byte from the SB16, the following procedure should be used:

   1. Read the read-buffer status port (2xE) until bit 7 is set
   2. Read the value from the read port (2xA)
*/

// SoundBlaster Registers
/*
Bit Number
Index    7    6    5    4    3    2    1    0
0x00    Reset Mixer
0x04    Voice Volume (L)    Voice Volume (R)
0x0A    .    Mic Volume
0x22    Master Volume (L)    Master Volume (R)
0x26    MIDI Volume (L)    MIDI Volume (R)
0x28    CD Volume (L)    CD Volume (R)
0x2E    Line Volume (L)    Line Volume (R)
0x30    Master Volume (L)    .
0x31    Master Volume (R)    .
0x32    Voice Volume (L)    .
0x33    Voice Volume (R)    .
0x34    MIDI Volume (L)    .
0x35    MIDI Volume (R)    .
0x36    CD Volume (L)    .
0x37    CD Volume (R)    .
0x38    Line Volume (L)    .
0x39    Line Volume (R)    .
0x3A    Mic Volume    .
0x3B    PC Speaker Volume    .
0x3C    Output Mixer Switches
.    .    Line.L    Line.R    CD.L    CD.R    Mic
0x3D    Input Mixer (L) Switches
.    .    MIDI.L    MIDI.R    Line.L    Line.R    CD.L    CD.R    Mic
0x3E    Input Mixer (R) Switches
.    .    MIDI.L    MIDI.R    Line.L    Line.R    CD.L    CD.R    Mic
0x3F    Input Gain (L)    .
0x40    Input Gain (R)    .
0x41    Output Gain (L)    .
0x42    Output Gain (R)    .
0x43    .    AGC
0x44    Treble (L)    .
0x45    Treble (R)    .
0x46    Bass (L)    .
0x47    Bass (R)
*/

typedef struct
{
    uint8_t masterleft, masterright;
    uint8_t vocleft, vocright;
    uint8_t midileft, midiright;
    uint8_t cdleft ,cdright;
    uint8_t lineleft, lineright;
    uint8_t micvolume;
    uint8_t pcspeaker;
    uint8_t outputswitches;
    uint8_t inputswitchesleft;
    uint8_t inputswitchesright;
    uint8_t inputgainleft, inputgainright;
    uint8_t outputgainleft, outputgainright;
    uint8_t agc;
    uint8_t trebleleft, trebleright;
    uint8_t bassleft, bassright;
} __attribute__((packed)) sb16_mixer_t;


void install_SB16(pciDev_t* device);


#endif
