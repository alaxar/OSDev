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

#include "os.h"
#include "video/console.h"
#include "util/util.h"

#include "cdi.h"
#include "cdi/misc.h"
#include "cdi/pci.h"
#include "cdi/mem.h"

#include "e1000.h"
#include "e1000_io.h"

#undef DEBUG

#define PHYS(netcard, field) \
    (netcard->phys + offsetof(struct e1000_device, field))

static void e1000_handle_interrupt(struct cdi_device* device);
static uint64_t get_mac_address(struct e1000_device* device);

static uint32_t e1000_reg_read(struct e1000_device *device, uint16_t offset)
{
    return reg_inl(device, offset);
}

static void e1000_reg_write(struct e1000_device *device, uint16_t offset, uint32_t value)
{
    reg_outl(device, offset, value);
}

static void e1000_write_flush(struct e1000_device *device)
{
    e1000_reg_read(device, REG_STATUS);
}

static void e1000_raise_eeprom_clock(struct e1000_device *device, uint32_t *eecd)
{
    *eecd |= E1000_EECD_SK;
    e1000_reg_write(device, REG_EECD, *eecd);
    e1000_write_flush(device);
    cdi_sleep_ms(5);
}

static void e1000_lower_eeprom_clock(struct e1000_device *device, uint32_t *eecd)
{
    *eecd &= ~E1000_EECD_SK;
    e1000_reg_write(device, REG_EECD, *eecd);
    e1000_write_flush(device);
    cdi_sleep_ms(5);
}

static void e1000_write_eeprom_bits(struct e1000_device *device, uint16_t data, uint16_t bitcount)
{
    uint32_t eecd, mask;

    mask = 0x1 << (bitcount - 1);
    eecd = e1000_reg_read(device, REG_EECD);
    eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
    do
    {
        eecd &= ~E1000_EECD_DI;
        if(data & mask)
            eecd |= E1000_EECD_DI;
        e1000_reg_write(device, REG_EECD, eecd);
        e1000_write_flush(device);

        cdi_sleep_ms(5);

        e1000_raise_eeprom_clock(device, &eecd);
        e1000_lower_eeprom_clock(device, &eecd);

        mask >>= 1;
    }
    while(mask);

    eecd &= ~E1000_EECD_DI;
    e1000_reg_write(device, REG_EECD, eecd);
}

static uint16_t e1000_read_eeprom_bits(struct e1000_device *device)
{
    uint32_t eecd, i;
    uint16_t data;

    eecd = e1000_reg_read(device, REG_EECD);
    eecd &= ~(E1000_EECD_DO | E1000_EECD_DI);
    data = 0;

    for(i = 0; i < 16; i++)
    {
        data <<= 1;
        e1000_raise_eeprom_clock(device, &eecd);

        eecd = e1000_reg_read(device, REG_EECD);

        eecd &= ~E1000_EECD_DI;
        if(eecd & E1000_EECD_DO)
            data |= 1;

        e1000_lower_eeprom_clock(device, &eecd);
    }

    return data;
}

static void e1000_prep_eeprom(struct e1000_device *device)
{
    uint32_t eecd = e1000_reg_read(device, REG_EECD);

    eecd &= ~(E1000_EECD_SK | E1000_EECD_DI);
    e1000_reg_write(device, REG_EECD, eecd);

    eecd |= E1000_EECD_CS;
    e1000_reg_write(device, REG_EECD, eecd);
}

static void e1000_standby_eeprom(struct e1000_device *device)
{
    uint32_t eecd = e1000_reg_read(device, REG_EECD);

    eecd &= ~(E1000_EECD_CS | E1000_EECD_SK);
    e1000_reg_write(device, REG_EECD, eecd);
    e1000_write_flush(device);
    cdi_sleep_ms(5);

    eecd |= E1000_EECD_SK;
    e1000_reg_write(device, REG_EECD, eecd);
    e1000_write_flush(device);
    cdi_sleep_ms(5);

    eecd |= E1000_EECD_CS;
    e1000_reg_write(device, REG_EECD, eecd);
    e1000_write_flush(device);
    cdi_sleep_ms(5);

    eecd &= ~(E1000_EECD_SK);
    e1000_reg_write(device, REG_EECD, eecd);
    e1000_write_flush(device);
    cdi_sleep_ms(5);
}

