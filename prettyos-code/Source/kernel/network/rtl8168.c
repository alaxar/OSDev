/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "rtl8168.h"
#include "util/util.h"
#include "timer.h"
#include "paging.h"
#include "kheap.h"
#include "video/console.h"


static void setupDescriptors(RTL8168_networkAdapter_t* rAdapter)
{
    rAdapter->Rx_Descriptors = malloc(RTL8168_DESCRIPTORS*sizeof(RTL8168_Desc)*2, 256 | HEAP_CONTINUOUS, "RTL8168 RxTxDesc");
    rAdapter->Tx_Descriptors = rAdapter->Rx_Descriptors + RTL8168_DESCRIPTORS;
    rAdapter->RxBuffers = malloc(RTL8168_BUFFER_LENGTH * RTL8168_DESCRIPTORS * 2, 8 | HEAP_CONTINUOUS, "RTL8168 Buffers");
    rAdapter->TxBuffers = rAdapter->RxBuffers + RTL8168_BUFFER_LENGTH * RTL8168_DESCRIPTORS;
    for (uint16_t i = 0; i < RTL8168_DESCRIPTORS; i++)
    {
        if (i == (RTL8168_DESCRIPTORS - 1)) // Last descriptor? if so, set the EOR bit
        {
            rAdapter->Rx_Descriptors[i].command = (RTL8168_OWN | RTL8168_EOR | (RTL8168_BUFFER_LENGTH & 0x3FFF));
            rAdapter->Tx_Descriptors[i].command = RTL8168_EOR;
        }
        else
        {
            rAdapter->Rx_Descriptors[i].command = (RTL8168_OWN | (RTL8168_BUFFER_LENGTH & 0x3FFF));
            rAdapter->Tx_Descriptors[i].command = 0;
        }
        rAdapter->Rx_Descriptors[i].vlan = 0;
        rAdapter->Tx_Descriptors[i].vlan = 0;
        rAdapter->Rx_Descriptors[i].low_buf = paging_getPhysAddr(rAdapter->RxBuffers + i*RTL8168_BUFFER_LENGTH);
        rAdapter->Tx_Descriptors[i].low_buf = paging_getPhysAddr(rAdapter->TxBuffers + i*RTL8168_BUFFER_LENGTH);
        rAdapter->Rx_Descriptors[i].high_buf = 0;
        rAdapter->Tx_Descriptors[i].high_buf = 0;
    }
  #ifdef _NETWORK_DIAGNOSIS_
    printf("\nDescriptors are set up.");
  #endif
}

