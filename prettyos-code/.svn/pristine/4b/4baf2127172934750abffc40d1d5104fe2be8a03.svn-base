/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cdi/net.h"
#include "cdi/pci.h"
#include "network/network.h"


static uint32_t netcard_id = 0;


void cdi_net_driver_init(struct cdi_net_driver* driver)
{
    driver->drv.type = CDI_NETWORK;
    cdi_driver_init((struct cdi_driver*)driver);
}

void cdi_net_driver_destroy(struct cdi_net_driver* driver)
{
    cdi_driver_destroy((struct cdi_driver*) driver);
}

void cdi_net_device_init(struct cdi_net_device* device)
{
    device->number = netcard_id++;

    struct cdi_pci_device* pcidev = (struct cdi_pci_device*)device->dev.bus_data;
    pcidev->meta.cdiDev = (struct cdi_device*)device;
    pcidev->meta.dev->data = device->dev.bus_data;
    device->dev.driver = pcidev->meta.driver;

    network_adapter_t* adapter = device->dev.backdev = network_createDevice(pcidev->meta.dev);
    for (uint8_t i = 0; i < 6; i++) // Copy MAC bytewise due to design failure in CDI
        adapter->MAC[i] = device->mac>>(i*8);
    network_installCDIDevice(device->dev.backdev);
}

void cdi_net_receive(struct cdi_net_device* device, void* buffer, size_t size)
{
    network_receivedPacket(device->dev.backdev, buffer, size);
}

/*
* Copyright (c) 2009-2016 The PrettyOS Project. All rights reserved.
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
