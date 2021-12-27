/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "ctype.h"


enum Feldstatus {Leer, X, O};

uint8_t tictactoe[9] = {Leer,Leer,Leer,
                        Leer,Leer,Leer,
                        Leer,Leer,Leer};
bool ende = false;

static void SetField(uint8_t x, uint8_t y, uint8_t Player)
{
    textColor(0x0F);
    iSetCursor(x*4+3,y*2+15);
    if (Player == X)
        putchar('X');
    else if (Player == O)
        putchar('O');
}

static void gewinnen(void)
{
    if ((tictactoe[0] == tictactoe[1] && tictactoe[0] == tictactoe[2] && tictactoe[0] == X) ||
        (tictactoe[3] == tictactoe[4] && tictactoe[3] == tictactoe[5] && tictactoe[3] == X) ||
        (tictactoe[6] == tictactoe[7] && tictactoe[6] == tictactoe[8] && tictactoe[6] == X) ||
        (tictactoe[0] == tictactoe[3] && tictactoe[0] == tictactoe[6] && tictactoe[0] == X) ||
        (tictactoe[1] == tictactoe[4] && tictactoe[1] == tictactoe[7] && tictactoe[1] == X) ||
        (tictactoe[2] == tictactoe[5] && tictactoe[2] == tictactoe[8] && tictactoe[2] == X) ||
        (tictactoe[0] == tictactoe[4] && tictactoe[0] == tictactoe[8] && tictactoe[0] == X) ||
        (tictactoe[2] == tictactoe[4] && tictactoe[2] == tictactoe[6] && tictactoe[2] == X))
    {
        printLine("Player X wins!", 26, 0x05);
        ende = true;
    }
    else if ((tictactoe[0] == tictactoe[1] && tictactoe[0] == tictactoe[2] && tictactoe[0] == O) ||
        (tictactoe[3] == tictactoe[4] && tictactoe[3] == tictactoe[5] && tictactoe[3] == O) ||
        (tictactoe[6] == tictactoe[7] && tictactoe[6] == tictactoe[8] && tictactoe[6] == O) ||
        (tictactoe[0] == tictactoe[3] && tictactoe[0] == tictactoe[6] && tictactoe[0] == O) ||
        (tictactoe[1] == tictactoe[4] && tictactoe[1] == tictactoe[7] && tictactoe[1] == O) ||
        (tictactoe[2] == tictactoe[5] && tictactoe[2] == tictactoe[8] && tictactoe[2] == O) ||
        (tictactoe[0] == tictactoe[4] && tictactoe[0] == tictactoe[8] && tictactoe[0] == O) ||
        (tictactoe[2] == tictactoe[4] && tictactoe[2] == tictactoe[6] && tictactoe[2] == O))
    {
        printLine("Player O wins!", 26, 0x05);
        ende = true;
    }
    else if (tictactoe[0] != Leer && tictactoe[1] != Leer && tictactoe[2] != Leer &&
        tictactoe[3] != Leer && tictactoe[4] != Leer && tictactoe[5] != Leer &&
        tictactoe[6] != Leer && tictactoe[7] != Leer && tictactoe[8] != Leer)
    {
        printLine("Remis!", 26, 0x05);
        ende = true;
    }
}

static void ClearLine(uint8_t line)
{
    printLine("                                                                                ", line, 0x0F);
}

static void Zug(uint16_t Player)
{
    textColor(0x0B);
    iSetCursor(7, 22);
    if(Player == X)
        putchar('X');
    else
        putchar('O');

    char str[80];
    uint32_t input = 9;

    for (;;)
    {
        iSetCursor(0, 24);
        gets_s(str, 80);
        if (isdigit(*str))
        {
            input = atoi(str);
        }
        else
        {
            input = 9; // String is empty -> Input not useful
        }

        ClearLine(24); // Clear Inputline
        ClearLine(26); // Clear Messageline

        if (input >= 9)
        {
            printLine("Your Input was not useful.", 26, 0x0C);
        }
        else if (tictactoe[input] != Leer)
        {
            printLine("Field already used.", 26, 0x0C);
        }
        else
        {
            break;
        }
    }

    tictactoe[input] = Player;
    SetField(input%3, input/3, Player);
    gewinnen();
}

int main(void)
{
    printLine("================================================================================", 0, 0x0B);
    printLine("                              TicTacToe 3x3  v0.7.1"                             , 2, 0x0B);
    printLine("--------------------------------------------------------------------------------", 4, 0x0B);

    iSetCursor(0,6);
    puts(" -------------\n | 0 | 1 | 2 |\n -------------\n | 3 | 4 | 5 |\n -------------\n | 6 | 7 | 8 |\n -------------\n\n");
    puts(" -------------\n |   |   |   |\n -------------\n |   |   |   |\n -------------\n |   |   |   |\n -------------");

    printLine("Player  : Please type in a number betwen 0 and 8.", 22, 0x0B);

    srand(getCurrentMilliseconds());
    int Spieler = rand()%2 + X;
    for(uint8_t i = 0; i < 9 && !ende; i++)
    {
        Zug(Spieler);
        if(Spieler == X)
            Spieler = O;
        else
            Spieler = X;
    }

    printLine("Press a key to continue...", 28, 0x0F);
    getchar();

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
