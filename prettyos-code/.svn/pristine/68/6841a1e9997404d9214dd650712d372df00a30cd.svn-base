/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "ata.h"
#include "util/util.h"
#include "kheap.h"
#include "serial.h"
#include "timer.h"

#ifdef _ENABLE_HDD_

//static const uint32_t ataTimeout = 30000; // Technically we have to wait 30 sec for drive spinning up

static const uint32_t ataIRQCheckInterval = 200;
static const uint32_t ataIRQRetries = 30000 /*ataTimeout*/ / 200;

static const uint32_t ataPollInterval = 10;
static const uint32_t ataPollRetries = 30000 /*ataTimeout*/ / 10;


static inline void wait400NS(uint16_t p) { inportb(p); inportb(p); inportb(p); inportb(p); }


void ata_install(void)
{
    outportb(ATA_REG_PRIMARY_DEVCONTROL, 0x00);
    outportb(ATA_REG_SECONDARY_DEVCONTROL, 0x00);
}

bool ata_getRegPorts(uint16_t channel, ata_regs_t* regs)
{
    if (!regs)
        return false;

    switch (channel)
    {
    case ATA_PRIMARY_MASTER:
    case ATA_PRIMARY_SLAVE:
        regs->base = ATA_PRIMARY_BASEPORT;
        regs->devcontrol = regs->altstatus = ATA_REG_PRIMARY_DEVCONTROL;
        break;
    case ATA_SECONDARY_MASTER:
    case ATA_SECONDARY_SLAVE:
        regs->base = ATA_SECONDARY_BASEPORT;
        regs->devcontrol = regs->altstatus = ATA_REG_SECONDARY_DEVCONTROL;
        break;
    default:
        return false;
    }

    regs->data    = regs->base + ATA_REG_DATA;
    regs->feature = regs->base + ATA_REG_FEATURE;
    regs->error   = regs->base + ATA_REG_ERRORINFO;
    regs->sectors = regs->base + ATA_REG_SECTORCOUNT;
    regs->lbalo   = regs->base + ATA_REG_LBALO;
    regs->lbamid  = regs->base + ATA_REG_LBAMID;
    regs->lbahi   = regs->base + ATA_REG_LBAHI;
    regs->drive   = regs->base + ATA_REG_DRIVE;
    regs->status  = regs->base + ATA_REG_STATUS;
    regs->cmd     = regs->base + ATA_REG_CMD;

    return true;
}

bool ata_isSlave(uint16_t channel, bool* slave)
{
    if (!slave || channel > ATA_SECONDARY_SLAVE)
        return false;

    *slave = (channel == ATA_PRIMARY_SLAVE || channel == ATA_SECONDARY_SLAVE);

    return true;
}

bool ata_getIRQ(uint16_t channel, IRQ_NUM_t* irq)
{
    if (!irq)
        return false;

    switch (channel)
    {
    case ATA_PRIMARY_MASTER:
    case ATA_PRIMARY_SLAVE:
        *irq = IRQ_ATA_PRIMARY;
        break;
    case ATA_SECONDARY_MASTER:
    case ATA_SECONDARY_SLAVE:
        *irq = IRQ_ATA_SECONDARY;
        break;
    default:
        return false;
    }

    return true;
}

static inline bool waitBSY(ata_regs_t* regs)
{
    wait400NS(regs->status);

    uint8_t portval = inportb(regs->status);

    for (uint32_t i = 0; i < ataPollRetries; ++i)
    {
        if (!(portval & ATA_STATUS_BSY))
        {
            return true;
        }

        sleepMilliSeconds(ataPollInterval);
        portval = inportb(regs->status);
    }

    return false;
}

// Caller has to call irq_resetCounter before calling this function
static inline bool waitIRQ(ata_portInfo_t* port, uint8_t errMask, uint8_t* status)
{
    // Use the alternate status registers when waiting for an IRQ
    // http://wiki.osdev.org/ATA_PIO_Mode#IRQs

    for (uint32_t i = 0; i < ataIRQRetries; ++i)
    {
        if (!waitForIRQ(port->irq, ataIRQCheckInterval))
        {
            wait400NS(port->regs.altstatus);
            uint8_t driveStatus = inportb(port->regs.altstatus);

            if (driveStatus & errMask)
            {
                if (status)
                    *status = driveStatus;
                return false;
            }
        }
        else
        {
            if (status)
            {
                wait400NS(port->regs.altstatus);
                uint8_t driveStatus = inportb(port->regs.altstatus);
                *status = driveStatus;
            }
            return true;
        }
    }

    if (status)
        *status = 0xFF; // Timeout

    return false;
}

static inline bool poll(ata_portInfo_t* port, uint8_t errMask, uint8_t goodMask, uint8_t *status)
{
    wait400NS(port->regs.status);

    uint8_t portval = inportb(port->regs.status);

    for (uint32_t i = 0; i < ataPollRetries; ++i)
    {
        if (!(portval & ATA_STATUS_BSY))
        {
            if (portval & errMask)
            {
                if (status)
                    *status = portval;
                return false;
            }
            else if (portval & goodMask)
            {
                if (status)
                    *status = portval;
                return true;
            }
        }

        sleepMilliSeconds(ataPollInterval);
        portval = inportb(port->regs.status);
    }

    if (status)
        *status = 0xFF; // Timeout
    return false;
}

