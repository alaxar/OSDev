/*
 * Copyright (c) 2007, 2008 The tyndur Project. All rights reserved.
 *
 * This code is derived from software contributed to the tyndur Project
 * by Kevin Wolf.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT HOLDERS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _E1000_DEVICE_H_
#define _E1000_DEVICE_H_

#include "os.h"

#include "cdi.h"
#include "cdi/net.h"

/* Register */

enum {
    REG_CTL             =    0x0,
    REG_STATUS          =    0x8,
    REG_EECD            =   0x10, /* EEPROM Control */
    REG_EEPROM_READ     =   0x14, /* EERD */
    REG_VET             =   0x38, /* VLAN */

    REG_INTR_CAUSE      =   0xc0, /* ICR */
    REG_INTR_MASK       =   0xd0, /* IMS */
    REG_INTR_MASK_CLR   =   0xd8, /* IMC */


    REG_RX_CTL          =  0x100,
    REG_TX_CTL          =  0x400,

    REG_RXDESC_ADDR_LO  = 0x2800,
    REG_RXDESC_ADDR_HI  = 0x2804,
    REG_RXDESC_LEN      = 0x2808,
    REG_RXDESC_HEAD     = 0x2810,
    REG_RXDESC_TAIL     = 0x2818,

    REG_RX_DELAY_TIMER  = 0x2820,
    REG_RADV            = 0x282c,


    REG_TXDESC_ADDR_LO  = 0x3800,
    REG_TXDESC_ADDR_HI  = 0x3804,
    REG_TXDESC_LEN      = 0x3808,
    REG_TXDESC_HEAD     = 0x3810,
    REG_TXDESC_TAIL     = 0x3818,

    REG_TX_DELAY_TIMER  = 0x3820,
    REG_TADV            = 0x382c,

    REG_RECV_ADDR_LIST  = 0x5400, /* RAL */
};

enum {
    CTL_AUTO_SPEED  = (1 <<  5), /* ASDE */
    CTL_LINK_UP     = (1 <<  6), /* SLU  */
    CTL_RESET       = (1 << 26), /* RST */
    CTL_PHY_RESET   = (1 << 31),
};

enum {
    STATUS_LINK_UP  = (1 <<  1), /* LU */
};

enum {
    RCTL_ENABLE     = (1 <<  1),
    RCTL_BROADCAST  = (1 << 15), /* BAM */
    RCTL_2K_BUFSIZE = (0 << 16), /* BSIZE */
};

enum {
    TCTL_ENABLE     = (1 <<  1),
    TCTL_PADDING    = (1 <<  3),
    TCTL_COLL_TSH   = (0x0f <<  4), /* CT - Collision Threshold */

    /* e1000 specific values */
    TCTL_COLL_DIST_E1000    = (0x40 << 12), /* COLD - Collision Distance */

    /* e1000e specific values */
    TCTL_COLL_DIST_E1000E   = (0x3f << 12), /* COLD - Collision Distance */
    TCTL_RRTHRESH           = (0x03 << 28), /* Read Request Threshold */
};

enum {
    ICR_TRANSMIT    = (1 <<  0),
    ICR_LINK_CHANGE = (1 <<  2),
    ICR_RECEIVE     = (1 <<  7),
};

enum {
    FLASH_REG_GLFPR     =   0x0, /* Gigabit LAN Flash Primary Region */
    FLASH_REG_HSFSTS    =   0x4, /* Hardware Sequencing Flash Status */
    FLASH_REG_HSFCTL    =   0x6, /* Hardware Sequencing Flash Control */
    FLASH_REG_FADDR     =   0x8, /* Flash Address Register */
    FLASH_REG_FDATA0    =  0x10, /* Flash Data 0 Register */
};

enum {
    HSFSTS_FDONE        = (1 <<  0),
    HSFSTS_FCERR        = (1 <<  1),
    HSFSTS_AEL          = (1 <<  2),
    HSFSTS_SCIP         = (1 <<  5),
    HSFSTS_FDV          = (1 << 14),
};

enum {
    HSFCTL_FGO          = (1 <<  0),
    HSFCTL_FCYCLE_READ  = (0 <<  1),
    /*HSFCTL_FCYCLE_WRITE = (2 <<  1),*/
    HSFCTL_FDBC_BYTE    = (0 <<  8),
    HSFCTL_FDBC_WORD    = (1 <<  8),
    HSFCTL_FDBC_DWORD   = (3 <<  8),
};

