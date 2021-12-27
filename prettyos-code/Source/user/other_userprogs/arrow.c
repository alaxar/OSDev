/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "stdio.h"
#include "stdlib.h"


#define MAX 2000

uint16_t timeout=MAX;
bool point[79][42] = {{0}};
uint8_t fighterPosition = 0;

static void clearLine(uint8_t line)
{
    iSetCursor(0,line);
    for (uint8_t i=0; i<79; i++)
    {
        putchar(' ');
    }
}

static void setWeapon(uint8_t x, uint8_t y)
{
    point[x][y-1] = true;
    iSetCursor(x,y-1);
    putchar('|');
    iSetCursor(x,y);
    putchar('v');
}

static void generateWeapons(void)
{
    clearLine(1);
    clearLine(2);

    if ((rand()%10 == 0))
    {
        setWeapon(rand()%79,2);
    }
}

static void deleteWeapons(void)
{
    int i = fighterPosition + 1 + rand()%(79-fighterPosition);
    for (int8_t j=41; j>=0; j--)
    {
        if(point[i][j])
        {
            point[i][j] = false;
            iSetCursor(i, j-1);
            putchar(' ');
            iSetCursor(i, j);
            putchar(' ');
            iSetCursor(i, j+1);
            putchar(' ');
            break;
        }
    }
}

static void moveFighter(void)
{
    if (keyPressed(KEY_A) && fighterPosition > 0)
    {
        fighterPosition--;
        iSetCursor(fighterPosition,43);
        putchar(1); putchar(' ');
    }
    if (keyPressed(KEY_D) && fighterPosition<79)
    {
        iSetCursor(fighterPosition,43);
        putchar(' '); putchar(1);
        fighterPosition++;
    }
    if (keyPressed(KEY_S) && fighterPosition<78 && timeout>0)
    {
        timeout--;
        deleteWeapons();
        iSetCursor(0,0);
        printf("trials: %u/%u", MAX-timeout, MAX);
    }
    if (keyPressed(KEY_ESC))
    {
        exit(EXIT_SUCCESS);
    }
}

bool checkCrash(void)
{
    return (point[fighterPosition][41]);
}

bool checkWin(void)
{
    return (fighterPosition >= 79);
}

int main(void)
{
    srand(getCurrentSeconds()); // initialize random generator
    textColor(0x0F);

    iSetCursor(fighterPosition,43);
    putchar(1);

    for (uint8_t i = 0; i < 6; i++)
    {
        setWeapon(5 + rand()%75, 42);
    }

    while (true)
    {
        iSetCursor(25,0);
        printf("\"ARROW ATTACK\" 0.15 A=left, D=right, S=delete arrows");
        generateWeapons();
        for (uint8_t line=0; line<41; line++)
        {
            for (uint8_t column=0; column<79; column++)
            {
                if (point[column][line] == true && rand()%6 == 0)
                {
                    iSetCursor(column,line);
                    putchar(' ');
                    iSetCursor(column,line+1);
                    putchar('|');
                    iSetCursor(column,line+2);
                    putchar('v');
                    point[column][line] = false;
                    point[column][line+1] = true;
                }
            }
            for (uint16_t i=0; i<250; i++){getCurrentSeconds();}
        }
        moveFighter();
        if (checkCrash())
        {
            iSetCursor(9,44);
            printf("GAME OVER - PLAYER LOST!");
            break;
        }
        if (checkWin())
        {
            iSetCursor(14,44);
            printf("PLAYER WINS!");
            break;
        }
    }

    printf("   Press ESC to quit.");
    while (!keyPressed(KEY_ESC));
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
