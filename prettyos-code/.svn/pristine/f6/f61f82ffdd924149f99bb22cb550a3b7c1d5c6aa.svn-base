#ifndef DEVICEMANAGER_H
#define DEVICEMANAGER_H

#include "os.h"
#include "filesystem/fsmanager.h"

#define PARTITIONARRAYSIZE 4


struct port;

typedef struct
{
    void (*motorOff)(struct port*);
    void (*pollDisk)(struct port*);
} portType_t;

typedef struct diskType
{
    // Parameters: LBA, buffer, sector count, disk
    // Implementation note: The devmgr will take care about disk_t::optAccSecCount
    //                      and always request optAccSecCount aligned request with
    //                      a maximum of optAccSecCount sectors at once, if
    //                      disk_t::alignedAccess is set.
    FS_ERROR (*readSectors) (uint32_t, void*, uint32_t, struct disk*);
    FS_ERROR (*writeSectors)(uint32_t, const void*, uint32_t, struct disk*);
} diskType_t;

extern const portType_t FDD, USB_UHCI, USB_OHCI, USB_EHCI, USB_XHCI, RAM, HDD;
extern const diskType_t FLOPPYDISK, USB_MSD, RAMDISK, HDDDISK;

typedef struct disk
{
    const diskType_t* type;
    partition_t* partition[PARTITIONARRAYSIZE]; // 0 if partition is not used
    uint64_t     size;                          // size of disk in bytes
    void*        data;                          // Contains additional information depending on disk-type
    uint32_t     accessRemaining;               // Used to control motor
    struct port* port;

    // Technical data of the disk
    uint32_t sectorSize;     // Bytes per sector
    uint16_t secPerTrack;    // Number of sectors per track (if the disk is separated into tracks)
    uint16_t headCount;      // Number of heads (if the disk is separated into heads)
    uint16_t optAccSecCount; // Number of sectors that should be accessed at once
    bool     alignedAccess;  // Accesses will be aligned by optAccSecCount
    uint8_t  BIOS_driveNum;  // Number of this disk given by BIOS
} disk_t;

typedef struct port
{
    const portType_t* type;
    disk_t*     insertedDisk; // 0 if no disk is inserted
    void*       data;         // Contains additional information depending on its type
    char        name[15];
} port_t;

// 16-byte partition record // http://en.wikipedia.org/wiki/Master_boot_record
typedef struct
{
    uint8_t  bootflag;     // Status: 0x80 = bootable (active), 0x00 = non-bootable, other = invalid
    uint8_t  startCHS[3];  // CHS address of first absolute sector in partition
    uint8_t  type;         // http://en.wikipedia.org/wiki/Partition_type
    uint8_t  endCHS[3];    // CHS address of last absolute sector in partition
    uint32_t startLBA;     // LBA of first absolute sector in the partition
    uint32_t sizeLBA;      // Number of sectors in partition
} __attribute__((packed)) partitionEntry_t;


extern partition_t* systemPartition;


void deviceManager_install(void);
void deviceManager_checkDrives(void);
void deviceManager_attachPort(port_t* port);
void deviceManager_destructPort(port_t* port);
void deviceManager_attachDisk(disk_t* disk);
void deviceManager_destructDisk(disk_t* disk);
void deviceManager_showPortList(void);
void deviceManager_showDiskList(void);

partition_t* deviceManager_getPartition(const char* path);

FS_ERROR deviceManager_analyzeDisk(disk_t* disk);

FS_ERROR sectorRead       (uint32_t sector, uint8_t* buffer, disk_t* disk);
FS_ERROR singleSectorRead (uint32_t sector, uint8_t* buffer, disk_t* disk);
FS_ERROR sectorWrite      (uint32_t sector, const uint8_t* buffer, disk_t* disk);
FS_ERROR singleSectorWrite(uint32_t sector, const uint8_t* buffer, disk_t* disk);

void deviceManager_fillCache(uint32_t sector, disk_t* disk, const uint8_t* buffer, uint32_t count, bool write);
void deviceManager_flushCaches(disk_t* disk);
void deviceManager_clearCaches(disk_t* disk);


#endif
