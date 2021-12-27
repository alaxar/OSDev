/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "devicemanager.h"
#include "video/console.h"
#include "util/util.h"
#include "util/array.h"
#include "kheap.h"
#include "usb_msd.h"
#include "flpydsk.h"
#include "filesystem/fat.h"
#include "usb/uhci.h"
#include "usb/xhci.h"
#include "hdd.h"
#include "ipc.h"


array(disk_t*) disks = array_init();
array(port_t*) ports = array_init();
static mutex_t devmgr_dpMutex = mutex_init();
partition_t* systemPartition = 0;

const portType_t FDD = {.motorOff = &flpydsk_motorOff, .pollDisk = 0},
            USB_UHCI = {.motorOff = 0,                 .pollDisk = &uhci_pollDisk},
            USB_OHCI = {.motorOff = 0,                 .pollDisk = 0},
            USB_EHCI = {.motorOff = 0,                 .pollDisk = 0},
            USB_XHCI = {.motorOff = 0,                 .pollDisk = &xhci_pollDisk},
            RAM      = {.motorOff = 0,                 .pollDisk = 0},
            HDD      = {.motorOff = 0,                 .pollDisk = 0};

const diskType_t FLOPPYDISK = {.readSectors = &flpydsk_readSectors, .writeSectors = &flpydsk_writeSectors},
                 USB_MSD    = {.readSectors = &usb_readSectors,     .writeSectors = &usb_writeSectors},
                 RAMDISK    = {.readSectors = 0,                    .writeSectors = 0};

#ifdef _ENABLE_HDD_
const diskType_t HDDDISK    = {.readSectors = &hdd_readSectors,     .writeSectors = &hdd_writeSectors};
#endif

// Cache
typedef struct
{
    uint8_t* buffer;
    size_t cacheSize;
    disk_t* disk;
    uint32_t sector;
    bool valid;
    bool write; // Contains unwritten data
} cache_t;

static size_t numCaches = 0;
static cache_t* caches = 0;
static mutex_t cacheMutex = mutex_init();


void deviceManager_install(void)
{
    int64_t ramsize;
    ipc_getInt("PrettyOS/RAM", &ramsize);
    uint32_t mb = ramsize / 1024 / 1024;
    if (mb < 50)       // Less than 50 MiB -> cache of 5...600 sectors
        numCaches = max(155,15*mb)-150;
    else if (mb < 150) // Less than 150 MiB -> cache of 600...1600 sectors
        numCaches = 100 + 10*mb;
    else if (mb < 500) // Less than 500 MiB -> cache of 1600...3350 sectors
        numCaches = 850 + 5*mb;
    else               // More than 500 MiB -> cache of more than 3350 sectors
        numCaches = 2850 + mb;

    caches = malloc(sizeof(cache_t)*numCaches, 0, "devmgr caches");
    for (size_t i = 0; i < numCaches; i++) // Invalidate all read caches
    {
        caches[i].valid = false;
        caches[i].buffer = 0;
        caches[i].cacheSize = 0;
    }
}

void deviceManager_checkDrives(void)
{
    mutex_lock(&devmgr_dpMutex);
    for (size_t i = 0; i < ports.size; i++)
    {
        if (ports.data[i] != 0 && ports.data[i]->type->pollDisk != 0)
        {
            ports.data[i]->type->pollDisk(ports.data[i]);
        }

        if (ports.data[i] != 0 && ports.data[i]->type->motorOff != 0 && ports.data[i]->insertedDisk->accessRemaining == 0)
        {
            ports.data[i]->type->motorOff(ports.data[i]);
        }
    }
    mutex_unlock(&devmgr_dpMutex);
}

void deviceManager_attachPort(port_t* port)
{
    mutex_lock(&devmgr_dpMutex);
    for (size_t i=0; i<ports.size; i++)
    {
        if (ports.data[i] == 0)
        {
            ports.data[i] = port;
            mutex_unlock(&devmgr_dpMutex);
            return;
        }
    }
    array_resize(&ports, ports.size + 1);
    ports.data[ports.size - 1] = port;
    mutex_unlock(&devmgr_dpMutex);
}

