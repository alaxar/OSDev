// Driver for network devices

#ifndef CDI_NET_H
#define CDI_NET_H

#include "util/types.h"
#include <cdi.h>

struct cdi_net_device
{
    struct cdi_device dev;
    uint64_t          mac : 48;
    int               number;
};

struct cdi_net_driver
{
    struct cdi_driver drv;
    void (*send_packet)(struct cdi_net_device* device, void* data, size_t size);
};

// Initialisiert die Datenstrukturen fuer einen Netzerktreiber (erzeugt die devices-Liste)
void cdi_net_driver_init(struct cdi_net_driver* driver);

// Deinitialisiert die Datenstrukturen fuer einen Netzwerktreiber (gibt die devices-Liste frei)
void cdi_net_driver_destroy(struct cdi_net_driver* driver);

// Initialisiert eine neue Netzwerkkarte
void cdi_net_device_init(struct cdi_net_device* device);

// Wird von Netzwerktreibern aufgerufen, wenn ein Netzwerkpaket empfangen wurde.
void cdi_net_receive(struct cdi_net_device* device, void* buffer, size_t size);

#endif
