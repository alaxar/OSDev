/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cdi/pci.h"
#include "kheap.h"
#include "../pci.h"


void cdi_pci_get_all_devices(cdi_list_t list)
{
    for (dlelement_t* e = pci_devices.head; e; e = e->next)
    {
        pciDev_t* dev = e->data;
        struct cdi_pci_device* cdiDev = malloc(sizeof(struct cdi_pci_device), 0, "cdi_pci_device");
        cdiDev->meta.dev = dev;
        cdiDev->bus = dev->bus;
        cdiDev->bus_data.bus_type = CDI_PCI;
        cdiDev->class_id = dev->classID;
        cdiDev->dev = dev->device;
        cdiDev->device_id = dev->deviceID;
        cdiDev->function = dev->func;
        cdiDev->interface_id = dev->interfaceID;
        cdiDev->irq = dev->irq;
        cdiDev->resources = cdi_list_create();
        for (uint8_t i = 0; i < 6; i++)
        {
            if (dev->bar[i].memoryType != PCI_INVALIDBAR)
            {
                struct cdi_pci_resource* res = malloc(sizeof(struct cdi_pci_resource), 0, "cdi_pci_ressource");
                res->start = dev->bar[i].baseAddress;
                res->index = i;
                res->length = dev->bar[i].memorySize;
                if (dev->bar[i].memoryType == PCI_IO)
                    res->type = CDI_PCI_IOPORTS;
                else
                    res->type = CDI_PCI_MEMORY;

                cdi_list_push(cdiDev->resources, res);
            }
        }
        cdiDev->rev_id = dev->revID;
        cdiDev->subclass_id = dev->subclassID;
        cdiDev->vendor_id = dev->vendorID;
        cdi_list_push(list, cdiDev);
    }
}

void cdi_pci_device_destroy(struct cdi_pci_device* device)
{
    while(!cdi_list_empty(device->resources))
    {
        free(cdi_list_pop(device->resources));
    }
    cdi_list_destroy(device->resources);
    free(device);
}

void cdi_pci_alloc_ioports(struct cdi_pci_device* device)
{
}

void cdi_pci_free_ioports(struct cdi_pci_device* device);

void cdi_pci_alloc_memory(struct cdi_pci_device* device);

void cdi_pci_free_memory(struct cdi_pci_device* device);

uint8_t cdi_pci_config_readb(struct cdi_pci_device* device, uint8_t offset)
{
    return pci_configRead(device->meta.dev, offset, 1);
}

uint16_t cdi_pci_config_readw(struct cdi_pci_device* device, uint8_t offset)
{
    return pci_configRead(device->meta.dev, offset, 2);
}

uint32_t cdi_pci_config_readl(struct cdi_pci_device* device, uint8_t offset)
{
    return pci_configRead(device->meta.dev, offset, 4);
}

void cdi_pci_config_writeb(struct cdi_pci_device* device, uint8_t offset, uint8_t value)
{
    pci_configWrite_byte(device->meta.dev, offset, value);
}

void cdi_pci_config_writew(struct cdi_pci_device* device, uint8_t offset, uint16_t value)
{
    pci_configWrite_word(device->meta.dev, offset, value);
}

void cdi_pci_config_writel(struct cdi_pci_device* device, uint8_t offset, uint32_t value)
{
    pci_configWrite_dword(device->meta.dev, offset, value);
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
