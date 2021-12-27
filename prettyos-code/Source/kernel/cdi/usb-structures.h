#ifndef CDI_USB_STRUCTURES_H
#define CDI_USB_STRUCTURES_H

#include "util/types.h"


struct cdi_usb_device_descriptor {
    uint8_t b_length;
    uint8_t b_descriptor_type;
    uint16_t bcd_usb;
    uint8_t b_device_class;
    uint8_t b_device_sub_class;
    uint8_t b_device_protocol;
    uint8_t b_max_packet_size_0;
    uint16_t id_vendor;
    uint16_t id_product;
    uint16_t bcd_device;
    uint8_t i_manufacturer;
    uint8_t i_product;
    uint8_t i_serial_number;
    uint8_t b_num_configurations;
} __attribute__((packed));

struct cdi_usb_configuration_descriptor {
    uint8_t b_length;
    uint8_t b_descriptor_type;
    uint16_t w_total_length;
    uint8_t b_num_interfaces;
    uint8_t b_configuration_value;
    uint8_t i_configuration;
    uint8_t bm_attributes;
    uint8_t b_max_power;
} __attribute__((packed));

struct cdi_usb_string_descriptor {
    uint8_t b_length;
    uint8_t b_descriptor_type;
    uint16_t b_string[127];
} __attribute__((packed));

struct cdi_usb_interface_descriptor {
    uint8_t b_length;
    uint8_t b_descriptor_type;
    uint8_t b_interface_number;
    uint8_t b_alternate_setting;
    uint8_t b_num_endpoints;
    uint8_t b_interface_class;
    uint8_t b_interface_sub_class;
    uint8_t b_interface_protocol;
    uint8_t i_interface;
} __attribute__((packed));


struct cdi_usb_endpoint_descriptor {
    uint8_t b_length;
    uint8_t b_descriptor_type;
    uint8_t b_endpoint_address;
    uint8_t bm_attributes;
    uint16_t w_max_packet_size;
    uint8_t b_interval;
} __attribute__((packed));

// cdi_usb_endpoint_descriptor.b_endpoint_address
#define CDI_USB_EPDSC_EPADR_ID(x)   ((x) & 0xf)
#define CDI_USB_EPDSC_EPADR_DIR     (1 << 7) // IN if set

// cdi_usb_endpoint_descriptor.bm_attributes
#define CDI_USB_EPDSC_ATTR_XFER_TYPE_MASK   0x03 // Transfer Type mask
#define CDI_USB_EPDSC_ATTR_CONTROL          0x00
#define CDI_USB_EPDSC_ATTR_ISOCHRONOUS      0x01
#define CDI_USB_EPDSC_ATTR_BULK             0x02
#define CDI_USB_EPDSC_ATTR_INTERRUPT        0x03
#define CDI_USB_EPDSC_ATTR_SYNC_TYPE_MASK   0x0c // Synchronization Type mask
#define CDI_USB_EPDSC_ATTR_NO_SYNC          0x00
#define CDI_USB_EPDSC_ATTR_ASYNC            0x04
#define CDI_USB_EPDSC_ATTR_ADAPTIVE         0x08
#define CDI_USB_EPDSC_ATTR_SYNC             0x0c
#define CDI_USB_EPDSC_ATTR_USAGE_TYPE_MASK  0x30 // Usage Type mask
#define CDI_USB_EPDSC_ATTR_DATA             0x00
#define CDI_USB_EPDSC_ATTR_FEEDBACK         0x10
#define CDI_USB_EPDSC_ATTR_IMPL_FB_DATA     0x20 // Implicit Feedback Data

// cdi_usb_endpoint_descriptor.w_max_packet_size
#define CDI_USB_EPDSC_MPS_MPS(x)    ((x) & 0x7ff)


struct cdi_usb_setup_packet {
    uint8_t bm_request_type;
    uint8_t b_request;
    uint16_t w_value;
    uint16_t w_index;
    uint16_t w_length;
} __attribute__((packed));