static uint32_t e1000_read_uwire(struct e1000_device *device, uint16_t offset)
{
    uint32_t eecd, i = 0;
    int large_eeprom = 0;

    // TODO: check for post-82544 chip, only run this handling if so
    eecd = e1000_reg_read(device, REG_EECD);
    if(eecd & E1000_EECD_SIZE)
        large_eeprom = 1;
    eecd |= E1000_EECD_REQ;
    e1000_reg_write(device, REG_EECD, eecd);
    eecd = e1000_reg_read(device, REG_EECD);
    while((!(eecd & E1000_EECD_GNT)) && (i++ < 100))
    {
        cdi_sleep_ms(1);
        eecd = e1000_reg_read(device, REG_EECD);
    }
    if(!(eecd & E1000_EECD_GNT))
    {
        eecd &= ~E1000_EECD_REQ;
        e1000_reg_write(device, REG_EECD, eecd);
        return (uint32_t) -1;
    }

    e1000_prep_eeprom(device);

    e1000_write_eeprom_bits(device, EEPROM_READ_OPCODE, 3);
    if(large_eeprom)
        e1000_write_eeprom_bits(device, offset, 8);
    else
        e1000_write_eeprom_bits(device, offset, 6);

    uint32_t data = e1000_read_eeprom_bits(device);

    e1000_standby_eeprom(device);

    // TODO: check for post-82544 chip
    eecd = e1000_reg_read(device, REG_EECD);
    eecd &= ~E1000_EECD_REQ;
    e1000_reg_write(device, REG_EECD, eecd);

    return data;
}

static uint32_t do_read_eerd(struct e1000_device *device, uint16_t offset,
                             uint32_t start_flag, uint32_t done_flag)
{
    uint32_t eerd, i;

    reg_outl(device, REG_EEPROM_READ, (offset << 8) | start_flag);
    for(i = 0; i < 100; i++)
    {
        eerd = reg_inl(device, REG_EEPROM_READ);
        if (eerd & done_flag)
            break;
        cdi_sleep_ms(1);
    }

    if (eerd & done_flag)
        return (eerd >> 16) & 0xFFFF;
    else
        return (uint32_t) -1;
}

static uint32_t e1000_read_eerd(struct e1000_device *device, uint16_t offset)
{
    return do_read_eerd(device, offset, EERD_E1000_START, EERD_E1000_DONE);
}

#if 0
static uint32_t e1000e_read_eerd(struct e1000_device *device, uint16_t offset)
{
    return do_read_eerd(device, offset, EERD_E1000E_START, EERD_E1000E_DONE);
}
#endif

static uint32_t e1000e_read_flash(struct e1000_device *device, uint16_t offset)
{
    uint16_t hsfsts;
    uint32_t linear_address;

    /* Convert from words to bytes */
    offset *= 2;

    if (device->flash_base == NULL) {
        printf("e1000: Cannot read from flash without flash BAR\n");
        return (uint32_t) -1;
    }

    if (offset + 2 > device->flash_size) {
        printf("e1000: Cannot read from flash offset %d (size %d)\n",
               offset, device->flash_size);
        return (uint32_t) -1;
    }

    /* Check that hardware sequencing is supported */
    hsfsts = flash_reg_inw(device, FLASH_REG_HSFSTS);
    if ((hsfsts & HSFSTS_FDV) == 0) {
        /* TODO Implement access using Software Sequencing registers */
        printf("e1000: Cannot read from flash without hardware sequencing\n");
        return (uint32_t) -1;
    }

    /* Check that no SPI cycle is in progress */
    if (hsfsts & HSFSTS_SCIP) {
        printf("e1000: Cannot read from flash while SPI cycle in progress\n");
        return (uint32_t) -1;
    }

    /* Start a new cycle */
    linear_address = device->flash_base_offset + offset;
    flash_reg_outl(device, FLASH_REG_FADDR, linear_address);

    flash_reg_outw(device, FLASH_REG_HSFSTS,
                   HSFSTS_FDONE | HSFSTS_FCERR | HSFSTS_AEL);
    flash_reg_outw(device, FLASH_REG_HSFCTL,
                   HSFCTL_FGO | HSFCTL_FCYCLE_READ | HSFCTL_FDBC_WORD);

    CDI_CONDITION_WAIT_SLEEP(
        flash_reg_inw(device, FLASH_REG_HSFSTS) & HSFSTS_FDONE, 100, 1);

    hsfsts = flash_reg_inw(device, FLASH_REG_HSFSTS);
    if ((hsfsts & (HSFSTS_FDONE | HSFSTS_FCERR)) == HSFSTS_FDONE) {
        return flash_reg_inl(device, FLASH_REG_FDATA0);
    } else if ((hsfsts & HSFSTS_FDONE) == 0){
        printf("e1000: Timeout while reading from flash\n");
    } else {
        printf("e1000: Error while reading from flash\n");
    }

    return (uint32_t) -1;
}