void deviceManager_destructPort(port_t* port)
{
    mutex_lock(&devmgr_dpMutex);
    for (size_t i=0; i<ports.size; i++)
    {
        if (ports.data[i] == port)
        {
            ASSERT(!ports.data[i]->insertedDisk);
            ports.data[i] = 0;
            mutex_unlock(&devmgr_dpMutex);
            return;
        }
    }
    mutex_unlock(&devmgr_dpMutex);
}

void deviceManager_attachDisk(disk_t* disk)
{
    mutex_lock(&devmgr_dpMutex);
    // Later: Searching correct ID in device-File
    for (size_t i=0; i<disks.size; i++)
    {
        if (disks.data[i] == 0)
        {
            disks.data[i] = disk;
            mutex_unlock(&devmgr_dpMutex);
            return;
        }
    }
    array_resize(&disks, disks.size + 1);
    disks.data[disks.size - 1] = disk;
    mutex_unlock(&devmgr_dpMutex);
}

static void removeDisk(disk_t* disk)
{
    mutex_lock(&devmgr_dpMutex);
    for (size_t i=0; i<disks.size; i++)
    {
        if (disks.data[i] == disk)
        {
            disks.data[i] = 0;
            mutex_unlock(&devmgr_dpMutex);
            return;
        }
    }
    mutex_unlock(&devmgr_dpMutex);
}

void deviceManager_destructDisk(disk_t* disk)
{
    if (disk)
    {
        removeDisk(disk);
        deviceManager_clearCaches(disk);
        disk->port->insertedDisk = 0;
        for (uint16_t i = 0; i < PARTITIONARRAYSIZE; i++)
        {
            if (disk->partition[i])
            {
                freePartition(disk->partition[i]);
                free(disk->partition[i]);
            }
        }
    }
}

static const char* intToChars(int16_t i)
{
    static char temp[3];
    temp[1] = temp[2] = 0;
    if (i < 26)
        temp[0] = 'A' + i;
    else
    {
        temp[0] = 'A' + (i / 26)-1;
        temp[1] = 'A' + i % 26;
    }
    return temp;
}

void deviceManager_showPortList(void)
{
    textColor(HEADLINE);
    printf("\n\nAvailable ports:");
    textColor(TABLE_HEADING);
    printf("\nType            Letter\tName\t\tInserted disk");
    printf("\n----------------------------------------------------------------------");
    textColor(TEXT);

    mutex_lock(&devmgr_dpMutex);
    for (int16_t i = 0; i < ports.size; i++)
    {
        if (ports.data[i] != 0)
        {
            if      (ports.data[i]->type == &FDD)      printf("\nFDD\t");
            else if (ports.data[i]->type == &RAM)      printf("\nRAM\t");
            else if (ports.data[i]->type == &USB_OHCI) printf("\nUSB 1.1 (OHCI)");
            else if (ports.data[i]->type == &USB_UHCI) printf("\nUSB 1.0 (UHCI)");
            else if (ports.data[i]->type == &USB_EHCI) printf("\nUSB 2.0 (EHCI)");
            else if (ports.data[i]->type == &USB_XHCI) printf("\nUSB 3.0 (xHCI)");
            else if (ports.data[i]->type == &HDD)      printf("\nATA harddisk");
            else                                       printf("\nUnknown\t");

            textColor(IMPORTANT);
            printf("\t%s", intToChars(i)); // number
            textColor(TEXT);
            size_t printed = printf("\t%s", ports.data[i]->name); // The ports name
            if (printed < 9)
                putch('\t');

            if (ports.data[i]->insertedDisk != 0)
            {
                for (size_t j = 0; j < disks.size; j++)
                {
                    if (ports.data[i]->insertedDisk == disks.data[j])
                    {
                        printf("\t%u:", j+1); // Attached disk
                        break;
                    }
                }
            }
            else
            {
                printf("\t-");
            }
        }
    }
    mutex_unlock(&devmgr_dpMutex);
    textColor(TABLE_HEADING);
    printf("\n----------------------------------------------------------------------\n");
    textColor(TEXT);
}

