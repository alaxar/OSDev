#ifndef IPV4_H
#define IPV4_H

#include "network/network.h"


typedef struct
{
    uint8_t  ipHeaderLength   : 4;
    uint8_t  version          : 4;
    uint8_t  typeOfService;
    uint16_t length;
    uint16_t identification;
    uint16_t fragmentation;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    IP4_t     sourceIP;
    IP4_t     destIP;
} __attribute__((packed)) ipv4Packet_t;


void ipv4_received(network_adapter_t* adapter, const ipv4Packet_t* packet, uint32_t length);
void ipv4_send(network_adapter_t* adapter, const void* data, uint32_t length, IP4_t IP, uint8_t protocol, uint8_t offloading);


#endif
