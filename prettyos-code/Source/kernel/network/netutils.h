#ifndef NETUTILS_H
#define NETUTILS_H

#include "util/types.h"

// hton = Host To Network
static inline uint16_t htons(uint16_t v)
{
    __asm__("xchg %h0, %b0" : "+Q"(v));
    return (v);
}
static inline uint32_t htonl(uint32_t v)
{
    __asm__("bswap %0" : "+r"(v));
    return (v);
}
// ntoh = Network To Host
#define ntohs(v) htons(v)
#define ntohl(v) htonl(v)


enum
{
    BL_NET_ARP
};

typedef union
{
    uint8_t IP4[4];
    uint32_t iIP4;
} __attribute__((packed)) IP4_t;

typedef union
{
    uint16_t IP6[8];
    uint64_t iIP6[2]; // prefix and interface ID
} __attribute__((packed)) IP6_t;

uint16_t internetChecksum(const void* addr, size_t count, uint32_t pseudoHeaderChecksum);
uint16_t udptcpCalculateChecksum(const void* p, uint16_t length, IP4_t srcIP, IP4_t destIP, uint16_t protocol);

bool sameSubnet(IP4_t IP1, IP4_t IP2, IP4_t subnet);


#endif