void rtl8168_install(network_adapter_t* adapter)
{
    RTL8168_networkAdapter_t* rAdapter = malloc(sizeof(RTL8168_networkAdapter_t), 0, "RTL8168");
    adapter->data = rAdapter;
    rAdapter->device = adapter;
    rAdapter->TxIndex = 0;

    adapter->features = OFFLOAD_IPv4 | OFFLOAD_TCP | OFFLOAD_UDP; // RTL8168 supports checksum offloading for IPv4, TCP and UDP. Some cards also support IPv6, but yet we do not know which.

    // Detect MMIO space, configure PCI
    pciDev_t* device = adapter->PCIdev;
    uint16_t pciCommandRegister = pci_configRead(device, PCI_COMMAND, 2);
    pci_configWrite_word(device, PCI_COMMAND, pciCommandRegister | PCI_CMD_MMIO | PCI_CMD_BUSMASTER | PCI_CMD_FASTBACKTOBACKENABLE | PCI_CMD_MEMORYWRITEINVALIDATEENABLE); // resets status register, sets command register
    rAdapter->MMIO_base = 0;
    for (uint8_t i = 0; i < 6 && !rAdapter->MMIO_base; i++)
    {
        rAdapter->MMIO_base = pci_aquireMemoryForMMIO(device->bar + i);
    }
  #ifdef _NETWORK_DIAGNOSIS_
    printf("\nMMIO_base (phys, virt): %Xh, %Xh", paging_getPhysAddr(rAdapter->MMIO_base), rAdapter->MMIO_base);
  #endif

    *((volatile uint16_t*)(rAdapter->MMIO_base + RTL8168_CPCMD)) = BIT(3); // Enable PCI multiple read/write. This register should be accessed before any other register.

    // Reset card
    *((volatile uint8_t*)(rAdapter->MMIO_base + RTL8168_CHIPCMD)) = RTL8168_CMD_RESET;

    WAIT_FOR_CONDITION(!(*((volatile uint8_t*)(rAdapter->MMIO_base + RTL8168_CHIPCMD)) & RTL8168_CMD_RESET), 20, 5, "\nWaiting not successful! Finished by timeout.\n");

    // Get MAC
    for (uint8_t i = 0; i < 6; i++)
    {
        adapter->MAC[i] = *(volatile uint8_t*)(rAdapter->MMIO_base + RTL8168_IDR0 + i);
    }

    setupDescriptors(rAdapter);

    *(volatile uint32_t*)(rAdapter->MMIO_base + RTL8168_RXCONFIG) = 0x0000E70F; // RxConfig = RXFTH: no, MXDMA: unlimited, AAP: set (promisc. mode set)
    *(volatile uint8_t*)(rAdapter->MMIO_base + RTL8168_CHIPCMD) = RTL8168_CMD_TX_ENABLE; // Enable Tx in the command register before writing TxConfig
    *(volatile uint32_t*)(rAdapter->MMIO_base + RTL8168_TXCONFIG) = 0x03000700; // TxConfig = IFG: normal, MXDMA: unlimited
    *(volatile uint16_t*)(rAdapter->MMIO_base + RTL8168_RPS) = RTL8168_BUFFER_LENGTH; // Max rx packet size: Use buffer size as we currently do not support packets splitted into several buffers
    *(volatile uint8_t*)(rAdapter->MMIO_base + RTL8168_ETTHR) = 0x3F; // Early transmit threshold: no early transmit

    *(volatile uint32_t*)(rAdapter->MMIO_base + RTL8168_TXDESC) = paging_getPhysAddr(rAdapter->Tx_Descriptors); // Tell the NIC where the first Tx descriptor is
    *(volatile uint32_t*)(rAdapter->MMIO_base + RTL8168_RXDESC) = paging_getPhysAddr(rAdapter->Rx_Descriptors); // Tell the NIC where the first Rx descriptor is

    *(volatile uint16_t*)(rAdapter->MMIO_base + RTL8168_INTRMASK) = BIT(14) | BIT(8) | BIT(7) | BIT(5) | BIT(4) | BIT(3) | BIT(1) | BIT(0); // Enable all but FEmp, FOVW, TOK interrupts

    *(volatile uint8_t*)(rAdapter->MMIO_base + RTL8168_CHIPCMD) = RTL8168_CMD_RX_ENABLE | RTL8168_CMD_TX_ENABLE; // Enable Rx/Tx in the Command register

    // Configure MSI
    if (pci_trySetMSIVector(adapter->PCIdev, 3))
    {
        pci_switchToMSI(adapter->PCIdev);
    }

  #ifdef _NETWORK_DIAGNOSIS_
    printf("\nRTL8168 configured");
  #endif
}

bool rtl8168_send(network_adapter_t* adapter, const uint8_t* data, size_t length, uint8_t offloading)
{
    if (length > RTL8168_BUFFER_LENGTH)
        return false; // Splitting packets not yet supported

    RTL8168_networkAdapter_t* rAdapter = adapter->data;

    memcpy(rAdapter->TxBuffers + rAdapter->TxIndex*RTL8168_BUFFER_LENGTH, data, length); // Copy data to TxBuffer

  #ifdef _NETWORK_DIAGNOSIS_
    printf("\n\n>>> Transmission starts <<<\nIndex of Tx = %u\nOffloading: ", rAdapter->TxIndex);
    if (offloading & OFFLOAD_IPv4)
        puts("IPv4");
    if (offloading & OFFLOAD_UDP)
        puts(", UDP.\n");
    if (offloading & OFFLOAD_TCP)
        puts(", TCP.\n");
  #endif
    uint32_t flags = RTL8168_OWN | (rAdapter->Tx_Descriptors[rAdapter->TxIndex].command & RTL8168_EOR) | (length & 0x3FFF) | BIT(29) | BIT(28);
    if (offloading & OFFLOAD_IPv4)
        flags |= BIT(18);
    if (offloading & OFFLOAD_UDP)
        flags |= BIT(17);
    if (offloading & OFFLOAD_TCP)
        flags |= BIT(16);
    rAdapter->Tx_Descriptors[rAdapter->TxIndex].command = flags;
    rAdapter->Tx_Descriptors[rAdapter->TxIndex].vlan = 0;
    *(volatile uint8_t*)(rAdapter->MMIO_base + RTL8168_TPPOLL) = BIT(6); // Tell the NIC that a packet is waiting

    rAdapter->TxIndex++;
    rAdapter->TxIndex %= RTL8168_DESCRIPTORS;
  #ifdef _NETWORK_DIAGNOSIS_
    textColor(LIGHT_BLUE);
    printf("\n>>> Packet sent. <<<");
    textColor(TEXT);
  #endif

    return true;
}

