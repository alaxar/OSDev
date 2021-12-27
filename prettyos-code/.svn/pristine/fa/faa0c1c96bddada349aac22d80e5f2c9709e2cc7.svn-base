/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "stdio.h"
#include "string.h"
#include "stdlib.h"
#include "dns_help.h"


int main(void)
{
    setScrollField(7, 46);
    printLine("================================================================================", 0, 0x0B);
    printLine("                                PrettyIRC client",                                 2, 0x0B);
    printLine("--------------------------------------------------------------------------------", 4, 0x0B);

    iSetCursor(0, 7);

    char buffer[2048];
    printf("Enter server name (default: irc.euirc.net): ");
    gets_s(buffer, 2048);

    IP4_t IP;
    if(buffer[0] == 0)
        IP = getAddrByName("irc.euirc.net");
    else
        IP = getAddrByName(buffer);

    uint32_t connection = tcp_connect(IP, 6667); // irc protocol
    printf("\nConnected (ID = %u). Wait until connection is established... ", connection);

    event_enable(true);
    EVENT_t ev = event_poll(buffer, 2048, EVENT_NONE);

    bool ctrl = false;
    srand(getCurrentMilliseconds());

    for (;;)
    {
        switch (ev)
        {
            case EVENT_NONE:
                waitForEvent(0);
                break;
            case EVENT_TCP_CONNECTED:
                printf("ESTABLISHED.\n");
                char pStr[100];
                unsigned int number = rand();
                snprintf(pStr, 100, "NICK Pretty%u\r\nUSER Pretty%u void servername : Pretty%u\r\n", number, number, number);
                tcp_send(connection, pStr, strlen(pStr));
                break;
            case EVENT_TCP_RECEIVED:
            {
                tcpReceivedEventHeader_t* header = (void*)buffer;
                char* data = (void*)(header+1);
                data[header->length] = 0;
                printf("\nPacket received (length = %u):\n", header->length);
                textColor(0x06);
                puts(data);
                textColor(0x0F);
                for (size_t i = 0; i < header->length; i++)
                {
                    if (strncmp(data+i, "PING", 4)==0)
                    {
                        data[i+1] = 'O';
                        tcp_send(connection, data+i, header->length - i);
                    }
                }
                break;
            }
            case EVENT_KEY_DOWN:
            {
                KEY_t* key = (KEY_t*)buffer;
                switch (*key)
                {
                    case KEY_LCTRL: case KEY_RCTRL:
                        ctrl = true;
                        break;
                    case KEY_ESC:
                    {
                        const char* const msgQuit = "QUIT\r\n";
                        tcp_send(connection, msgQuit, strlen(msgQuit));
                        tcp_close(connection);
                        return (0);
                    }
                    case KEY_J:
                    {
                        if (ctrl)
                        {
                            printf("\nEnter Channel name: ");
                            getchar(); // Ommit first character, because its a 'j'
                            char str[50];
                            gets_s(str, 50);
                            char msg[70];
                            snprintf(msg, 70, "JOIN %s\r\n", str);
                            tcp_send(connection, msg, strlen(msg));
                        }
                        break;
                    }
                    case KEY_P: case KEY_L:
                    {
                        if (ctrl)
                        {
                            printf("\nEnter message: ");
                            getchar(); // Ommit first character, because its a 'p' or a 'l'
                            char str[200], msg[240];
                            gets_s(str, 200);

                            if(str[0] != '/')
                            {
                                if (*key == KEY_P)
                                    snprintf(msg, 240, "PRIVMSG #PrettyOS :%s\r\n", str);
                                else if (*key == KEY_L)
                                    snprintf(msg, 240, "PRIVMSG #lost :%s\r\n", str);
                            }
                            else
                            {
                                snprintf(msg, 240, "%s\r\n", str+1); // / means: The input is sent unchanged
                            }

                            tcp_send(connection, msg, strlen(msg));
                        }
                        break;
                    }
                    case KEY_C:
                    {
                        if(ctrl)
                        {
                            printf("\nEnter command: ");
                            getchar(); // Ommit first character, because its a 'c'

                            char cmd[202];
                            gets_s(cmd, 200);

                            strcat(cmd, "\r\n");

                            tcp_send(connection, cmd, strlen(cmd));
                        }
                        break;
                    }
                    default:
                        break;
                }
                break;
            }
            case EVENT_KEY_UP:
                switch (*(KEY_t*)buffer)
                {
                    case KEY_LCTRL: case KEY_RCTRL:
                        ctrl = false;
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        ev = event_poll(buffer, 2048, EVENT_NONE);
    }

    tcp_close(connection);
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