static uint16_t e1000_eeprom_read(struct e1000_device* device, uint16_t offset)
{
    static int eerd_safe = 1;

    uint32_t data = 0;
    if (eerd_safe)
        data = device->model->eeprom_read(device, offset);
    if(!eerd_safe || (data == ((uint32_t) -1)))
    {
        eerd_safe = 0;

        data = e1000_read_uwire(device, offset);
        if(data == ((uint32_t) -1))
        {
            printf("e1000: couldn't read eeprom via EERD or uwire!");
            return (0);
        }
    }

    return (uint16_t) (data & 0xFFFF);
}

static void reset_nic(struct e1000_device* netcard)
{
    uint64_t mac;
    int i;

    // Rx/Tx deaktivieren
    reg_outl(netcard, REG_RX_CTL, 0);
    reg_outl(netcard, REG_TX_CTL, 0);

    // Reset ausfuehren
    reg_outl(netcard, REG_CTL, CTL_PHY_RESET);
    cdi_sleep_ms(10);

    reg_outl(netcard, REG_CTL, CTL_RESET);
    cdi_sleep_ms(10);
    while (reg_inl(netcard, REG_CTL) & CTL_RESET);

    // Flash-Basisadresse
    if (netcard->flash_base) {
        uint32_t glfpr = flash_reg_inl(netcard, FLASH_REG_GLFPR);
        uint32_t base = (glfpr & 0x1fff) << 12;
        uint32_t limit = ((glfpr >> 16) & 0x1fff) << 12;

        netcard->flash_base_offset = base;
        netcard->flash_size = limit - base;

        printf("e1000: Flash base offset %x; size %x\n", base, limit - base);
    }

    // Kontrollregister initialisieren
    reg_outl(netcard, REG_CTL, CTL_AUTO_SPEED | CTL_LINK_UP);

    // Rx/Tx-Ring initialisieren
    reg_outl(netcard, REG_RXDESC_ADDR_HI, 0);
    reg_outl(netcard, REG_RXDESC_ADDR_LO, PHYS(netcard, rx_desc[0]));
    //printf("e1000: RX descriptors at %x\n", PHYS(netcard, rx_desc[0]));
    reg_outl(netcard, REG_RXDESC_LEN,
        RX_BUFFER_NUM * sizeof(struct e1000_rx_descriptor));
    reg_outl(netcard, REG_RXDESC_HEAD, 0);
    reg_outl(netcard, REG_RXDESC_TAIL, RX_BUFFER_NUM - 1);
    reg_outl(netcard, REG_RX_DELAY_TIMER, 0);
    reg_outl(netcard, REG_RADV, 0);

    reg_outl(netcard, REG_TXDESC_ADDR_HI, 0);
    reg_outl(netcard, REG_TXDESC_ADDR_LO, PHYS(netcard, tx_desc[0]));
    reg_outl(netcard, REG_TXDESC_LEN,
        TX_BUFFER_NUM * sizeof(struct e1000_tx_descriptor));
    reg_outl(netcard, REG_TXDESC_HEAD, 0);
    reg_outl(netcard, REG_TXDESC_TAIL, 0);
    reg_outl(netcard, REG_TX_DELAY_TIMER, 0);
    reg_outl(netcard, REG_TADV, 0);

    // VLANs deaktivieren
    reg_outl(netcard, REG_VET, 0);

    // MAC-Filter
    mac = get_mac_address(netcard);
    reg_outl(netcard, REG_RECV_ADDR_LIST, mac & 0xFFFFFFFF);
    reg_outl(netcard, REG_RECV_ADDR_LIST + 4,
        ((mac >> 32) & 0xFFFF) | RAH_VALID);

    netcard->net.mac = mac;
    //printf("e1000: MAC-Adresse: %012llx\n", (uint64_t) netcard->net.mac);

    // Rx-Deskriptoren aufsetzen
    for (i = 0; i < RX_BUFFER_NUM; i++) {
        netcard->rx_desc[i].length = RX_BUFFER_SIZE;
        netcard->rx_desc[i].buffer = PHYS(netcard, rx_buffer[i * RX_BUFFER_SIZE]);

#ifdef DEBUG
        printf("e1000: [%d] Rx: Buffer @ phys %08x, Desc @ phys %08x\n",
            i,
            netcard->rx_desc[i].buffer,
            PHYS(netcard, rx_desc[i]));
#endif
    }

    netcard->tx_cur_buffer = 0;
    netcard->rx_cur_buffer = 0;

    // Rx/Tx aktivieren
    reg_outl(netcard, REG_RX_CTL, RCTL_ENABLE | RCTL_BROADCAST
        | RCTL_2K_BUFSIZE);
    reg_outl(netcard, REG_TX_CTL, TCTL_ENABLE | TCTL_PADDING
        | TCTL_COLL_TSH | netcard->model->tctl_flags);
}


