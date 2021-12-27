/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "ipv6.h"
#include "tcp.h"
#include "udp.h"
#include "ethernet.h"
#include "video/console.h"
#include "kheap.h"
#include "util/util.h"


void ipv6_received(network_adapter_t* adapter, const ipv6Packet_t* packet, uint32_t length)
{
  #ifdef _NETWORK_DATA_
    textColor(HEADLINE);
    printf("\nIPv6:");
    textColor(IMPORTANT);
    printf(" %I6\t<== %I6", packet->destIP, packet->sourceIP);
    textColor(TEXT);
  #endif
    if (packet->destIP.iIP6 != adapter->IP6.iIP6 && (packet->destIP.iIP6[0] & 0xFF01) != 0xFF01) // Filter all packets which are neither sent to us nor to broadcast
    {
      #ifdef _NETWORK_DATA_
        printf("\nWe are not the addressee.");
      #endif
        return;
    }

    uint8_t nextHeader = packet->nextHeader;
    const void* data = packet + 1;
    while (nextHeader != 59)
    {
        // IPv6 protocol is parsed here and distributed in switch/case
        switch (nextHeader)
        {
        case 0: // Hop-by-Hop (Extension Header)
        case 43: // Routing (Extension Header)
        case 60: // Destination Options (Extension Header)
            nextHeader = ((const uint8_t*)data)[0];
            data += ((const uint8_t*)data)[1];
            break;
        case 44: // Fragment (Extension Header)
            nextHeader = ((const uint8_t*)data)[0];
            data += 8;
            break;
        case 51: // Authentication Header (Extension Header)
            nextHeader = ((const uint8_t*)data)[0];
            data += ((const uint8_t*)data)[1] * 4 + 2;
            break;
        case 50: // Encapsulating Security Payload (Extension Header)
            return; // TODO: Unclear how to parse this
        case 135: // Mobility (Extension Header)
            return;
        case 58: // ICMPv6
            printf("\nICMPv6");
            return;
        case 6: // TCP
            printf("\nTCP via IPv6");
            return;
        case 17: // UDP
            printf("\nUDP via IPv6");
            return;
        default:
            textColor(IMPORTANT);
            printf("\nUnexpected protocol after IPv6 packet: %u", packet->nextHeader);
            textColor(TEXT);
            return;
        }
    }
}

static bool ipv6_sendPacket(network_adapter_t* adapter, const ipv6Packet_t* packet, uint32_t length, IP6_t IP, uint8_t features)
{
    return false;
}

void ipv6_send(network_adapter_t* adapter, const void* data, uint32_t length, IP6_t IP, uint8_t protocol, uint8_t offloading)
{
    ipv6Packet_t* packet = malloc(sizeof(ipv6Packet_t)+length, 0, "ipv6 packet");

    memcpy(packet+1, data, length);

    memcpy(packet->destIP.IP6, IP.IP6, 16);
    memcpy(packet->sourceIP.IP6, adapter->IP6.IP6, 16);
    packet->version        = 6;
    packet->trafficClass   = 0; // TODO
    packet->flowLabel      = 0; // TODO
    packet->payloadLength  = htons(length);
    packet->ttl            = 128;

    if(IP.IP6[0] == 0xFE80) // Link-Local
        ipv6_sendPacket(adapter, packet, length, IP, offloading); // TODO: Send to gateway?
    else
        ipv6_sendPacket(adapter, packet, length, adapter->Gateway_IP6, offloading);

    free(packet);
}


/*
* Copyright (c) 2016 The PrettyOS Project. All rights reserved.
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