void deviceManager_showDiskList(void)
{
    textColor(HEADLINE);
    printf("\n\nAttached disks:");
    textColor(TABLE_HEADING);
    printf("\nType\tNumber\tSize\t\tPart.\tName\t\tSerial");
    printf("\n----------------------------------------------------------------------");
    textColor(TEXT);

    mutex_lock(&devmgr_dpMutex);
    for (int16_t i = 0; i < disks.size; i++)
    {
        if (disks.data[i] != 0)
        {
            if (disks.data[i]->type == &FLOPPYDISK) /// Todo: Move to flpydsk.c, name set on floppy insertion
            {
                flpydsk_refreshVolumeName(disks.data[i]);
            }

            if (disks.data[i]->type == &FLOPPYDISK && (!disks.data[i]->partition[0] || *disks.data[i]->partition[0]->name == 0)) // Floppy workaround
                continue;

            if      (disks.data[i]->type == &FLOPPYDISK) printf("\nFloppy");
            else if (disks.data[i]->type == &RAMDISK)    printf("\nRAMdisk");
            else if (disks.data[i]->type == &USB_MSD)    printf("\nUSB MSD");
          #ifdef _ENABLE_HDD_
            else if (disks.data[i]->type == &HDDDISK)    printf("\nHDD");
          #endif
            else                                         printf("\nUnknown");

            textColor(IMPORTANT);
            printf("\t%u", i+1); // Number
            textColor(TEXT);

            if (printf("\t%Sa", disks.data[i]->size) < 8)
                putch('\t');

            for (uint8_t j=0; j < PARTITIONARRAYSIZE; j++)
            {
                if (disks.data[i]->partition[j] == 0) // Empty
                    continue;

                if (j!=0)
                {
                    printf("\n\t\t\t"); // Not first, indent
                }

                printf("\t%u\t", j); // Partition number

                if (disks.data[i]->partition[j]->name)
                {
                    size_t printed = printf("%s", disks.data[i]->partition[j]->name); // Name of disk
                    if (printed < 9)
                        putch('\t');
                }
                else
                    putch('\t');

                printf("\t%X", disks.data[i]->partition[j]->serial); // serial of partition
            }
        }
    }
    mutex_unlock(&devmgr_dpMutex);
    textColor(TABLE_HEADING);
    printf("\n----------------------------------------------------------------------\n");
    textColor(TEXT);
}

static int16_t charsToInt(const char* path)
{
    if (isalpha(path[1]))
    {
        int16_t i = path[0] - 'A' + 1;
        return i * 26 + charsToInt(path+1);
    }
    return path[0] - 'A';
}

partition_t* deviceManager_getPartition(const char* path)
{
    int16_t PortID = -1;
    int16_t DiskID = -1;
    uint8_t PartitionID = 0;
    for (size_t i=0; path[i]; i++)
    {
        if ((path[i] == ':') || (path[i] == '-'))
        {
            if (isalpha(path[0]))
            {
                PortID = charsToInt(path);
            }
            else
            {
                DiskID = atoi(path);
            }

            for (size_t j=i+1; ; j++)
            {
                if (path[j] == ':')
                {
                    PartitionID = atoi(path+i+1);
                    break;
                }

                if ((path[j] == '/') || (path[j] == '|') || (path[j] == '\\'))
                    break;

                if (!path[j] || !isdigit(path[j]))
                    return 0; // Invalid path
            }
            break;
        }
        if (!isalnum(path[i]))
        {
            return 0; // Invalid path
        }
    }

    partition_t* retVal = 0;
    mutex_lock(&devmgr_dpMutex);
    if ((PortID != -1) && (PortID < ports.size))
    {
        if (ports.data[PortID] && ports.data[PortID]->insertedDisk && PartitionID < PARTITIONARRAYSIZE)
        {
            retVal = ports.data[PortID]->insertedDisk->partition[PartitionID];
        }
    }
    else
    {
        if (DiskID == 0)
        {
            retVal = systemPartition;
        }
        else if (DiskID > 0 && DiskID <= disks.size && disks.data[DiskID - 1] && PartitionID < PARTITIONARRAYSIZE)
        {
            retVal = disks.data[DiskID - 1]->partition[PartitionID];
        }
    }
    mutex_unlock(&devmgr_dpMutex);
    return retVal;
}