enum {
    EEPROM_OFS_MAC      = 0x0,
};

#define RAH_VALID   (1 << 31) /* AV */

#define EERD_E1000_START    (1 <<  0)
#define EERD_E1000_DONE     (1 <<  4)

#define EERD_E1000E_START   (1 <<  0)
#define EERD_E1000E_DONE    (1 <<  1)

/* EEPROM/Flash Control */
#define E1000_EECD_SK        0x00000001 /* EEPROM Clock */
#define E1000_EECD_CS        0x00000002 /* EEPROM Chip Select */
#define E1000_EECD_DI        0x00000004 /* EEPROM Data In */
#define E1000_EECD_DO        0x00000008 /* EEPROM Data Out */
#define E1000_EECD_FWE_MASK  0x00000030
#define E1000_EECD_FWE_DIS   0x00000010 /* Disable FLASH writes */
#define E1000_EECD_FWE_EN    0x00000020 /* Enable FLASH writes */
#define E1000_EECD_FWE_SHIFT 4
#define E1000_EECD_SIZE      0x00000200 /* EEPROM Size (0=64 word 1=256 word) */
#define E1000_EECD_REQ       0x00000040 /* EEPROM Access Request */
#define E1000_EECD_GNT       0x00000080 /* EEPROM Access Grant */
#define E1000_EECD_PRES      0x00000100 /* EEPROM Present */

#define EEPROM_READ_OPCODE  0x6

/* Allgemeine Definitionen */

#define TX_BUFFER_SIZE  2048
//#define TX_BUFFER_NUM   64

#define RX_BUFFER_SIZE  2048
//#define RX_BUFFER_NUM   64

// Die Anzahl von Deskriptoren muss jeweils ein vielfaches von 8 sein
#define RX_BUFFER_NUM   8
#define TX_BUFFER_NUM   8

struct e1000_tx_descriptor {
    uint64_t            buffer;
    uint16_t            length;
    uint8_t             checksum_offset;
    uint8_t             cmd;
    uint8_t             status;
    uint8_t             checksum_start;
    uint16_t            special;
} __attribute__((packed)) __attribute__((aligned (4)));
CDI_BUILD_BUG_ON((sizeof(struct e1000_tx_descriptor) * TX_BUFFER_NUM) % 128);

enum {
    TX_CMD_EOP  = 0x01,
    TX_CMD_IFCS = 0x02,
};

struct e1000_rx_descriptor {
    uint64_t            buffer;
    uint16_t            length;
    uint16_t            padding;
    uint8_t             status;
    uint8_t             error;
    uint16_t            padding2;
} __attribute__((packed)) __attribute__((aligned (4)));
CDI_BUILD_BUG_ON((sizeof(struct e1000_rx_descriptor) * RX_BUFFER_NUM) % 128);

struct e1000_device;

struct e1000_model {
    uint16_t vendor_id;
    uint16_t device_id;

    uint32_t tctl_flags;
    uint32_t (*eeprom_read)(struct e1000_device *device, uint16_t offset);
};

struct e1000_device {
    struct cdi_net_device       net;

    uintptr_t                   phys;

    volatile struct e1000_tx_descriptor  tx_desc[TX_BUFFER_NUM] __attribute__((aligned(16)));
    uint8_t                     tx_buffer[TX_BUFFER_NUM * TX_BUFFER_SIZE];
    uint32_t                    tx_cur_buffer;

    volatile struct e1000_rx_descriptor  rx_desc[RX_BUFFER_NUM] __attribute__((aligned(16)));
    uint8_t                     rx_buffer[RX_BUFFER_NUM * RX_BUFFER_SIZE];
    uint32_t                    rx_cur_buffer;

    void*                       mem_base;
    void*                       flash_base;
    uint32_t                    flash_base_offset;
    uint32_t                    flash_size;
    uint8_t                     revision;

    struct e1000_model*         model;
};

struct cdi_device* e1000_init_device(struct cdi_bus_data* bus_data);
void e1000_remove_device(struct cdi_device* device);

void e1000_send_packet
    (struct cdi_net_device* device, void* data, size_t size);


#endif