static inline void selectDrive(ata_portInfo_t* port, bool lba48, uint32_t sector) // highest 4 bits are used in LBA28
{
    if (lba48)
        outportb(port->regs.drive, port->slave ? 0x50 : 0x40);
    else // LBA28
        outportb(port->regs.drive, (port->slave ? 0xF0 : 0xE0) | ((sector >> 24) & 0x0F));
}

static inline void setupLba(ata_portInfo_t* port, bool lba48, uint32_t sector, uint16_t count)
{
    if (lba48)
    {
        outportb(port->regs.sectors, (count >> 8) & 0xFF);
        outportb(port->regs.lbalo, (sector >> 24) & 0xFF);
        outportb(port->regs.lbamid, 0x00); // TODO: Support bits 32-47 of LBA48
        outportb(port->regs.lbahi, 0x00);
    }

    outportb(port->regs.sectors, count & 0xFF);
    outportb(port->regs.lbalo, sector & 0xFF);
    outportb(port->regs.lbamid, (sector >> 8) & 0xFF);
    outportb(port->regs.lbahi, (sector >> 16) & 0xFF);
}

void ata_softReset(ata_portInfo_t* port)
{
    outportb(port->regs.devcontrol, inportb(port->regs.devcontrol) | ATA_DEVCTRL_SOFTRESET);
    outportb(port->regs.devcontrol, inportb(port->regs.devcontrol) & (~ATA_DEVCTRL_SOFTRESET));
}

static inline FS_ERROR ataPerformRead(ata_portInfo_t* port, bool lba48, uint16_t count, void* buf)
{
    irq_resetCounter(port->irq);
    outportb(port->regs.cmd, lba48 ? 0x24 : 0x20); // READ SECTORS EXT : READ SECTORS

    uint16_t* buf16 = buf;
    while (count)
    {
        uint8_t status;
        if (!waitIRQ(port, ATA_STATUS_ERR | ATA_STATUS_DF, &status))
        {
          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK, "[ataPerformRead] Read error: %y\n", status);
          #endif
            if (status == 0xFF)
                return CE_TIMEOUT;
            else
                return CE_BAD_SECTOR_READ;
        }

        wait400NS(port->regs.status);
        status = inportb(port->regs.status); // We have to read this register at least 1 time, or the IRQ will not fire again.
        if (!(status & ATA_STATUS_RDY && status & ATA_STATUS_DRQ))
        {
          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK,
                       "[ataPerformRead] Read error: IRQ fired, but device is not ready to accept data [%y]\n",
                       status);
          #endif
            return CE_BAD_SECTOR_READ;
        }

        irq_resetCounter(port->irq); // Interrupt fired after each DRQ block [NOT WORKING]
        repinsw(port->regs.data, buf16, 256);
        buf16 += 256;

        --count;
    }

    return CE_GOOD;
}

static inline FS_ERROR performWrite(ata_portInfo_t* port, bool lba48, uint16_t count, void* buf)
{
    outportb(port->regs.cmd, lba48 ? 0x34 : 0x30); // WRITE SECTORS EXT : WRITE SECTORS

    uint16_t* buf16 = buf;

    uint8_t status;
    while (count)
    {
        if (!poll(port, ATA_STATUS_ERR | ATA_STATUS_DF, ATA_STATUS_DRQ | ATA_STATUS_RDY, &status))
        {
          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK, "[ataPerformWrite] Write error: Device not ready %y\n", status);
          #endif
            if (status == 0xFF)
                return CE_TIMEOUT;
            else
                return CE_WRITE_ERROR;
        }

        irq_resetCounter(port->irq); // Interrupt fired after each DRQ block
        for (int i = 0; i < 256; ++i)
        {
            __asm__ volatile("jmp .+2"); // ATA needs a 'tiny delay of jmp $+2'
            //__asm__ volatile("outsw" : : "d"(hdd->regs.data), "S"(buffer++)); // Not working
            outportw(port->regs.data, *buf16++);
        }

        if (!waitIRQ(port, ATA_STATUS_ERR | ATA_STATUS_DF, &status))
        {
          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK, "[ataPerformWrite] Write error: Data could not be written %y\n", status);
          #endif
            if (status == 0xFF)
                return CE_TIMEOUT;
            else
                return CE_WRITE_ERROR;
        }

        --count;
    }

    if (!poll(port, ATA_STATUS_ERR | ATA_STATUS_DF, ATA_STATUS_RDY, &status))
    {
      #ifdef _HDD_DIAGNOSIS_
        serial_log(SER_LOG_HRDDSK, "[ataPerformWrite] Write error: Drive got not ready for cache flush %y\n", status);
      #endif
        if (status == 0xFF)
            return CE_TIMEOUT;
        else
            return CE_WRITE_ERROR;
    }

    outportb(port->regs.cmd, lba48 ? 0xEA : 0xE7); // CACHE FLUSH EXT : CACHE FLUSH

    if (!poll(port, ATA_STATUS_ERR | ATA_STATUS_DF, ATA_STATUS_RDY, &status))
    {
      #ifdef _HDD_DIAGNOSIS_
        serial_log(SER_LOG_HRDDSK, "[ataPerformWrite] Write error: Cache flush failed %y\n", status);
      #endif
        if (status == 0xFF)
            return CE_TIMEOUT;
        else
            return CE_WRITE_ERROR;
    }

    return CE_GOOD;
}

