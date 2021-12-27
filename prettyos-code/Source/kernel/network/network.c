/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "network.h"
#include "cdi/net.h"
#include "cdi/pci.h"
#include "util/util.h"
#include "util/list.h"
#include "util/todo_list.h"
#include "kheap.h"
#include "irq.h"
#include "video/console.h"
#include "netprotocol/ethernet.h"
#include "rtl8139.h"
#include "rtl8168.h"
#include "pcnet.h"


const network_driver_t network_drivers[ND_COUNT] =
{
    {.install = &rtl8139_install, .interruptHandler = &rtl8139_handler, .sendPacket = &rtl8139_send},
    {.install = &rtl8168_install, .interruptHandler = &rtl8168_handler, .sendPacket = &rtl8168_send},
    {.install = &PCNet_install,   .interruptHandler = &PCNet_handler,   .sendPacket = &PCNet_send}
};

Packet_t lastPacket; // save data during packet receive thru the protocols

static list_t adapters = list_init();


bool network_installDevice(pciDev_t* device)
{
  #ifdef _NETWORK_ENABLE_
    textColor(HEADLINE);

    const network_driver_t* driver = 0;
    if (device->deviceID == 0x8139 && device->vendorID == 0x10EC) // RTL 8139
    {
        driver = &network_drivers[RTL8139];
        printf("\nRealtek RTL8139");
    }
    else if ((device->deviceID == 0x8169 || device->deviceID == 0x8168) && device->vendorID == 0x10EC) // RTL 8111b/8168/8169
    {
        driver = &network_drivers[RTL8168];
        printf("\nRealtek RTL8168/RTL6169");
    }
    else if (device->deviceID == 0x2000 && device->vendorID == 0x1022) // AMD PCNet III (Am79C973)
    {
        driver = &network_drivers[PCNET];
        printf("\nAMD PCnet III");
    }

    if (driver == 0 || driver->install == 0) // PrettyOS does not know the card or the driver is not properly installed
    {
        textColor(TEXT);
        return (false);
    }

    printf(" network adapter:");
    textColor(TEXT);

    // PrettyOS has a driver for this adapter. Install it.
    network_adapter_t* adapter = network_createDevice(device);
    adapter->driver = driver;
    device->data = adapter;

    // Set IRQ handler
    irq_installPCIHandler(device->irq, driver->interruptHandler, device);

    adapter->driver->install(adapter);

    list_append(&adapters, adapter);

    // Try to get an IP by DHCP
    DHCP_Discover(adapter);

    textColor(TEXT);
    printf("\nMAC: ");
    textColor(IMPORTANT);
    printf("%M", adapter->MAC);
    textColor(TEXT);
    printf("\tIP: ");
    textColor(IMPORTANT);
    printf("%I4", adapter->IP);
    textColor(TEXT);
  #endif
    return (true);
}

network_adapter_t* network_createDevice(pciDev_t* device)
{
    network_adapter_t* adapter = malloc(sizeof(network_adapter_t), 0, "network apdapter");
    adapter->driver = 0;
    adapter->PCIdev = device;
    adapter->DHCP_State = DHCP_START;
    adapter->features = 0;

    arp_initTable(&adapter->arpTable);

    // nic
    adapter->IP.IP4[0]           =  IP_1;
    adapter->IP.IP4[1]           =  IP_2;
    adapter->IP.IP4[2]           =  IP_3;
    adapter->IP.IP4[3]           =  IP_4;

    // gateway
    adapter->Gateway_IP.IP4[0]   = GW_IP_1;
    adapter->Gateway_IP.IP4[1]   = GW_IP_2;
    adapter->Gateway_IP.IP4[2]   = GW_IP_3;
    adapter->Gateway_IP.IP4[3]   = GW_IP_4;
    adapter->Gateway_IP6.iIP6[0] = 0;
    adapter->Gateway_IP6.iIP6[1] = 0;

    // DNS server
    adapter->dnsServer_IP.IP4[0] = DNS_IP_1;
    adapter->dnsServer_IP.IP4[1] = DNS_IP_2;
    adapter->dnsServer_IP.IP4[2] = DNS_IP_3;
    adapter->dnsServer_IP.IP4[3] = DNS_IP_4;

    return (adapter);
}

void network_installCDIDevice(network_adapter_t* adapter)
{
    list_append(&adapters, adapter);

    DHCP_Discover(adapter);
}

bool network_sendPacket(network_adapter_t* adapter, const uint8_t* buffer, size_t length, uint8_t offloading)
{
    if (adapter && adapter->driver)
        return (adapter->driver->sendPacket != 0 && adapter->driver->sendPacket(adapter, buffer, length, offloading));
    else if (adapter && adapter->PCIdev->data)
    {
        struct cdi_pci_device* cdiPciDev = adapter->PCIdev->data;
        struct cdi_net_driver* cdiDriver = (struct cdi_net_driver*)cdiPciDev->meta.cdiDev->driver;
        if (cdiDriver->send_packet)
        {
            cdiDriver->send_packet((struct cdi_net_device*)cdiPciDev->meta.cdiDev, (void*)buffer, length);
            return true;
        }
    }
    return false;
}

static void network_handleReceivedBuffer(void* data, size_t length)
{
    network_adapter_t* adapter = *(network_adapter_t**)data;
    ethernet_t* eth = data + sizeof(adapter);
    ethernet_received(adapter, eth, length - sizeof(adapter));
}

void network_receivedPacket(network_adapter_t* adapter, const uint8_t* buffer, size_t length) // Called by driver
{
    char data[length+sizeof(adapter)];
    *(network_adapter_t**)data = adapter;
    memcpy(data +sizeof(adapter), buffer, length);

    todoList_add(&kernel_idleTasks, &network_handleReceivedBuffer, data, length+sizeof(adapter), 0);
}

void network_displayArpTables(void)
{
    textColor(HEADLINE);
    printf("\n\nARP Cache:");
    textColor(TEXT);
    uint8_t i = 0;
    for (dlelement_t* e = adapters.head; e != 0; e = e->next, i++)
    {
        printf("\n\nAdapter %u: %I4", i, ((network_adapter_t*)e->data)->IP);
        arp_showTable(&((network_adapter_t*)e->data)->arpTable);
    }
    putch('\n');
}

network_adapter_t* network_getAdapter(IP4_t IP)
{
    for (dlelement_t* e = adapters.head; e != 0; e = e->next)
    {
        network_adapter_t* adapter = e->data;
        if (adapter->IP.iIP4 == IP.iIP4)
        {
            return (adapter);
        }
    }
    return (0);
}

network_adapter_t* network_getFirstAdapter(void)
{
    if (adapters.head == 0)
        return (0);
    return (adapters.head->data);
}

uint32_t getMyIP(void)
{
    network_adapter_t* adapter = network_getFirstAdapter();
    if (adapter)
    {
        return adapter->IP.iIP4;
    }
    return (0);
}

void dns_setServer(IP4_t server)
{
    network_adapter_t* adapter = network_getFirstAdapter();
    if (adapter)
        adapter->dnsServer_IP.iIP4  = server.iIP4;
}

void dns_getServer(IP4_t* server)
{
    network_adapter_t* adapter = network_getFirstAdapter();
    if (adapter)
        (*server).iIP4 = adapter->dnsServer_IP.iIP4;
}


/*
* Copyright (c) 2011-2017 The PrettyOS Project. All rights reserved.
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