FS_ERROR deviceManager_analyzeDisk(disk_t* disk)
{
    if(disk->sectorSize < 512)
        return CE_UNSUPPORTED_SECTOR_SIZE;

    uint8_t buffer[disk->sectorSize];
    FS_ERROR err = singleSectorRead(0, buffer, disk); // first sector of partition
    if (err != CE_GOOD)
    {
        printf("\nsingleSectorRead does not work! Error: %u", err);
        waitForKeyStroke();
        return err;
    }

    BPBbase_t* BPB = (BPBbase_t*)buffer; // BIOS Parameter Block (BPB)
    if (!(BPB->FATcount > 0 && BPB->bytesPerSector % 512 == 0 && BPB->bytesPerSector != 0) && // Data look not like a BPB ...
        (buffer[510] == 0x55 && buffer[511] == 0xAA)) // ... but like a Master Boot Record (MBR)
    {
        // Read partitions from MBR
        printf("\nFound MBR (DiskID: %xh):", ((uint16_t*)buffer)[440/2]);
        partitionEntry_t* entries = (partitionEntry_t*)(buffer+446);

        for (uint8_t i=0; i < 4; i++) // four entries in partition table
        {
            if (entries[i].type != 0) // valid entry
            {
                printf("\npartition entry %u: start: %u\tsize: %u\t type: ", i, entries[i].startLBA, entries[i].sizeLBA);

                disk->partition[i]         = malloc(sizeof(partition_t), 0, "partition_t");
                disk->partition[i]->start  = entries[i].startLBA;
                disk->partition[i]->size   = entries[i].sizeLBA;
                disk->partition[i]->disk   = disk;
                disk->partition[i]->serial = 0;
                disk->partition[i]->name   = 0;

                if (analyzePartition(disk->partition[i]) != CE_GOOD)
                {
                    printf("unknown");
                }
            }
            else
            {
                disk->partition[i] = 0;
            }
        }
    }
    else // There is just one partition
    {
        printf("\n     => Only single partition on disk. (type: ");

        disk->partition[0]         = malloc(sizeof(partition_t), 0, "partition_t");
        disk->partition[0]->start  = 0;
        disk->partition[0]->size   = disk->size; // Whole size of disk
        disk->partition[0]->disk   = disk;
        disk->partition[0]->serial = 0;
        disk->partition[0]->name   = 0;
        disk->partition[1]         = 0;
        disk->partition[2]         = 0;
        disk->partition[3]         = 0;

        if (analyzePartition(disk->partition[0]) != CE_GOOD)
        {
            printf("unknown)");
            return (CE_NOT_FORMATTED);
        }
        putch(')');
    }
    return (CE_GOOD);
}

#ifdef _CACHE_DIAGNOSIS_
static void logCache(void)
{
    printf("\n\nCaches:\nID\tdisk\t\tsector\tsize\t\tvalid\tsynced");
    textColor(LIGHT_GRAY);
    printf("\n--------------------------------------------------------------------------------");
    for (size_t i = 0; i < numCaches; i++)
    {
        if (caches[i].valid)
        {
            textColor(GREEN);
        }
        else
        {
            textColor(LIGHT_GRAY);
        }
        printf("\n%u:\t%Xh\t%u\t%u\t%s\t%s",
               i, caches[i].disk, caches[i].sector, caches[i].cacheSize,
               caches[i].valid?"yes":"no", caches[i].write?"no":"yes");
    }
    textColor(LIGHT_GRAY);
    printf("\n--------------------------------------------------------------------------------");
    textColor(TEXT);
    waitForKeyStroke();
}
#endif