FS_ERROR ata_accessSectors(uint32_t sector, uint16_t count, void* buf,
                           ata_portInfo_t* port, ATA_ACCESS access)
{
    bool useLba48 = (sector > 0x0FFFFFFF || count > 0xFF);
    if ((useLba48 && !port->supportsLba48) || !count || access < ATA_READ || access > ATA_WRITE)
        return CE_INVALID_ARGUMENT;

    mutex_lock(port->rwLock);

    selectDrive(port, useLba48, sector);

    uint8_t stat = 0;
    if (!poll(port, ATA_STATUS_ERR | ATA_STATUS_DF, ATA_STATUS_RDY, &stat))
    {
      #ifdef _HDD_DIAGNOSIS_
        serial_log(SER_LOG_HRDDSK, "[ataAccessSectors] Drive was not ready in 30 sec: %y\n", stat);
      #endif

        mutex_unlock(port->rwLock);
        return CE_TIMEOUT;
    }

    setupLba(port, useLba48, sector, count);

    FS_ERROR ret;
    if (access == ATA_WRITE)
        ret = performWrite(port, useLba48, count, buf);
    else // ATA_READ
        ret = ataPerformRead(port, useLba48, count, buf);

    if (ret != CE_GOOD)
        ata_softReset(port);

    mutex_unlock(port->rwLock);

    return ret;
}

ATA_DEVICE_TYPE ata_identify(ATA_CHANNEL channel, uint16_t *output)
{
    ata_regs_t regs;
    bool slave;
    if (!(ata_getRegPorts(channel, &regs) && ata_isSlave(channel, &slave)))
        return ATA_UNKOWN_TYPE;

    outportb(regs.drive, slave ? 0xB0 : 0xA0); // Select drive

    wait400NS(regs.status);

    if (inportb(regs.status) == 0xFF) // Floating port
    {
      #ifdef _HDD_DIAGNOSIS_
        serial_log(SER_LOG_HRDDSK, "[hdd_ATAIdentify] floating port: %d\n", (int32_t)channel);
      #endif
        return ATA_NOT_IN_USE;
    }

    if (!waitBSY(&regs))
    {
      #ifdef _HDD_DIAGNOSIS_
        serial_log(SER_LOG_HRDDSK, "[hdd_ATAIdentify] Drive not ready!(l %d)\n", __LINE__);
      #endif
        return ATA_UNKOWN_TYPE;
    }

    outportb(regs.sectors, 0x00);
    outportb(regs.lbalo, 0x00);
    outportb(regs.lbamid, 0x00);
    outportb(regs.lbahi, 0x00);
    outportb(regs.cmd, 0xEC); // IDENTIFY

    wait400NS(regs.status);

    uint8_t tmp = inportb(regs.status);
    if (tmp == 0) // Drive does not exist
    {
      #ifdef _HDD_DIAGNOSIS_
        serial_log(SER_LOG_HRDDSK, "[hdd_ATAIdentify] drive does not exist (returned %y from port %x): %d\n",
                   tmp, (uint16_t)(regs.status), (int32_t)channel);
      #endif
        return ATA_NOT_IN_USE;
    }

    if (!waitBSY(&regs))
    {
      #ifdef _HDD_DIAGNOSIS_
        serial_log(SER_LOG_HRDDSK, "[hdd_ATAIdentify] Drive was busy for more than 30 sec!(l %d)\n", __LINE__);
      #endif
        return ATA_UNKOWN_TYPE;
    }

    uint8_t portval = inportb(regs.status);

    for (uint32_t i = 0; i < ataPollRetries; ++i)
    {
        if (portval & ATA_STATUS_DRQ)
        {
            break;
        }

        if (portval & ATA_STATUS_ERR)
        {
          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK, "[hdd_ATAIdentify] Error during IDENTIFY: %y\n", portval);
          #endif
            return ATA_UNKOWN_TYPE;
        }

        if (inportb(regs.lbamid) || inportb(regs.lbahi))
        {
          #ifdef _HDD_DIAGNOSIS_
            serial_log(SER_LOG_HRDDSK,
                       "[hdd_ATAIdentify] Nonstandard device!\n");
          #endif
            return ATA_UNKOWN_TYPE; // TODO: This is may be an ATAPI device
        }

        sleepMilliSeconds(ataPollInterval);
        portval = inportb(regs.status);
    }

    repinsw(regs.data, output, 256);
    return ATA_HARDDISK; // TODO: Check for the removable media interface. Cardreaders might be detected as HDDs atm.
}
#endif

/*
* Copyright (c) 2014-2015 The PrettyOS Project. All rights reserved.
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
