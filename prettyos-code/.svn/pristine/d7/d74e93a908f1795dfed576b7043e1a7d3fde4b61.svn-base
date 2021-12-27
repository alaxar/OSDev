/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/


#include "userlib.h"
#include "stdio.h"
#include "string.h"
#include "dns_help.h"


int main(void)
{
    setScrollField(7, 46);
    printLine("================================================================================", 0, 0x0B);
    printLine("                                  PrettyBrowser",                                 2, 0x0B);
    printLine("--------------------------------------------------------------------------------", 4, 0x0B);
    printLine("                    F5 - reload; F6 - new file; F7 - new host",                    5, 0x0F);
    printLine("--------------------------------------------------------------------------------", 6, 0x0B);

    event_enable(true);
    char buffer[4096];
    EVENT_t ev = event_poll(buffer, 4095, EVENT_NONE);

    iSetCursor(0, 7);

    char* data;

    textColor(0x0F);
    printf("Enter the address (no subdirs yet!):\n");
    char hostname[100];
    gets_s(hostname, 100);
    printf("Enter filename (Don't forget / ):\n");
    char filename[100];
    gets_s(filename, 100);

    IP4_t IP = getAddrByName(hostname);

    printf("%u.%u.%u.%u", IP.IP4[0], IP.IP4[1], IP.IP4[2], IP.IP4[3]);
    uint32_t connection = tcp_connect(IP, 80);
    printf("\nConnected (ID = %u). Wait until connection is established... ", connection);

    for(;;)
    {
        switch(ev)
        {
            case EVENT_NONE:
                waitForEvent(0);
                break;
            case EVENT_TCP_CONNECTED:
            {
                printf("ESTABLISHED.\n");
                char pStr[200];
                snprintf(pStr, 200, "GET %s HTTP/1.1\r\nHost: %s\r\nConnection: close\r\n\r\n", filename, hostname);
                textColor(0x0A);
                puts(pStr);
                textColor(0x0F);
                tcp_send(connection, pStr, strlen(pStr));
                break;
            }
            case EVENT_TCP_RECEIVED:
            {
                textColor(0x06);
                tcpReceivedEventHeader_t* header = (void*)buffer;
                data = (void*)(header+1);
                data[header->length] = 0;
                printf("%s", data);
                textColor(0x0F);
                break;
            }
            case EVENT_KEY_DOWN:
            {
                KEY_t* key = (void*)buffer;
                switch(*key)
                {
                    case KEY_ESC:
                        tcp_close(connection);
                        return (0);
                    case KEY_F5:
                        printf("Reload...\n");
                        tcp_close(connection);
                        connection = tcp_connect(IP, 80);
                        printf("\nConnected (ID = %u). Wait until connection is established... ", connection);
                        break;
                    case KEY_F6:
                        printf("\nEnter filename (Don't forget / ):\n");
                        gets_s(filename, 100);
                        tcp_close(connection);
                        connection = tcp_connect(IP, 80);
                        printf("\nConnected (ID = %u). Wait until connection is established... ", connection);
                        break;
                    case KEY_F7:
                        printf("\nEnter hostname:\n");
                        gets_s(hostname, 100);
                        printf("\nEnter filename (Don't forget / ):\n");
                        gets_s(filename, 100);
                        IP = getAddrByName(hostname);
                        tcp_close(connection);
                        connection = tcp_connect(IP, 80);
                        printf("\nConnected (ID = %u). Wait until connection is established... ", connection);
                        break;
                    default:
                        break;
                }
                break;
            }
            default:
                break;
        }
        ev = event_poll(buffer, 4095, EVENT_NONE);
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