static void flushCache(cache_t* c)
{
    if (c->write && c->valid)
    {
        // Collect continuous sectors to be written
        uint32_t sectorStart = c->sector;
        uint32_t sectorReadStart = sectorStart;
        uint32_t sectorEnd = sectorStart;
        uint32_t sectorReadEnd = sectorStart;

        bool change;
        do
        {
            change = false;
            for (size_t i = 0; i < numCaches && sectorEnd - sectorStart + 1 < c->disk->optAccSecCount; i++)
            {
                if (caches[i].valid && caches[i].disk == c->disk) // Valid cache from same disk
                {
                    if (caches[i].sector == sectorReadEnd + 1 && sectorReadStart < 0xFFFFFFFF) // Next sector
                    {
                        sectorReadEnd++;
                        if (caches[i].write)
                            sectorEnd = sectorReadEnd;
                        change = true;
                    }
                    else if (caches[i].sector == sectorReadStart - 1 && sectorReadStart > 0) // Previous
                    {
                        sectorReadStart--;
                        if (caches[i].write)
                            sectorStart = sectorReadStart;
                        change = true;
                    }
                }
            }
        } while (change);

        // Ensure that the sector in question is written
        if (c->disk->alignedAccess)
        {
            if (sectorEnd < c->sector)
            {
                if (sectorReadEnd < c->sector)
                {
                    // Write a single sector only
                    sectorStart = sectorEnd = c->sector;
                }
                else
                    // Write also sectors that don't need to be written
                    sectorEnd = sectorReadEnd;
            }
            if (sectorStart / c->disk->optAccSecCount != c->sector / c->disk->optAccSecCount)
                sectorStart = c->sector - (c->sector % c->disk->optAccSecCount);
            if (sectorEnd / c->disk->optAccSecCount != c->sector / c->disk->optAccSecCount)
                sectorEnd = c->sector + c->disk->optAccSecCount - (c->sector % c->disk->optAccSecCount) - 1;
        }
        uint32_t count = sectorEnd - sectorStart + 1;

      #ifdef _DEVMGR_DIAGNOSIS_
        printf("\nFlush caches: from %u to %u (count: %u).", sectorStart, sectorEnd, count);
      #endif
        // Collect data, put into a single buffer
        static array(uint8_t) tempBuf; // Too large for stack
        array_resize(&tempBuf, count*c->disk->sectorSize);
        for (uint32_t i = 0; i < count; i++)
        {
            for (size_t j = 0; j < numCaches; j++)
            {
                if (caches[j].valid && caches[j].sector == sectorStart + i && caches[j].disk == c->disk)
                {
                    memcpy(tempBuf.data + i*c->disk->sectorSize, caches[j].buffer, c->disk->sectorSize);
                    caches[j].write = false;
                }
            }
        }

        c->disk->accessRemaining++;
        c->disk->type->writeSectors(sectorStart, tempBuf.data, sectorEnd - sectorStart + 1, c->disk);
    }
}

void deviceManager_flushCaches(disk_t* disk)
{
    mutex_lock(&cacheMutex);
    for (size_t i = 0; i < numCaches; i++)
    {
        if (caches[i].disk == disk)
            flushCache(caches+i);
    }
    mutex_unlock(&cacheMutex);
}

void deviceManager_clearCaches(disk_t* disk)
{
    mutex_lock(&cacheMutex);
    for (size_t i = 0; i < numCaches; i++)
    {
        if (caches[i].disk == disk)
            caches[i].valid = false;
    }
    mutex_unlock(&cacheMutex);
}