static uint64_t get_mac_address(struct e1000_device* device)
{
    uint64_t mac = 0;
    int i;

    for (i = 0; i < 3; i++) {
        mac |= (uint64_t) e1000_eeprom_read(
            device, EEPROM_OFS_MAC + i) << (i * 16);
    }

    return mac;
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static struct e1000_model models[] = {
    {
        .vendor_id      = 0x8086,
        .device_id      = 0x1004,
        .tctl_flags     = TCTL_COLL_DIST_E1000,
        .eeprom_read    = e1000_read_eerd,
    }, {
        .vendor_id      = 0x8086,
        .device_id      = 0x100e,
        .tctl_flags     = TCTL_COLL_DIST_E1000,
        .eeprom_read    = e1000_read_eerd,
    }, {
        .vendor_id      = 0x8086,
        .device_id      = 0x100f,
        .tctl_flags     = TCTL_COLL_DIST_E1000,
        .eeprom_read    = e1000_read_eerd,
    }, {
        .vendor_id      = 0x8086,
        .device_id      = 0x10f5,
        .tctl_flags     = TCTL_COLL_DIST_E1000E | TCTL_RRTHRESH,
        .eeprom_read    = e1000e_read_flash,
    }
};

struct cdi_device* e1000_init_device(struct cdi_bus_data* bus_data)
{
    struct cdi_pci_device* pci = (struct cdi_pci_device*) bus_data;
    struct e1000_device* netcard;
    struct cdi_mem_area* buf;
    int i;

    for (i = 0; i < ARRAY_SIZE(models); i++) {
        if (pci->vendor_id == models[i].vendor_id
            && pci->device_id == models[i].device_id)
        {
            goto found;
        }
    }
    return NULL;

found:
    buf = cdi_mem_alloc(sizeof(*netcard),
        CDI_MEM_PHYS_CONTIGUOUS | CDI_MEM_DMA_4G | 2);
    if (buf == NULL) {
        return NULL;
    }

    netcard = buf->vaddr;
    memset(netcard, 0, sizeof(*netcard));

    netcard->model = &models[i];
    netcard->phys = buf->paddr.items[0].start;
    netcard->net.dev.bus_data = (struct cdi_bus_data*) pci;

    // PCI-bezogenes Zeug initialisieren
    netcard->revision = pci->rev_id;
    cdi_register_irq(pci->irq, e1000_handle_interrupt, &netcard->net.dev);
    cdi_pci_alloc_ioports(pci);

    cdi_list_t reslist = pci->resources;
    struct cdi_pci_resource* res;
    for (i = 0; (res = cdi_list_get(reslist, i)); i++) {
        if (res->type == CDI_PCI_MEMORY && res->index == 0) {
            struct cdi_mem_area* mmio = cdi_mem_map(res->start, res->length);
            netcard->mem_base = mmio->vaddr;
        } else if (res->type == CDI_PCI_MEMORY && res->index == 1) {
            struct cdi_mem_area* mmio = cdi_mem_map(res->start, res->length);
            netcard->flash_base = mmio->vaddr;
        }
    }

    // Karte initialisieren
    printf("e1000: IRQ %d, MMIO an %p  Revision:%d\n",
        pci->irq, netcard->mem_base, netcard->revision);

    printf("e1000: Fuehre Reset der Karte durch\n");
    reset_nic(netcard);

    printf("e1000: Warte auf Link\n");
    CDI_CONDITION_WAIT_SLEEP(reg_inl(netcard, REG_STATUS) & STATUS_LINK_UP,
                             3000, 10);
    if ((reg_inl(netcard, REG_STATUS) & STATUS_LINK_UP) == 0) {
        printf("e1000: Kein Link\n");
    }

    cdi_net_device_init(&netcard->net);

    // Interrupts aktivieren
    reg_outl(netcard, REG_INTR_MASK_CLR, 0xFFFF);
    reg_outl(netcard, REG_INTR_MASK, 0xFFFF);

    return &netcard->net.dev;
}

void e1000_remove_device(struct cdi_device* device)
{
}

/**
 * Die Uebertragung von Daten geschieht durch einen Ring von
 * Transmit-Deskriptoren, die jeweils ein zu uebertragendes Paket
 * beschreiben.
 *
 * Die Hardware kennt dabei zwei besondere Deskriptoren, die Head und
 * Tail heissen. Wenn der Treiber ein neues Paket zum Senden einstellt,
 * fuegt er einen neuen Deskriptor nach Tail ein und erhoeht Tail.
 *
 * Die Hardware erhoeht ihrerseits Head, wenn sie ein Paket abgeschickt hat.
 * Wenn Head = Tail ist, ist die Sendewarteschlange leer.
 */
void e1000_send_packet(struct cdi_net_device* device, void* data, size_t size)
{
    struct e1000_device* netcard = (struct e1000_device*) device;
    uint32_t cur, head;

#ifdef DEBUG
    printf("e1000: e1000_send_packet\n");
#endif

    // Aktuellen Deskriptor erhoehen
    cur = netcard->tx_cur_buffer;
    netcard->tx_cur_buffer++;
    netcard->tx_cur_buffer %= TX_BUFFER_NUM;

    // Head auslesen
    head = reg_inl(netcard, REG_TXDESC_HEAD);
    if (netcard->tx_cur_buffer == head) {
        printf("e1000: Kein Platz in der Sendewarteschlange!\n");
        netcard->tx_cur_buffer = cur;
        return;
    }

    // Buffer befuellen
    if (size > TX_BUFFER_SIZE) {
        size = TX_BUFFER_SIZE;
    }
    memcpy(netcard->tx_buffer + cur * TX_BUFFER_SIZE, data, size);

    // TX-Deskriptor setzen und Tail erhoehen
    netcard->tx_desc[cur].cmd = TX_CMD_EOP | TX_CMD_IFCS;
    netcard->tx_desc[cur].length = size;
    netcard->tx_desc[cur].buffer =
        PHYS(netcard, tx_buffer) + (cur * TX_BUFFER_SIZE);

#ifdef DEBUG
    printf("e1000: Setze Tail auf %d, Head = %d\n", netcard->tx_cur_buffer, head);
#endif
    reg_outl(netcard, REG_TXDESC_TAIL, netcard->tx_cur_buffer);
}

static void e1000_handle_interrupt(struct cdi_device* device)
{
    struct e1000_device* netcard = (struct e1000_device*) device;

    uint32_t icr = reg_inl(netcard, REG_INTR_CAUSE);

#ifdef DEBUG
    printf("e1000: Interrupt, ICR = %08x\n", icr);
#endif

    if (icr & ICR_RECEIVE) {

        uint32_t head = reg_inl(netcard, REG_RXDESC_HEAD);

        while (netcard->rx_cur_buffer != head) {

            size_t size = netcard->rx_desc[netcard->rx_cur_buffer].length;
            uint8_t status = netcard->rx_desc[netcard->rx_cur_buffer].status;

            // Wenn Descriptor Done nicht gesetzt ist, war die Hardware
            // noch nicht gant fertig mit Kopieren
            if ((status & 0x1) == 0) {
                break;
            }

            // 4 Bytes CRC von der Laenge abziehen
            size -= 4;

#ifdef DEBUG
            printf("e1000: %d Bytes empfangen (status = %x)\n", size, status);
/*
            int i;
            for (i = 0; i < (size < 49 ? size : 49); i++) {
                printf("%02hhx ", netcard->rx_buffer[
                    netcard->rx_cur_buffer * RX_BUFFER_SIZE + i]);
                if (i % 25 == 0) {
                    printf("\n");
                }
            }
            printf("\n\n");
*/
#endif

            cdi_net_receive(
                (struct cdi_net_device*) netcard,
                &netcard->rx_buffer[netcard->rx_cur_buffer * RX_BUFFER_SIZE],
                size);

            netcard->rx_cur_buffer++;
            netcard->rx_cur_buffer %= RX_BUFFER_NUM;
        }

        if (netcard->rx_cur_buffer == head) {
            reg_outl(netcard, REG_RXDESC_TAIL,
                (head + RX_BUFFER_NUM - 1) % RX_BUFFER_NUM);
        } else {
            reg_outl(netcard, REG_RXDESC_TAIL, netcard->rx_cur_buffer);
        }

    } else if (icr & ICR_TRANSMIT) {
        // Nichts zu tun
    } else if (icr & ICR_LINK_CHANGE) {
        printf("e1000: Link %s\n",
               (reg_inl(netcard, REG_STATUS) & STATUS_LINK_UP) ? "up" : "down");
    } else {
#ifdef DEBUG
        printf("e1000: Unerwarteter Interrupt.\n");
#endif
    }
}
