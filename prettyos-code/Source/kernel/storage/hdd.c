/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "hdd.h"
#include "ata.h"
#include "devicemanager.h"
#include "kheap.h"
#include "serial.h"


#ifdef _ENABLE_HDD_
void hdd_install(void)
{
    uint16_t buf[256]; // TODO: Proper struct

    mutex_t* ataPrimaryChannelLock = 0;
    mutex_t* ataSecondaryChannelLock = 0;

    for (int i = ATA_PRIMARY_MASTER; i <= ATA_SECONDARY_SLAVE; ++i)
    {
        if (ata_identify((ATA_CHANNEL)i, buf) == ATA_HARDDISK)
        {
          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK, "[hdd_install] Disk connected in ATA-Channel %d\n", i);
          #endif
            ata_portInfo_t* portInfo = malloc(sizeof(ata_portInfo_t), 0, "ata_portInfo_t");
            portInfo->channel = (ATA_CHANNEL)i;

            if (!(ata_getRegPorts(portInfo->channel, &portInfo->regs) &&
                    ata_getIRQ(portInfo->channel, &portInfo->irq) &&
                    ata_isSlave(portInfo->channel, &portInfo->slave)))
            {
                // This is a fatal error, so report it even when _HDD_DIAGNOSIS_ is not set
                serial_log(SER_LOG_HRDDSK,
                           "[hdd_install] FATAL ERROR: Setup of hdd [%d] failed (this should never happen)\n",
                           i);
                free(portInfo);
                continue;
            }

            portInfo->supportsDma = false; // TODO
            portInfo->supportsLba48 = false; // TODO

            if (i == ATA_PRIMARY_MASTER || i == ATA_PRIMARY_SLAVE)
            {
                if (!ataPrimaryChannelLock)
                    ataPrimaryChannelLock = mutex_create();
                portInfo->rwLock = ataPrimaryChannelLock;

                outportb(ATA_REG_PRIMARY_DEVCONTROL, 0x00);
            }
            else if (i == ATA_SECONDARY_MASTER || i == ATA_SECONDARY_SLAVE)
            {
                if (!ataSecondaryChannelLock)
                    ataSecondaryChannelLock = mutex_create();
                portInfo->rwLock = ataSecondaryChannelLock;

                outportb(ATA_REG_SECONDARY_DEVCONTROL, 0x00);
            }

            portInfo->drive = malloc(sizeof(port_t), 0, "hdd-Port");

            portInfo->drive->type = &HDD;
            portInfo->drive->data = portInfo;
            portInfo->drive->insertedDisk = malloc(sizeof(disk_t), 0, "hdd-Disk");

            portInfo->drive->insertedDisk->type = &HDDDISK;
            portInfo->drive->insertedDisk->data = portInfo;
            portInfo->drive->insertedDisk->port = portInfo->drive;
            // buf[60] and buf[61] give the total size of the lba28 sectors
            portInfo->drive->insertedDisk->size = ((uint64_t)(*(uint32_t*)&buf[60])) * 512;
            portInfo->drive->insertedDisk->headCount = 0;
            portInfo->drive->insertedDisk->optAccSecCount = 1; // TODO: Set appropriate value
            portInfo->drive->insertedDisk->alignedAccess = false;
            portInfo->drive->insertedDisk->secPerTrack = 0;
            portInfo->drive->insertedDisk->sectorSize = 512;
            portInfo-> drive->insertedDisk->accessRemaining = 0;

            memset(portInfo-> drive->insertedDisk->partition, 0, sizeof(partition_t*) * PARTITIONARRAYSIZE);

          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK, "[hdd_install] Size of disk at channel %d is %d\n",
                       i, portInfo->drive->insertedDisk->size);
          #endif

            deviceManager_attachDisk(portInfo->drive->insertedDisk); // disk == hard disk
            deviceManager_attachPort(portInfo->drive);
            deviceManager_analyzeDisk(portInfo->drive->insertedDisk);

            ata_softReset(portInfo);
        }
    }
}

FS_ERROR hdd_writeSectors(uint32_t sector, const void* buf, uint32_t count, disk_t* device)
{
    return ata_accessSectors(sector, count, (void*)buf, device->data, ATA_WRITE);
}

FS_ERROR hdd_readSectors(uint32_t sector, void* buf, uint32_t count, disk_t* device)
{
    return ata_accessSectors(sector, count, buf, device->data, ATA_READ);
}
#endif

/*
* Copyright (c) 2012-2015 The PrettyOS Project. All rights reserved.
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