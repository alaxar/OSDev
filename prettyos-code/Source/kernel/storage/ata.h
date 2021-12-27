#ifndef ATA_H
#define ATA_H

#include "devicemanager.h"
#include "irq.h"
#include "util/types.h"
#include "tasking/synchronisation.h"

#define ATA_PRIMARY_BASEPORT 0x1F0
#define ATA_SECONDARY_BASEPORT 0x170

#define ATA_REG_PRIMARY_DEVCONTROL 0x3F6
#define ATA_REG_SECONDARY_DEVCONTROL 0x376

#define ATA_REG_DATA 0
#define ATA_REG_FEATURE 1
#define ATA_REG_ERRORINFO 1
#define ATA_REG_SECTORCOUNT 2
#define ATA_REG_LBALO 3
#define ATA_REG_LBAMID 4
#define ATA_REG_LBAHI 5
#define ATA_REG_DRIVE 6
#define ATA_REG_STATUS 7
#define ATA_REG_CMD 7

#define ATA_STATUS_ERR 0x01
#define ATA_STATUS_DRQ 0x08
#define ATA_STATUS_SRV 0x10
#define ATA_STATUS_DF  0x20
#define ATA_STATUS_RDY 0x40
#define ATA_STATUS_BSY 0x80

#define ATA_DEVCTRL_DISABLE_INTERRUPTS 0x02
#define ATA_DEVCTRL_SOFTRESET 0x04
#define ATA_DEVCTRL_HOB 0x80

typedef enum
{
    ATA_PRIMARY_MASTER,
    ATA_PRIMARY_SLAVE,
    ATA_SECONDARY_MASTER,
    ATA_SECONDARY_SLAVE
} ATA_CHANNEL;

typedef struct
{
    uint16_t base, data, feature, error, sectors, lbalo, lbamid, lbahi, drive,
             status, cmd, devcontrol, altstatus;
} ata_regs_t;

typedef struct
{
    mutex_t* rwLock;
    port_t* drive;

    ATA_CHANNEL channel;

    IRQ_NUM_t irq;
    ata_regs_t regs;

    bool slave;
    bool supportsDma; //Reserved for later use
    bool supportsLba48;
} ata_portInfo_t;

typedef enum
{
    ATA_READ,
    ATA_WRITE
} ATA_ACCESS;

typedef enum
{
    ATA_HARDDISK, // HDD, SSD, ... (non-Packet-Interface and not removable media)
    ATA_REMOVABLE, // Card readers and so on [not used yet]
    ATA_PACKET_INTERFACE, // ATAPI-Devices, used for CD-ROM-Drives [not used yet]
    ATA_NOT_IN_USE,
    ATA_UNKOWN_TYPE
} ATA_DEVICE_TYPE;

void ata_install(void);
void ata_softReset(ata_portInfo_t* port);
FS_ERROR ata_accessSectors(uint32_t sector, uint16_t count, void* buf, ata_portInfo_t* port, ATA_ACCESS access);

// Helpers
ATA_DEVICE_TYPE ata_identify(ATA_CHANNEL channel, uint16_t* output);
bool ata_getRegPorts(uint16_t channel, ata_regs_t* regs);
bool ata_isSlave(uint16_t channel, bool* slave);
bool ata_getIRQ(uint16_t channel, IRQ_NUM_t* irq);

#endif
