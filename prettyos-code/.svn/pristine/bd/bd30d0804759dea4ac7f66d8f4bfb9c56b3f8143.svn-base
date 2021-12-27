/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "ipv4.h"
#include "tcp.h"
#include "udp.h"
#include "icmp.h"
#include "arp.h"
#include "ethernet.h"
#include "video/console.h"
#include "kheap.h"
#include "util/util.h"


void ipv4_received(network_adapter_t* adapter, const ipv4Packet_t* packet, uint32_t length)
{
  #ifdef _NETWORK_DATA_
    textColor(HEADLINE);
    printf("\nIPv4:");
    textColor(IMPORTANT);
    printf(" %I4\t<== %I4", packet->destIP, packet->sourceIP);
    textColor(TEXT);
  #endif
    if (packet->destIP.iIP4 != adapter->IP.iIP4 && packet->destIP.iIP4 != 0xFFFFFFFF) // Filter all packets which are neither sent to us nor to broadcast
    {
      #ifdef _NETWORK_DATA_
        printf("\nWe are not the addressee.");
      #endif
        return;
    }

    // IPv4 protocol is parsed here and distributed in switch/case
    uint32_t ipHeaderLengthBytes = 4 * packet->ipHeaderLength; // is given as number of 32 bit pieces (4 byte)
    switch (packet->protocol)
    {
        case 1: // icmp
            icmp_receive(adapter, (const void*)packet+ipHeaderLengthBytes, ntohs(packet->length)-ipHeaderLengthBytes, packet->sourceIP);
            break;
        case 6: // tcp
            tcp_receive(adapter, (const void*)packet+ipHeaderLengthBytes, ntohs(packet->length)-ipHeaderLengthBytes, packet->sourceIP);
            break;
        case 17: // udp
            udp_receive(adapter, (const void*)packet+ipHeaderLengthBytes, packet->sourceIP);
            break;
        default:
            textColor(IMPORTANT);
            printf("\nUnexpected protocol after IP packet: %u", packet->protocol);
            textColor(TEXT);
            break;
    }
}

static bool ipv4_sendPacket(network_adapter_t* adapter, const ipv4Packet_t* packet, uint32_t length, IP4_t IP, uint8_t features)
{
    // Find IP
    arpTableEntry_t* entry = arp_findEntry(&adapter->arpTable, IP);
    if(entry == 0)
    {
        arp_sendRequest(adapter, IP);
        arp_waitForReply(adapter, IP);
        entry = arp_findEntry(&adapter->arpTable, IP);
        if(entry == 0)
            return (false); // IP not found
    }

    ethernet_send(adapter, packet, length+sizeof(ipv4Packet_t), entry->MAC, 0x0800, features); // Send packet

    return (true);
}

void ipv4_send(network_adapter_t* adapter, const void* data, uint32_t length, IP4_t IP, uint8_t protocol, uint8_t offloading)
{
    ipv4Packet_t* packet = malloc(sizeof(ipv4Packet_t)+length, 0, "ipv4 packet");

    memcpy(packet+1, data, length);

    packet->destIP.iIP4     = IP.iIP4;
    packet->sourceIP.iIP4   = adapter->IP.iIP4;
    packet->version        = 4;
    packet->ipHeaderLength = sizeof(ipv4Packet_t) / 4;
    packet->typeOfService  = 0;
    packet->length         = htons(sizeof(ipv4Packet_t) + length);
    packet->identification = 0;
    packet->fragmentation  = htons(0x4000); // do not fragment
    packet->ttl            = 128;
    packet->protocol       = protocol;
    packet->checksum       = 0;
    if (!(adapter->features & OFFLOAD_IPv4))
        packet->checksum   = htons(internetChecksum(packet, sizeof(ipv4Packet_t), 0));


    // TODO: Tell routing table to route the ip address

    if (IP.iIP4 == 0 || IP.iIP4 == 0xFFFFFFFF || sameSubnet(IP, adapter->IP, adapter->Subnet)) // IP is in LAN
    {
      #ifdef _NETWORK_DATA_
        printf("\nIP is in LAN. ");
      #endif
        if(!ipv4_sendPacket(adapter, packet, length, IP, offloading | (adapter->features & OFFLOAD_IPv4))) // Try to send packet
        {
          #ifdef _NETWORK_DATA_
            printf("Destination not found. We try to deliver the packet to the gateway %I4...", adapter->Gateway_IP);
          #endif
            if(!ipv4_sendPacket(adapter, packet, length, adapter->Gateway_IP, offloading | (adapter->features & OFFLOAD_IPv4))) // Try to send packet to gateway
            {
              #ifdef _NETWORK_DATA_
                printfe(" failed!");
              #endif
            }
        }
    }
    else // IP is not in LAN. Send packet to server
    {
        if(!ipv4_sendPacket(adapter, packet, length, adapter->Gateway_IP, offloading | (adapter->features & OFFLOAD_IPv4))) // Try to send packet to gateway
        {
            printfe("\nThe server was not found");
        }
    }
    free(packet);
}


/*
* Copyright (c) 2010-2016 The PrettyOS Project. All rights reserved.
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
