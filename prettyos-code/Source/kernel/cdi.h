/* The Core module contains the fundamental CDI data structures and functions
   on which the other modules are based.

   Data structures:
   CDI is based on three basic structures. The other modules extend these
   structures in a compatible way: For example, cdi_net_driver is a structure
   that contains a cdi_driver as its first element so that it can be cast to
   cdi_device. The structures are:

   - cdi_device describes a device. Amongst others, it contains fields that
     reference the corresponding driver and an address that identifies the
     device uniquely on its bus.
   - cdi_driver describes a driver. It has a type which determines to which
     CDI module the driver belongs (and thus which type the struct may be cast
     to). Moreover, each driver has some function points which can be used by
     the CDI library to call driver functions (e.g. for sending a network
     packet)
   - cdi_bus_data contains bus specific information on a device, like the
     address of the device on the bus (e.g. bus.dev.func for PCI) or other
     bus related information (e.g. device/vendor ID). This structure is used
     for initialisation of devices, i.e. when the device itself doesn't exist
     yet.


   Initialisation:
   Drivers are initialised in the following steps:

   - Initialisation of drivers: The init() callback of each driver is called.
     After the driver has returned from there, it is registered with CDI and
     the operating system. The driver needs not to be able to handle requests
     before it has completed its init() call, but it must be prepared for them
     immediately afterwards.
   - Search for and initialisation of devices: For each device init_device is
     called in the available drivers until a driver accepts the device (or all
     of the drivers reject it).

   todo: Call scan_bus for each device and initialise possible child devices */

#ifndef CDI_H
#define CDI_H

#include "pci.h"
#include <cdi-osdep.h>
#include <cdi/lists.h>

typedef enum {
    CDI_UNKNOWN         = 0,
    CDI_NETWORK         = 1,
    CDI_STORAGE         = 2,
    CDI_SCSI            = 3,
    CDI_VIDEO           = 4,
    CDI_AUDIO           = 5,
    CDI_AUDIO_MIXER     = 6,
    CDI_USB_HCD         = 7,
    CDI_USB             = 8,
    CDI_FILESYSTEM      = 9,
    CDI_PCI             = 10,
    CDI_AHCI            = 11,
} cdi_device_type_t;

struct cdi_driver;

// Describes a device in relation to its bus. This information usually contains a bus address and some additional data like device/vendor ID.
struct cdi_bus_data {
    cdi_device_type_t   bus_type;
};

/* This structure, which is extended by any bus type supporting it, contains
   information about which devices can be handled by a certain driver.
   Typically, it will contain some of the fields provided by the appropriate
   cdi_bus_data structure(the ones which are used to identify a device like
   device/vendor ID) where negative values signify a "don't care". */
struct cdi_bus_device_pattern {
    cdi_device_type_t bus_type;
};

// Describes a device
struct cdi_device {
    // Name of the device (must be unique among the devices of the same driver)
    const char*             name;

    // Driver used for the device
    struct cdi_driver*      driver;

    // Bus specific data for the device
    struct cdi_bus_data*    bus_data;

    // PrettyOS specific
    void*                   backdev;
};

// Describes a CDI driver
struct cdi_driver {
    cdi_device_type_t   type;
    cdi_device_type_t   bus;
    const char*         name;
    bool                initialised;

    // Contains all devices (cdi_device) which use this driver
    cdi_list_t          devices;

    /* Tries to initialise a device. The function may only be called by the CDI library if bus_data->type matches the type of the driver.
       return: A new cdi_device created by malloc() if the driver accepts the device. NULL if the device is not supported by this driver. */
    struct cdi_device* (*init_device)(struct cdi_bus_data* bus_data);
    void (*remove_device)(struct cdi_device* device);

    int (*init)(void);
    int (*destroy)(void);
};

/* Drivers which implement their own main() function must call this function before they call any other CDI function.
   It initialises internal data structures of the CDI implementation and starts all drivers.
   This function should only be called once, additional calls will have no effect. Depending on the implementation,
   this function may or may not return. */
void cdi_init(void);

// Initialises the data structures for a driver
void cdi_driver_init(struct cdi_driver* driver);

// Deinitialises the data structures for a driver
void cdi_driver_destroy(struct cdi_driver* driver);

/* Registers a new driver with CDI
   driver: driver to be registred */
void cdi_driver_register(struct cdi_driver* driver);

/* Allows a driver to inform the operating system that a new device has become available.

   The rationale behind cdi_provide_device is to allow controller interfaces
   to provide devices on their bus which may not be available through the
   conventional PCI bus (for example, a USB mass storage device). This also
   allows devices to inform the OS of the presence of other devices outside
   of the context of a controller interface, where necessary.

   The operating system should determine which driver to load (or inform of
   the new device) based on the cdi_bus_data struct passed. How the operating
   system decides which driver to load/inform is not defined by CDI. An example
   would be a simple list generated from a script which parses each driver in
   the source tree, finds relevant cdi_bus_data structs, and adds them to a
   list which maps the structs to a string. The OS can then use that string to
   load a driver somehow.

   Whilst CDI could provide a pre-defined list of mappings for operating
   systems to use when implementing cdi_provide_device, it was decided that
   this would make the interface too rigid. Whilst this method requires a
   little more effort from the CDI implementor, it also allows operating
   systems to load non-CDI drivers (eg, from a native driver interface) as
   defined by the OS-specific mapping list. This would be impossible if CDI
   rigidly enforced a specific method.

   Operating systems may also choose to implement cdi_provide_device and then
   use it when iterating over the PCI bus in order to load drivers dynamically
   (effectively treating coldplug devices as hotplugged before boot).

   return: 0 on success or -1 if an error was encountered. */
int cdi_provide_device(struct cdi_bus_data* device);

/* Informs the operating system that a new device has become available and
   is to be handled by an internal driver (i.e. a driver linked into the same
   binary).

   This function can be used to give drivers for devices with multiple
   components a clearer structure (e.g. model SATA disks as CDI devices
   separate from the AHCI controller).

   Be careful though: It should only be used if a tight coupling between device
   and subdevice is unvoidable, as it requires linking the code of both drivers
   into the same binary. This might not be a problem for monolithic kernels,
   but CDI is used in OSes of different designs.

   If the driver isn't initialised yet, it is initialised before creating the new device.

   device: Addressing information for the driver to find the device
   driver: Driver that must be used for this device
   return: 0 on success or -1 if an error was encountered. */
int cdi_provide_device_internal_drv(struct cdi_bus_data* device, struct cdi_driver* driver);

/* Tells the CDI implementation that the driver drv is able to handle devices
   with the pattern pattern (appearing on the bus pattern->bus_type). This
   function can be called multiple times. Any device appearing on the bus not
   fitting the pattern will not be passed to the drv->init_device() function.
   That function may however return NULL even after the driver has called
   cdi_handle_bus_device(), that is, it is valid to call this function with a
   pattern that does not only fit devices handled by drv but others as well
   (which can then be rejected by letting drv->init_device() return NULL).

   This function must only be called from drv->init_driver(). */
void cdi_handle_bus_device(struct cdi_driver* drv, struct cdi_bus_device_pattern* pattern);

#endif
