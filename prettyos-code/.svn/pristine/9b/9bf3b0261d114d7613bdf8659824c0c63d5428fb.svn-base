#ifndef ARP_H
#define ARP_H

#include "util/list.h"
#include "network/netutils.h"

#define ARP_TABLE_TIME_TO_CHECK   2    // time in minutes
#define ARP_TABLE_TIME_TO_DELETE 10    // time in minutes


typedef struct
{
    uint16_t hardware_addresstype;
    uint16_t protocol_addresstype;
    uint8_t  hardware_addresssize;
    uint8_t  protocol_addresssize;
    uint16_t operation;
    uint8_t  source_mac[6];
    IP4_t     sourceIP;
    uint8_t  dest_mac[6];
    IP4_t     destIP;
} __attribute__((packed)) arpPacket_t;

typedef struct
{
    list_t   table;
    uint32_t lastCheck;
} arpTable_t;

typedef struct
{
    uint32_t seconds;
    uint8_t  MAC[6];
    IP4_t    IP;
    bool     dynamic;
} arpTableEntry_t;


struct network_adapter;

void arp_initTable(arpTable_t* cache);
void arp_deleteTable(arpTable_t* cache);
void arp_addTableEntry(arpTable_t* cache, const uint8_t MAC[6], IP4_t IP, bool dynamic);
arpTableEntry_t* arp_findEntry(arpTable_t* cache, IP4_t IP);
void arp_showTable(arpTable_t* cache);
void arp_received(struct network_adapter* adapter, const arpPacket_t* packet);
bool arp_sendRequest(struct network_adapter* adapter, IP4_t searchedIP); // Pass adapter->IP to it, to issue a gratuitous request
bool arp_waitForReply(struct network_adapter* adapter, IP4_t searchedIP);


#endif
