/* The Storage module contains drivers for mass storage devices.

   Storage implements two of the data structures defined in \ref core and extends them by properties specific to mass storage devices:
   - cdi_storage_device describes a mass storage device
   - cdi_storage_driver contains function pointers to the functions that a mass storage driver provides.

   Drivers may call the functions defined in storage.h in order to communicate with the CDI implementation. */

#ifndef CDI_STORAGE_H
#define CDI_STORAGE_H

#include "util/types.h"
#include <cdi.h>

// Represents a mass storage device.
struct cdi_storage_device {
    struct cdi_device   dev;
    // Block size of this device
    size_t              block_size;
    // Size of the mass storage device in blocks
    uint64_t            block_count;
};

// Describes a driver for mass storage devices.
struct cdi_storage_driver {
    struct cdi_driver   drv;

    /* Reads blocks from the device
       start:  Number of the first block to be read (first block on the device is 0)
       count:  Number of blocks to read
       buffer: Buffer in which the read data should be stored
       return: 0 on success, -1 in error cases */
    int (*read_blocks)(struct cdi_storage_device* device, uint64_t start, uint64_t count, void* buffer);

    /* Writes blocks to the device
       start:  Number of the first block to be written (first block on the device is 0)
       count:  Number of blocks to write
       buffer: Buffer which contains the data to be written
       return: 0 on success, -1 in error cases */
    int (*write_blocks)(struct cdi_storage_device* device, uint64_t start, uint64_t count, void* buffer);
};

// Initialises the data structures for a mass storage device. This function must be called during initialisation of a mass storage driver.
void cdi_storage_driver_init(struct cdi_storage_driver* driver);

// Deinitialises the data structures for a mass storage driver
void cdi_storage_driver_destroy(struct cdi_storage_driver* driver);

/* Registers a mass storage device with the operating system. This function must be called once a mass storage driver has completed all
   required initialisations.
   The operation system may react to this e.g. with scanning the device for partitions or creating device nodes for the user. */
void cdi_storage_device_init(struct cdi_storage_device* device);

#endif
