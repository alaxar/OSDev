/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "arp.h"
#include "ethernet.h"
#include "video/console.h"
#include "timer.h"
#include "util/util.h"
#include "tasking/scheduler.h"
#include "network/netutils.h"


void arp_deleteTableEntry(arpTable_t* cache, arpTableEntry_t* entry)
{
    list_delete(&cache->table, list_find(&cache->table, entry));
}

static void arp_checkTable(arpTable_t* cache)
{
    if (timer_getSeconds() > (cache->lastCheck + ARP_TABLE_TIME_TO_CHECK * 60)) // Check only every ... minutes
    {
        cache->lastCheck = timer_getSeconds();
        for (dlelement_t* e = cache->table.head; e != 0;)
        {
            arpTableEntry_t* entry = e->data;
            if (entry->dynamic &&                                                    // Only dynamic entries should be killed.
                timer_getSeconds() > entry->seconds + ARP_TABLE_TIME_TO_DELETE * 60) // Entry is older than ... minutes -> Obsolete entry. Delete it.
            {
                e = list_delete(&cache->table, e);
            }
            else
                e = e->next;
        }
    }
}

void arp_addTableEntry(arpTable_t* cache, const uint8_t MAC[6], IP4_t IP, bool dynamic)
{
    arpTableEntry_t* entry = arp_findEntry(cache, IP); // Check if there is already an entry with the same IP.
    if (entry == 0) // No entry found. Create new one.
    {
        dlelement_t* elem = list_alloc_elem(sizeof(arpTableEntry_t), "arp entry");
        entry = elem->data;
        list_append_elem(&cache->table, elem);
    }
    entry->IP.iIP4 = IP.iIP4;
    memcpy(entry->MAC, MAC, 6);
    entry->dynamic = dynamic;
    entry->seconds = timer_getSeconds();
}

arpTableEntry_t* arp_findEntry(arpTable_t* cache, IP4_t IP)
{
    arp_checkTable(cache); // We check the arp cache for obsolete entries.

    for (dlelement_t* e = cache->table.head; e != 0; e = e->next)
    {
        arpTableEntry_t* entry = e->data;
        if (entry->IP.iIP4 == IP.iIP4)
        {
            entry->seconds = timer_getSeconds(); // Update time stamp.
            return (entry);
        }
    }
    return (0);
}

void arp_showTable(arpTable_t* cache)
{
    arp_checkTable(cache); // We check the table for obsolete entries.

    textColor(TABLE_HEADING);
    printf("\nIP\t\t  MAC\t\t\tType\t  Time(sec)");
    printf("\n--------------------------------------------------------------------------------");
    textColor(TEXT);
    for (dlelement_t* e = cache->table.head; e != 0; e = e->next)
    {
        arpTableEntry_t* entry = e->data;
        size_t length = printf("%I4\t", entry->IP);
        if (length < 9) putch('\t');
        printf("  %M\t%s\t  %u\n", entry->MAC, entry->dynamic?"dynamic":"static", entry->seconds);
    }
    textColor(TABLE_HEADING);
    printf("--------------------------------------------------------------------------------");
}

