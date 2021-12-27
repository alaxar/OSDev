/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/


#include "userlib.h"
#include "stdio.h"
#include "string.h"


int main(void)
{
    setScrollField(7, 46);
    printLine("================================================================================", 0, 0x0B);
    printLine("                     Pretty UDPreceive - Network test program!",                   2, 0x0B);
    printLine("--------------------------------------------------------------------------------", 4, 0x0B);

    event_enable(true);
    const uint32_t BUFFERSIZE = 4000;
    char buffer[BUFFERSIZE];
    EVENT_t ev = event_poll(buffer, BUFFERSIZE, EVENT_NONE);

    udp_bind(8085);

    iSetCursor(0, 7);

    for (;;)
    {
        switch (ev)
        {
            case EVENT_NONE:
            {
                waitForEvent(0);
                break;
            }
            case EVENT_UDP_RECEIVED:
            {
                static uint32_t count=0;
                count++;
                udpReceivedEventHeader_t* header = (void*)buffer;
                char* data = (void*)(header+1);
                data[header->length] = 0;
                printf("\npacket received from %u.%u.%u.%u:%u to port %u. Length = %u", header->srcIP.IP4[0], header->srcIP.IP4[1], header->srcIP.IP4[2], header->srcIP.IP4[3], header->srcPort, header->destPort, header->length);
                printf("\n%u:\t",count);
                textColor(0x0A);
                puts(data);
                textColor(0x0F);
                break;
            }
            case EVENT_KEY_DOWN:
            {
                KEY_t* key = (void*)buffer;
                if (*key == KEY_ESC)
                {
                    udp_unbind(8085);
                    return (0);
                }
                break;
            }
            default:
                break;
        }
        ev = event_poll(buffer, BUFFERSIZE, EVENT_NONE);
    }

    udp_unbind(8085);
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
