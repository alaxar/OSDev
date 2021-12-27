#ifndef ETHERNET_H
#define ETHERNET_H

#include "network/network.h"


typedef struct
{
    uint8_t recv_mac[6];
    uint8_t send_mac[6];
    uint16_t type_len; // Type(Ethernet 2) or Length(Ethernet 1)
} __attribute__((packed)) ethernet_t;


void ethernet_received(network_adapter_t* adapter, const ethernet_t* eth, uint32_t length);
bool ethernet_send(network_adapter_t* adapter, const void* data, uint32_t length, const uint8_t MAC[6], uint16_t type, uint8_t offloading);


#endif