void arp_initTable(arpTable_t* cache)
{
    list_construct(&cache->table);
    cache->lastCheck = timer_getSeconds();

    // Create default entries
    uint8_t broadcast[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    IP4_t broadcastIP = {.iIP4 = 0xFFFFFFFF};
    arp_addTableEntry(cache, broadcast, broadcastIP, false);
}

void arp_deleteTable(arpTable_t* cache)
{
    list_destruct(&cache->table);
}

void arp_received(network_adapter_t* adapter, const arpPacket_t* packet)
{
    // 1 = Ethernet, 0x0800 = IPv4
    if (ntohs(packet->hardware_addresstype) == 1 && ntohs(packet->protocol_addresstype) == 0x0800 &&
        packet->hardware_addresssize == 6 && packet->protocol_addresssize == 4)
    {
        // extract the operation
        switch (ntohs(packet->operation))
        {
            case 1: // ARP-Request
              #ifdef _ARP_DEBUG_
                textColor(HEADLINE);
                if (packet->sourceIP.iIP4 == packet->destIP.iIP4) // IP requ. and searched is identical
                {
                    printf("\nARP Gratuitous Request:");
                }
                else
                {
                    printf("\nARP Request:");
                }
                textColor(LIGHT_GRAY); printf("\nMAC Requesting: "); textColor(DATA); printf("%M", packet->source_mac);
                textColor(LIGHT_GRAY); printf("  IP Requesting: ");  textColor(DATA); printf("%I4", packet->sourceIP);
                textColor(LIGHT_GRAY); printf("\nMAC Searched:   "); textColor(DATA); printf("%M", packet->dest_mac);
                textColor(LIGHT_GRAY); printf("  IP Searched:   ");  textColor(DATA); printf("%I4", packet->destIP);
              #endif

                // requested IP is our own IP?
                if (packet->destIP.iIP4 == adapter->IP.iIP4)
                {
                    arpPacket_t reply;

                    reply.operation = htons(2); // reply

                    reply.hardware_addresstype = packet->hardware_addresstype;
                    reply.protocol_addresstype = packet->protocol_addresstype;

                    reply.hardware_addresssize = packet->hardware_addresssize;
                    reply.protocol_addresssize = packet->protocol_addresssize;

                    for (uint8_t i = 0; i < 6; i++)
                    {
                        reply.dest_mac[i]   = packet->source_mac[i];
                        reply.source_mac[i] = adapter->MAC[i];
                    }

                    reply.destIP.iIP4   = packet->sourceIP.iIP4;
                    reply.sourceIP.iIP4 = adapter->IP.iIP4;

                    ethernet_send(adapter, (void*)&reply, sizeof(arpPacket_t), packet->source_mac, 0x0806, 0);
                }
                break;

            case 2: // ARP-Response
              #ifdef _ARP_DEBUG_
                textColor(HEADLINE);
                printf("\nARP Response:");
                textColor(LIGHT_GRAY); printf("\nMAC Replying:   "); textColor(DATA); printf("%M", packet->source_mac);
                textColor(LIGHT_GRAY); printf("  IP Replying:   ");  textColor(DATA); printf("%I4", packet->sourceIP);
                textColor(LIGHT_GRAY); printf("\nMAC Requesting: "); textColor(DATA); printf("%M", packet->dest_mac);
                textColor(LIGHT_GRAY); printf("  IP Requesting: ");  textColor(DATA); printf("%I4", packet->destIP);
              #endif
                break;
        } // switch
        arp_addTableEntry(&adapter->arpTable, packet->source_mac, packet->sourceIP, true); // ARP table entry
        scheduler_unblockEvent(BL_NETPACKET, (void*)BL_NET_ARP);
    } // if
    else
    {
        printfe("\nNo Ethernet and IPv4 - Unknown packet sent to ARP.");
    }
}

bool arp_sendRequest(network_adapter_t* adapter, IP4_t searchedIP)
{
    arpPacket_t request;

    request.operation = htons(1); // Request

    request.hardware_addresstype = htons(1); // Ethernet
    request.protocol_addresstype = htons(0x0800); // IP

    request.hardware_addresssize = 6;
    request.protocol_addresssize = 4;

    for (uint8_t i = 0; i < 6; i++)
    {
        request.dest_mac[i]   = 0x00;
        request.source_mac[i] = adapter->MAC[i];
    }

    request.destIP.iIP4   = searchedIP.iIP4;
    request.sourceIP.iIP4 = adapter->IP.iIP4;

  #ifdef _ARP_DEBUG_
    textColor(HEADLINE);
    printf("\nARP Request sent:");
    textColor(LIGHT_GRAY);
    printf(" IP Searched: ");
    textColor(IMPORTANT);
    printf("%I4", searchedIP);
  #endif

    static const uint8_t destMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    return ethernet_send(adapter, (void*)&request, sizeof(arpPacket_t), destMAC, 0x0806, 0);
}

bool arp_waitForReply(struct network_adapter* adapter, IP4_t searchedIP)
{
    uint32_t timewait = 2000; // ms
    do {
        if(arp_findEntry(&adapter->arpTable, searchedIP) != 0)
            return true;
    } while (scheduler_blockCurrentTask(BL_NETPACKET, (void*)BL_NET_ARP, timewait));
    return (arp_findEntry(&adapter->arpTable, searchedIP) != 0);
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