void deviceManager_fillCache(uint32_t sector, disk_t* disk, const uint8_t* buffer, uint32_t count, bool write)
{
    if (numCaches == 0)
        return;

    mutex_lock(&cacheMutex);

    for (uint32_t i = 0; i < count; i++)
    {
        bool done = false;
        size_t firstInvalidCache = 0xFFFFFFFF;
        for (size_t j = 0; j < numCaches && !done; j++) // Look for cache that can be updated
        {
            if (!caches[j].valid)
                firstInvalidCache = j;
            else if (caches[j].sector == sector && caches[j].disk == disk)
            {
                if (write)
                {
                    caches[j].write = true;
                }
                memcpy(caches[j].buffer, buffer, disk->sectorSize);
                done = true;
            }
        }

        if (!done)
        {
            static size_t currCache = 0;
            if (caches[currCache].valid)
            {
                if (firstInvalidCache != 0xFFFFFFFF)
                    currCache = firstInvalidCache;
                else
                    flushCache(caches + currCache); // Write cache before its overwritten
            }

            // fill new cache
            caches[currCache].sector = sector;
            caches[currCache].disk = disk;
            caches[currCache].valid = true;
            caches[currCache].write = write;
            if (caches[currCache].cacheSize < disk->sectorSize)
            {
                free(caches[currCache].buffer);
                caches[currCache].buffer = malloc(disk->sectorSize, 0, "cache buffer");
                caches[currCache].cacheSize = disk->sectorSize;
            }
            memcpy(caches[currCache].buffer, buffer, disk->sectorSize);

            currCache++;
            currCache %= numCaches;
        }
        buffer += disk->sectorSize;
        sector++;
    }
  #ifdef _CACHE_DIAGNOSIS_
    logCache();
  #endif
    mutex_unlock(&cacheMutex);
}

FS_ERROR sectorWrite(uint32_t sector, const uint8_t* buffer, disk_t* disk)
{
  #ifdef _DEVMGR_DIAGNOSIS_
    textColor(YELLOW);
    printf("\n>>>>> sectorWrite: %u <<<<<", sector);
    textColor(TEXT);
  #endif

    deviceManager_fillCache(sector, disk, buffer, 1, true);
    disk->accessRemaining--;
    return (CE_GOOD);
}

FS_ERROR singleSectorWrite(uint32_t sector, const uint8_t* buffer, disk_t* disk)
{
    disk->accessRemaining++;
    return (sectorWrite(sector, buffer, disk));
}

FS_ERROR sectorRead(uint32_t sector, uint8_t* buffer, disk_t* disk)
{
  #ifdef _DEVMGR_DIAGNOSIS_
    textColor(0x03);
    printf("\n>>>>> sectorRead: %u <<<<<", sector);
    textColor(TEXT);
  #endif

    mutex_lock(&cacheMutex);
    for (size_t i = 0; i < numCaches; i++)
    {
        if (caches[i].valid && caches[i].sector == sector && caches[i].disk == disk)
        {
          #ifdef _DEVMGR_DIAGNOSIS_
            printf("\nsector: %u <--- read from RAM Cache", caches[i].sector);
          #endif

            memcpy(buffer, caches[i].buffer, disk->sectorSize); // Take data from read cache
            mutex_unlock(&cacheMutex);
            disk->accessRemaining--;
            return (CE_GOOD);
        }
    }
    mutex_unlock(&cacheMutex);

    uint32_t count;
    uint32_t firstSector;
    if (disk->alignedAccess)
    {
        count = disk->optAccSecCount;
        firstSector = sector - (sector % disk->optAccSecCount);
    }
    else
    {
        count = min(disk->optAccSecCount, numCaches / 2); // Fill numCaches/2 at maximum
        firstSector = sector;
    }

    // read
    mutex_lock(&cacheMutex);
    static array(uint8_t) tempBuf; // Too large for stack
    array_resize(&tempBuf, count*disk->sectorSize);
    FS_ERROR error = disk->type->readSectors(firstSector, tempBuf.data, count, disk);
    if (error == CE_GOOD)
        deviceManager_fillCache(firstSector, disk, tempBuf.data, count, false);

    memcpy(buffer, tempBuf.data + (sector - firstSector)*disk->sectorSize, disk->sectorSize);
    mutex_unlock(&cacheMutex);
    return (error);
}

FS_ERROR singleSectorRead(uint32_t sector, uint8_t* buffer, disk_t* disk)
{
    disk->accessRemaining++;
    return (sectorRead(sector, buffer, disk));
}

/*
* Copyright (c) 2010-2017 The PrettyOS Project. All rights reserved.
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
