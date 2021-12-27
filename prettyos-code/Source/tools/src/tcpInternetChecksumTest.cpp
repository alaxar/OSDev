#include "stdint.h"
#include "stdio.h"

#define htons(v) ((((v) >> 8) & 0xFF) | (((v) & 0xFF) << 8))  // big endian

typedef struct
{
    uint8_t src[4];
    uint8_t dest[4];
    uint8_t res;
    uint8_t prot;
    uint16_t length;
} __attribute__((packed)) pseudoheader_t;


uint16_t internetChecksum(void* addr, size_t count, uint32_t pseudoHeaderChecksum)
{
    uint32_t sum  = pseudoHeaderChecksum;
    uint8_t* data = (uint8_t*)addr;

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



uint16_t udptcpCalculateChecksum(void* p, uint16_t length, uint8_t srcIP[4], uint8_t destIP[4], uint16_t protocol)
{
    pseudoheader_t pseudo;
    for (uint8_t i=0; i<4; i++)
    {
        pseudo.src[i] = srcIP[i];
        pseudo.dest[i] = destIP[i];
    }
    pseudo.length = htons(length);
    pseudo.prot = protocol;
    pseudo.res = 0;

    uint32_t pseudoHeaderChecksum = 0;
    uint8_t  count = 12; // pseudo header contains 12 byte

    uint8_t* data = (uint8_t*)&pseudo;



    while (count > 1)
    {
        // pseudo header contains 6 WORD
        pseudoHeaderChecksum += (data[0] << 8) | data[1]; // Big Endian
        data   += 2;
        count  -= 2;
    }

    return internetChecksum(p, length, pseudoHeaderChecksum); // util.c
}

/*
TCP-Paket: 0402001700000000000000005002ffff88b00000
Checksum: 0x88b0 [incorrect, should be 0x2801 ...]
*/

uint8_t src[4]  = {192,168,1,97};
uint8_t dest[4] = {192,168,1,23};

int main()
{
    // The two checksum byte have to be set to zero (0, 0,) before calculating the checksum:
    uint8_t packet[] = {0x04, 0x02, 0x00, 0x17, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x50, 0x02, 0xff, 0xff, 0, 0, 0x00, 0x00};
    uint16_t checksum = udptcpCalculateChecksum(packet, 20, src, dest, 6);
    printf("%X\n", checksum);
}

// Another packet with same source and dest IP:
// Should: 0xf96f //uint8_t packet[] = {00, 0x17, 0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0x1c, 0xb3, 0x0f, 0xc9, 0x50, 0x12, 0xff, 0xff, 0, 0, 0x00, 0x00};
