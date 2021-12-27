#ifndef PCNET_H
#define PCNET_H

#include "network.h"


typedef struct
{
    uint16_t mode;
    unsigned reserved1        :  4;
    unsigned receive_length   :  4;
    unsigned reserved2        :  4;
    unsigned transfer_length  :  4;
    uint8_t  physical_address[6];
    uint16_t reserved3;
    uint64_t logical_address;
    uint32_t receive_descriptor;
    uint32_t transmit_descriptor;
} __attribute__((packed)) PCNet_initBlock;

typedef struct
{
    uint32_t address;
    uint32_t flags;
    uint32_t flags2;
    uint32_t avail;  // Can be used by OS. Stores the virtual address of the buffer.
} __attribute__((packed)) PCNet_descriptor;

typedef struct
{
    network_adapter_t* device;
    PCNet_descriptor*  receiveDesc;
    PCNet_descriptor*  transmitDesc;
    uint16_t           IO_base;
    uint8_t            currentRecDesc;
    uint8_t            currentTransDesc;
    bool               initialized;
} PCNet_card;


void PCNet_install(network_adapter_t* adapter);
bool PCNet_send(network_adapter_t* adapter, const uint8_t* data, size_t length, uint8_t offloading);
void PCNet_handler(registers_t* data, pciDev_t* device);


#endif