static void rtl8168_received(network_adapter_t* adapter)
{
    RTL8168_networkAdapter_t* rAdapter = adapter->data;

    for (uint16_t i = 0; i < RTL8168_DESCRIPTORS; i++)
    {
        if (!(rAdapter->Rx_Descriptors[i].command & RTL8168_OWN)) // Buffer contains received data
        {
          #ifdef _NETWORK_DIAGNOSIS_
            textColor(LIGHT_BLUE);
            printf("\n>>> Packet received. <<<");
            textColor(TEXT);
          #endif
            // Read data
            size_t length = rAdapter->Rx_Descriptors[i].command & 0x3FFF;
            if (length > 4)
                network_receivedPacket(adapter, rAdapter->RxBuffers + i*RTL8168_BUFFER_LENGTH, length-4); // Strip CRC from packet.

            // Reset descriptor
            if (i == (RTL8168_DESCRIPTORS - 1)) // Last descriptor? if so, set the EOR bit
                rAdapter->Rx_Descriptors[i].command = (RTL8168_OWN | RTL8168_EOR | (RTL8168_BUFFER_LENGTH & 0x3FFF));
            else
                rAdapter->Rx_Descriptors[i].command = (RTL8168_OWN | (RTL8168_BUFFER_LENGTH & 0x3FFF));
            rAdapter->Rx_Descriptors[i].vlan = 0;
        }
    }
}


void rtl8168_handler(registers_t* r, pciDev_t* device)
{
    network_adapter_t* adapter = device->data;
    if (!adapter || adapter->driver != &network_drivers[RTL8168])
        return;

    RTL8168_networkAdapter_t* rAdapter = adapter->data;
    uint16_t intStatus = *(volatile uint16_t*)(rAdapter->MMIO_base + RTL8168_INTRSTATUS);

  #ifdef _NETWORK_DIAGNOSIS_
    printf("\nRTL8168 - IRQ Status: %xh", intStatus);
  #endif

    if (intStatus & (BIT(15) | BIT(14) | BIT(7) | BIT(6) | BIT(4) | BIT(3) | BIT(1))) // Error
    {
      #ifndef _NETWORK_DIAGNOSIS_
        puts("\nRTL8168: ");
      #endif
        textColor(ERROR);
        if (intStatus & BIT(15))
            puts("System Error\t");
        if (intStatus & BIT(14))
            puts("Timeout\t");
        if (intStatus & BIT(7))
            puts("Tx Descriptor Unavailable\t");
        if (intStatus & BIT(6))
            puts("Rx FIFO Overflow\t");
        if (intStatus & BIT(4))
            puts("Rx Descriptor Unavailable\t");
        if (intStatus & BIT(3))
            puts("Transmit Error\t");
        if (intStatus & BIT(1))
            puts("Receive Error\t");
        textColor(TEXT);
    }
#ifdef _NETWORK_DIAGNOSIS_
    textColor(SUCCESS);
    if (intStatus & BIT(0))
        puts("Receive OK\t");
    if (intStatus & BIT(2))
        puts("Transmit OK\t");
    textColor(DATA);
    if (intStatus & BIT(5))
        puts("Link Change\t");
    if (intStatus & BIT(8))
        puts("Software Interrupt");
    textColor(TEXT);
#endif

    *(volatile uint16_t*)(rAdapter->MMIO_base + RTL8168_INTRSTATUS) = intStatus;

    if (intStatus & BIT(0)) // Rx OK - handle received data
        rtl8168_received(adapter);
}

/*
* Copyright (c) 2011-2017 The PrettyOS Project. All rights reserved.
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
