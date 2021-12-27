/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "netutils.h"


typedef struct
{
    IP4_t src;
    IP4_t dest;
    uint8_t res;
    uint8_t prot;
    uint16_t length;
} __attribute__((packed)) updtcpPseudoHeader_t;


// compute internet checksum for "count" bytes beginning at location "addr"
uint16_t internetChecksum(const void* addr, size_t count, uint32_t pseudoHeaderChecksum)
{
    uint32_t sum  = pseudoHeaderChecksum;
    const uint8_t* data = addr;

    while (count > 1) // inner loop
    {
        sum   += (data[0] << 8) | data[1]; // Big Endian
        data  += 2;
        count -= 2;
    }

    if (count > 0) // add left-over byte, if any
    {
        sum += data[0] << 8;
    }

    while (sum >> 16) // fold 32-bit sum to 16 bits
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum & 0xFFFF;
}

uint16_t udptcpCalculateChecksum(const void* p, uint16_t length, IP4_t srcIP, IP4_t destIP, uint16_t protocol)
{
    updtcpPseudoHeader_t pseudo;
    pseudo.src.iIP4 = srcIP.iIP4;
    pseudo.dest.iIP4 = destIP.iIP4;
    pseudo.length = htons(length);
    pseudo.prot = protocol;
    pseudo.res = 0;

    uint32_t pseudoHeaderChecksum = 0;
    const uint8_t* data = (uint8_t*)&pseudo;

    for (uint8_t i = 0; i < sizeof(updtcpPseudoHeader_t); i+=2)
    {
        // pseudo header contains 6 WORD
        pseudoHeaderChecksum += (data[i] << 8u) | data[i+1]; // Big Endian
    }

    return internetChecksum(p, length, pseudoHeaderChecksum);
}

bool sameSubnet(IP4_t IP1, IP4_t IP2, IP4_t subnet)
{
    return ((IP1.iIP4 & subnet.iIP4) == (IP2.iIP4 & subnet.iIP4));
}


/*
* Copyright (c) 2011-2013 The PrettyOS Project. All rights reserved.
*
* http://www.c-plusplus.de/forum/viewforum-var-f-is-62.html
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
