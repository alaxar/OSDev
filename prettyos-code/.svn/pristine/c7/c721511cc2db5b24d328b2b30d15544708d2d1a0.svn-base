#ifndef NETWORK_H
#define NETWORK_H

#include "../pci.h"
#include "netprotocol/arp.h"
#include "netprotocol/dhcp.h"
#include "netutils.h"
#include "irq.h"
#include "util/util.h"

/*
typical qemu.bat:
del ser1.txt
del ser2.txt
del ser3.txt
del ser4.txt
set QEMU_AUDIO_DRV=wav
qemu.exe  -boot a -fda FloppyImage.img -soundhw pcspk -net nic,model=rtl8139 -redir tcp:5023::23
-redir tcp:8080::80 -localtime -net user -net dump,file=netdump.pcap -usb
-serial file:ser1.txt -serial file:ser2.txt -serial file:ser3.txt -serial file:ser4.txt
*/

// own IP at start
#define IP_1    192
#define IP_2    168
#define IP_3      1
#define IP_4     22

// requested IP
#define RIP_1   192
#define RIP_2   168
#define RIP_3     1
#define RIP_4    22

// gateway IP for routing to the internet
#define GW_IP_1   192
#define GW_IP_2   168
#define GW_IP_3     1
#define GW_IP_4     1

// DNS IP for resolving name to IP
#define DNS_IP_1   208
#define DNS_IP_2    67
#define DNS_IP_3   222
#define DNS_IP_4   222


typedef struct network_adapter network_adapter_t;


enum network_drivers
{
    RTL8139, RTL8168, PCNET, ND_COUNT
};

typedef struct
{
    void (*install)(network_adapter_t*); // Device
    void (*interruptHandler)(registers_t*, pciDev_t*); // Device
    bool (*sendPacket)(network_adapter_t*, const uint8_t*, size_t, uint8_t); // Device, buffer, length, offloading
} network_driver_t;

enum network_features
{
    OFFLOAD_IPv4 = BIT(0),
    OFFLOAD_UDP = BIT(2),
    OFFLOAD_TCP = BIT(3)
};

struct network_adapter
{
    const network_driver_t* driver;
    pciDev_t*         PCIdev;
    void*             data; // Drivers internal data
    uint8_t           MAC[6];
    uint8_t           features;

    // IPv4
    IP4_t             IP;
    arpTable_t        arpTable;
    DHCP_state        DHCP_State;
    IP4_t             Gateway_IP;
    IP4_t             Subnet;
    IP4_t             dnsServer_IP;

    // IPv6
    IP6_t             IP6;
    IP6_t             Gateway_IP6;
};

typedef struct
{
    uint8_t MAC[6];
} Packet_t;


extern const network_driver_t network_drivers[ND_COUNT];


network_adapter_t* network_createDevice(pciDev_t* device);
bool network_installDevice(pciDev_t* device);
void network_installCDIDevice(network_adapter_t* adapter);
bool network_sendPacket(network_adapter_t* adapter, const uint8_t* buffer, size_t length, uint8_t offloading);
void network_receivedPacket(network_adapter_t* adapter, const uint8_t* buffer, size_t length); // Called by driver
void network_displayArpTables(void);
network_adapter_t* network_getAdapter(IP4_t IP);
network_adapter_t* network_getFirstAdapter(void);
uint32_t getMyIP(void);
void dns_setServer(IP4_t server);
void dns_getServer(IP4_t* server);


#endif
