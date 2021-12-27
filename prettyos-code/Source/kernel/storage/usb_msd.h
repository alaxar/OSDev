#ifndef USB_MSD_H
#define USB_MSD_H

#include "os.h"
#include "devicemanager.h"
#include "usb/usb.h"


struct usb_CommandBlockWrapper
{
    uint32_t  CBWSignature;
    uint32_t  CBWTag;
    uint32_t  CBWDataTransferLength;
    uint8_t   CBWFlags;
    uint8_t   CBWLUN;           // only bits 3:0
    uint8_t   CBWCBLength;      // only bits 4:0
    uint8_t   commandByte[16];
} __attribute__((packed));

struct usb_CommandBlockWrapperUFI
{
    uint8_t   commandByte[12];
} __attribute__((packed));

typedef struct
{
    disk_t           disk;
    usb_interface_t* interface;

    usb_endpoint_t* endpointInMSD;
    usb_endpoint_t* endpointOutMSD;
    usb_endpoint_t* endpointInterruptMSD; // UFI specific
} usb_msd_t;


void     usb_setupMSD(usb_interface_t* interface);
void     usb_destroyMSD(usb_interface_t* interface);

FS_ERROR usb_readSectors (uint32_t sector, void* buffer, uint32_t count, disk_t* device);
FS_ERROR usb_writeSectors(uint32_t sector, const void* buffer, uint32_t count, disk_t* device);


#endif
