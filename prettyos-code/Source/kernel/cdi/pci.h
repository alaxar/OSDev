#ifndef CDI_PCI_H
#define CDI_PCI_H

#include "util/types.h"

#include <cdi.h>
#include <cdi-osdep.h>
#include <cdi/lists.h>

#define PCI_CLASS_STORAGE         0x01
#define PCI_SUBCLASS_ST_SATA      0x06

#define PCI_CLASS_MULTIMEDIA      0x04
#define PCI_SUBCLASS_MM_HDAUDIO   0x03

// Describes a PCI device
struct cdi_pci_device {
    struct cdi_bus_data bus_data;

    uint16_t    bus;
    uint16_t    dev;
    uint16_t    function;

    uint16_t    vendor_id;
    uint16_t    device_id;

    uint8_t     class_id;
    uint8_t     subclass_id;
    uint8_t     interface_id;

    uint8_t     rev_id;

    uint8_t     irq;

    // List of I/O resources which belong to the device (content of the BARs, struct cdi_pci_ressource*)
    cdi_list_t resources;

    cdi_pci_device_osdep meta;
};

// Type of the resource described by a BAR
typedef enum {
    CDI_PCI_MEMORY,
    CDI_PCI_IOPORTS
} cdi_res_t;

// Describes an I/O resource of a device (represents a BAR)
struct cdi_pci_resource {
    // Type of the resource (memory or ports)
    cdi_res_t    type;
    // Base address of the resource (physical memory address or port number)
    uintptr_t    start;
    // Size of the ressource in bytes
    size_t       length;
    // Index of the BAR that belongs to the resource, starting with 0
    unsigned int index;
    // Virtual address of the mapped MMIO memory (is set by cdi_pci_alloc_memory)
    void*        address;
};


// Queries all PCI devices in the machine. The devices (struct cdi_pci_device*) are inserted into a given list.
void cdi_pci_get_all_devices(cdi_list_t list);

// Frees the information for a PCI device
void cdi_pci_device_destroy(struct cdi_pci_device* device);

// Allocates the IO ports of the PCI device for the driver
void cdi_pci_alloc_ioports(struct cdi_pci_device* device);

// Frees the IO ports of the PCI device
void cdi_pci_free_ioports(struct cdi_pci_device* device);

// Maps the MMIO memory of the PCI device
void cdi_pci_alloc_memory(struct cdi_pci_device* device);

// Frees the MMIO memory of the PCI device
void cdi_pci_free_memory(struct cdi_pci_device* device);

// Indicates direct access to the PCI configuration space to CDI drivers
#define CDI_PCI_DIRECT_ACCESS

/* Reads a byte (8 bit) from the PCI configuration space of a PCI device
   device: The device
   offset: The offset in the configuration space
   return: The corresponding value */
uint8_t cdi_pci_config_readb(struct cdi_pci_device* device, uint8_t offset);

/* Reads a word (16 bit) from the PCI configuration space of a PCI device
   device: The device
   offset: The offset in the configuration space
   return: The corresponding value */
uint16_t cdi_pci_config_readw(struct cdi_pci_device* device, uint8_t offset);

/* Reads a dword (32 bit) from the PCI configuration space of a PCI device
   device: The device
   offset: The offset in the configuration space
   return: The corresponding value */
uint32_t cdi_pci_config_readl(struct cdi_pci_device* device, uint8_t offset);

/* Writes a byte (8 bit) into the PCI configuration space of a PCI device
   device: The device
   offset: The offset in the configuration space
   value:  Value to be set */
void cdi_pci_config_writeb(struct cdi_pci_device* device, uint8_t offset, uint8_t value);

/* Writes a word (16 bit) into the PCI configuration space of a PCI device
   device: The device
   offset: The offset in the configuration space
   value:  Value to be set */
void cdi_pci_config_writew(struct cdi_pci_device* device, uint8_t offset, uint16_t value);

/* Writes a dword (32 bit) into the PCI configuration space of a PCI device
   device: The device
   offset: The offset in the configuration space
   value:  Value to be set */
void cdi_pci_config_writel(struct cdi_pci_device* device, uint8_t offset, uint32_t value);

#endif