// Values constituting cdi_usb_setup_packet.bm_request_type
#define CDI_USB_CREQ_DEVICE     0
#define CDI_USB_CREQ_INTERFACE  1
#define CDI_USB_CREQ_ENDPOINT   2
#define CDI_USB_CREQ_OTHER      3
#define CDI_USB_CREQ_CLASS      (1 << 5)
#define CDI_USB_CREQ_OUT        (0 << 7)
#define CDI_USB_CREQ_IN         (1 << 7)

// Control request numbers (cdi_usb_setup_packet.b_request)
enum {
    CDI_USB_CREQ_GET_STATUS         =  0,
    CDI_USB_CREQ_CLEAR_FEATURE      =  1,
    CDI_USB_CREQ_SET_FEATURE        =  3,
    CDI_USB_CREQ_SET_ADDRESS        =  5,
    CDI_USB_CREQ_GET_DESCRIPTOR     =  6,
    CDI_USB_CREQ_SET_DESCRIPTOR     =  7,
    CDI_USB_CREQ_GET_CONFIGURATION  =  8,
    CDI_USB_CREQ_SET_CONFIGURATION  =  9,
    CDI_USB_CREQ_GET_INTERFACE      = 10,
    CDI_USB_CREQ_SET_INTERFACE      = 11,
    CDI_USB_CREQ_SYNCH_FRAME        = 12,
};

// Descriptor IDs
enum {
    CDI_USB_DESC_DEVICE         = 1,
    CDI_USB_DESC_CONFIGURATION  = 2,
    CDI_USB_DESC_STRING         = 3,
    CDI_USB_DESC_INTERFACE      = 4,
    CDI_USB_DESC_ENDPOINT       = 5,
};

// USB class codes. Defined here because they are not part of their respective specifications.

// USB class codes on the device level
#define CDI_USB_DEVCLS_NONE     0x00 // No device-wide class, use interfaces
#define CDI_USB_DEVCLS_CCDCC    0x02 // Communications and CDC Control
#define CDI_USB_DEVCLS_HUB      0x09 // Hub
#define CDI_USB_DEVCLS_BILLBD   0x11 // Billboard Device
#define CDI_USB_DEVCLS_DIAG     0xdc // Diagnostic Device
#define CDI_USB_DEVCLS_MISC     0xef // Miscellaneous
#define CDI_USB_DEVCLS_VENDOR   0xff // Vendor-specific

// USB class codes on the interface level
#define CDI_USB_IFCCLS_AUDIO    0x01 // Audio
#define CDI_USB_IFCCLS_CCDCC    CDI_USB_DEVCLS_CCDCC
#define CDI_USB_IFCCLS_HID      0x03 // Human Interface Device
#define CDI_USB_IFCCLS_PHYS     0x05 // Physical
#define CDI_USB_IFCCLS_IMAGE    0x06 // Image
#define CDI_USB_IFCCLS_PRINTER  0x07 // Printer
#define CDI_USB_IFCCLS_STORAGE  0x08 // Mass Storage
#define CDI_USB_IFCCLS_CDCDATA  0x0a // CDC Data
#define CDI_USB_IFCCLS_SMARTCD  0x0b // Smart Card
#define CDI_USB_IFCCLS_CNTTSEC  0x0d // Content Security
#define CDI_USB_IFCCLS_VIDEO    0x0e // Video
#define CDI_USB_IFCCLS_HEALTH   0x0f // Personal Healthcare
#define CDI_USB_IFCCLS_AV       0x10 // Audio/Video
#define CDI_USB_IFCCLS_DIAG     CDI_USB_DEVCLS_DIAG
#define CDI_USB_IFCCLS_WIRELESS 0xe0 // Wireless Controller
#define CDI_USB_IFCCLS_MISC     CDI_USB_DEVCLS_MISC
#define CDI_USB_IFCCLS_APP      0xfe // Application-specific
#define CDI_USB_IFCCLS_VENDOR   CDI_USB_DEVCLS_VENDOR

#endif
