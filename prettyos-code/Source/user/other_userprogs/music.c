/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"


unsigned int duration = 400;
void PlayTone(unsigned int Frequency, double DurFak) {
    if (Frequency == 0) {
        sleep(duration*DurFak);
    }
    else {
        beep(Frequency, duration*DurFak);
    }
}

void Play(const char* string)
{
    unsigned int Frequency = 0;
    for (size_t i = 0; string[i] != 0; i++)
    {
        switch (string[i]) {
            case 'c':
                Frequency = 132;
                break;
            case 'd':
                Frequency = 149;
                break;
            case 'e':
                Frequency = 165;
                break;
            case 'f':
                Frequency = 176;
                break;
            case 'g':
                Frequency = 198;
                break;
            case 'a':
                Frequency = 220;
                break;
            case 'h':
                Frequency = 248;
                break;
            case 'C':
                Frequency = 264;
                break;
            case 'D':
                Frequency = 297;
                break;
            case 'E':
                Frequency = 330;
                break;
            case 'F':
                Frequency = 352;
                break;
            case 'G':
                Frequency = 396;
                break;
            case 'A':
                Frequency = 440;
                break;
            case 'H':
                Frequency = 495;
                break;
            case '0':
                Frequency = 0;
                break;
            default:
                continue;
        }
        if (string[i+1] == '+') {
            if (string[i+2] == '+') {
                PlayTone(Frequency, 4);
            }
            else {
                PlayTone(Frequency, 2);
            }
        }
        else if (string[i+1] == '-') {
            if (string[i+2] == '-') {
                PlayTone(Frequency, 0.25);
            }
            else {
                PlayTone(Frequency, 0.5);
            }
        }
        else {
            PlayTone(Frequency, 1.0);
        }
    }
}

int main(void) {
    textColor(0x0B);
    puts("================================================================================\n");
    puts("                          Simple-Music-Creator  v0.4.4\n");
    puts("--------------------------------------------------------------------------------\n\n");
    puts("Please type in the duration of a full note or type in \"Alle meine Entchen\" or \"Hänschen klein\" to play that song and press ENTER.\n");
    char string1[100];
    gets_s(string1, 100);
    if (strstr(string1, "Alle meine Entchen") == string1) {
        duration = 500;
        Play("cdefg+g+aaaag++aaaag++ffffe+e+ddddc++");
    }
    else if (strstr(string1, "Hänschen klein") == string1) {
        duration = 500;
        Play("gee+fdd+cdefggg+gee+fdd+ceggc++dddddef+eeeeefg+gee+fdd+ceggc++");
    }
    else {
        duration = atoi(string1);
        puts("Please insert your notes (cdefgahCDEFGAH or 0 (break); speed controlled by --, -, nothing, +, ++) and press ENTER.\n");
        gets_s(string1, 100);
        Play(string1);
    }
    return (0);
}

/*
* Copyright (c) 2011-2015 The PrettyOS Project. All rights reserved.
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
