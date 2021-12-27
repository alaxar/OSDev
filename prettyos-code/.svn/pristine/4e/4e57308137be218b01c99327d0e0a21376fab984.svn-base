#ifndef IPV6_H
#define IPV6_H

#include "network/network.h"


typedef struct
{
    uint32_t version          :  4;
    uint8_t  trafficClass;
    uint32_t flowLabel        : 20;

    uint16_t payloadLength;
    uint8_t  nextHeader;
    uint8_t  ttl;

    IP6_t    sourceIP;
    IP6_t    destIP;
} __attribute__((packed)) ipv6Packet_t;


void ipv6_received(network_adapter_t* adapter, const ipv6Packet_t* packet, uint32_t length);
void ipv6_send(network_adapter_t* adapter, const void* data, uint32_t length, IP6_t IP, uint8_t protocol, uint8_t offloading);


#endif
