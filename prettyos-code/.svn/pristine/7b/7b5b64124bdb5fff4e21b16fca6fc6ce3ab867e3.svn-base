#ifndef CDI_OSDEP_H
#define CDI_OSDEP_H

#include "pci.h"


/* Die folgenden Makros werden dazu benutzt, ein Array aller Treiber, die in der Binary vorhanden sind, zu erstellen (das Array
   besteht aus struct cdi_driver*).
   Um die Grenzen des Arrays zu kennen, werden Pointer auf die einzelnen Treiber in eine eigene Section cdi_drivers gelegt.
   Ueber Symbole am Anfang und Ende der Section kann die Bibliothek alle Treiber aufzaehlen.

   Die Variablennamen sind an sich unwichtig (der Wert wird nie ueber den Variablennamen abgefragt), aber sie muessen
   eindeutig sein. Sie werden daher static deklariert und innerhalb derselben Datei mit einer fortlaufenden Nummer unterschieden
   (__cdi_driver_0 usw.). */
#define cdi_glue(x, y) x ## y
#define cdi_declare_driver(drv, counter) \
    static const void* __attribute__((section(".cdi"), used)) \
        cdi_glue(__cdi_driver_, counter) = &drv;

/* CDI_DRIVER shall be used exactly once for each CDI driver. It registers the driver with the CDI library.
   name: Name of the driver
   drv:  A driver description (struct cdi_driver)
   deps: List of names of other drivers on which this driver depends */
#define CDI_DRIVER(name, drv, deps...) cdi_declare_driver(drv, __COUNTER__)

struct cdi_device;
struct cdi_driver;

// OS-specific PCI data.
typedef struct
{
    pciDev_t* dev;
    struct cdi_device* cdiDev;
    struct cdi_driver* driver;
} cdi_pci_device_osdep;

// OS-specific USB device data.
typedef struct
{
} cdi_usb_device_osdep;

// OS-specific DMA data.
typedef struct
{
} cdi_dma_osdep;

// OS-specific data for memory areas.
typedef struct
{
    bool malloced;
} cdi_mem_osdep;

// OS-specific data for file systems
typedef struct
{
} cdi_fs_osdep;

#endif
