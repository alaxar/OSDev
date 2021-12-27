/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "rtl8139.h"
#include "util/util.h"
#include "timer.h"
#include "paging.h"
#include "video/console.h"
#include "kheap.h"

/// Information: The eeprom code for the RTL8139 (rtl8139_eeprom.h/c) has been deleted in rev. 1002. Check out this revision to access it.

#define RTL8139_RX_BUFFER_SIZE 65536 // 64 KiB
#define RTL8139_TX_BUFFER_SIZE 4096

static void rtl8139_receive(network_adapter_t* adapter);

void rtl8139_handler(registers_t* r, pciDev_t* device)
{
    network_adapter_t* adapter = device->data;
    if (!adapter || adapter->driver != &network_drivers[RTL8139])
    {
        return;
    }
    RTL8139_networkAdapter_t* rAdapter = adapter->data;

  #ifdef _NETWORK_DIAGNOSIS_
    textColor(HEADLINE);
    printf("\n--------------------------------------------------------------------------------");
  #endif

    // read bytes 003Eh bis 003Fh, Interrupt Status Register
    volatile uint16_t val = *((uint16_t*)(rAdapter->MMIO_base + RTL8139_INTRSTATUS));
  #ifdef _NETWORK_DIAGNOSIS_
    textColor(SUCCESS);
    if (val & RTL8139_INT_RX_OK)           { puts("Receive OK\n"); }
    if (val & RTL8139_INT_TX_OK)           { puts("Transmit OK\n"); }
    textColor(TEXT);
  #endif
    if (val & RTL8139_INT_RX_ERR)          { printfe("Receive Error\n"); }
    if (val & RTL8139_INT_TX_ERR)          { printfe("Transmit Error\n"); }
    if (val & RTL8139_INT_RXBUF_OVERFLOW)  { printfe("Rx Buffer Overflow\n");}
    if (val & RTL8139_INT_RXFIFO_UNDERRUN) { printfe("Packet Underrun / Link change\n");}
    if (val & RTL8139_INT_RXFIFO_OVERFLOW) { printfe("Rx FIFO Overflow\n");}
    if (val & RTL8139_INT_CABLE)           { printfe("Cable Length Change\n");}
    if (val & RTL8139_INT_TIMEOUT)         { printfe("Time Out\n");}
    if (val & RTL8139_INT_PCIERR)          { printfe("PCI Bus Error\n");}

    // reset interrupts by writing 1 to the bits of offset 003Eh to 003Fh, Interrupt Status Register
    *((uint16_t*)(rAdapter->MMIO_base + RTL8139_INTRSTATUS)) = val;

    if (val & RTL8139_INT_RX_OK)
    {
        rtl8139_receive(adapter);
    }
}

void rtl8139_install(network_adapter_t* adapter)
{
    RTL8139_networkAdapter_t* rAdapter = malloc(sizeof(RTL8139_networkAdapter_t), 0, "RTL8139");
    rAdapter->device                   = adapter;
    adapter->data                      = rAdapter;

    rAdapter->RxBuffer                 = malloc(RTL8139_RX_BUFFER_SIZE + RTL8139_TX_BUFFER_SIZE, 4 | HEAP_CONTINUOUS, "RTL8139 Buffers");
    rAdapter->RxBufferPointer          = 0;
    memset(rAdapter->RxBuffer, 0, RTL8139_RX_BUFFER_SIZE); // clear receiving buffer

    rAdapter->TxBuffer                 = rAdapter->RxBuffer + RTL8139_RX_BUFFER_SIZE;
    rAdapter->TxBufferPhys             = paging_getPhysAddr(rAdapter->TxBuffer);
    rAdapter->TxBufferIndex            = 0;

    // Detect MMIO space, configure PCI
    pciDev_t* device = adapter->PCIdev;
    uint16_t pciCommandRegister = pci_configRead(device, PCI_COMMAND, 2);
    pci_configWrite_word(device, PCI_COMMAND, pciCommandRegister | PCI_CMD_MMIO | PCI_CMD_BUSMASTER | PCI_CMD_FASTBACKTOBACKENABLE); // resets status register, sets command register
    rAdapter->MMIO_base = 0;
    for (uint8_t i = 0; i < 6 && !rAdapter->MMIO_base; i++)
    {
        rAdapter->MMIO_base = pci_aquireMemoryForMMIO(device->bar + i);
    }

    /*http://wiki.osdev.org/RTL8139

    Turning on the RTL8139:
    Send 0x00 to the CONFIG_1 register (0x52) to set the LWAKE + LWPTN to active high.
    this should essentially "power on" the device.

    Software Reset:
    Sending 0x10 to the Command register (0x37) will send the RTL8139 into a software reset.
    Once that byte is sent, the RST bit must be checked to make sure that the chip has finished the reset.
    If the RST bit is 1, then the reset is still in operation.

    Init Receive Buffer:
    Send the chip a memory location to use as its receive buffer start location.
    One way to do it, would be to define a buffer variable
    and send that variables memory location to the RBSTART register (0x30).*/

  #ifdef _NETWORK_DIAGNOSIS_
    printf("\nMMIO base mapped to virt. addr. %Xh", rAdapter->MMIO_base);
  #endif

    // "power on" the card
    *((volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_CONFIG1)) = 0x00;

    // carry out reset of network card: set bit 4 at offset 0x37 (1 Byte)
    *((volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_CHIPCMD)) = RTL8139_CMD_RESET;

    // wait for the reset of the "reset flag"
    WAIT_FOR_CONDITION(!(*((volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_CHIPCMD)) & RTL8139_CMD_RESET), 200, 10, "\nReset flag could not be cleared! Finished by timeout.\n");

    uint32_t versionID = *((uint32_t*)(rAdapter->MMIO_base + RTL8139_TXCONFIG));
    versionID &= (BIT(30) | BIT(29) | BIT(28) | BIT(27) | BIT(26) | BIT(23) | BIT(22));

    if      (versionID == (versionID & (BIT(30) | BIT(29)                                                  ))) rAdapter->version = 0; // RTL8139
    else if (versionID == (versionID & (BIT(30) | BIT(29) | BIT(28)                                        ))) rAdapter->version = 1; // RTL8139A
    else if (versionID == (versionID & (BIT(30) | BIT(29) | BIT(28) | BIT(27)                              ))) rAdapter->version = 2; // RTL8139B
    else if (versionID == (versionID & (BIT(30) | BIT(29) | BIT(28)           | BIT(26)                    ))) rAdapter->version = 3; // RTL8139C
    else if (versionID == (versionID & (BIT(30) | BIT(29) | BIT(28) | BIT(27)           | BIT(23)          ))) rAdapter->version = 5; // RTL8100
    else if (versionID == (versionID & (BIT(30) | BIT(29) | BIT(28)           | BIT(26)           | BIT(22)))) rAdapter->version = 4; // RTL8139D
    else if (versionID == (versionID & (BIT(30) | BIT(29) | BIT(28)           | BIT(26) | BIT(23)          ))) rAdapter->version = 6; // RTL8139C+
    else if (versionID == (versionID & (BIT(30) | BIT(29) | BIT(28)           | BIT(26) | BIT(23) | BIT(22)))) rAdapter->version = 7; // RTL8101
    else                                                                                                       rAdapter->version = 8; // Unknown

    if (rAdapter->version < 8)
    {
        static const char* const rtlVersions[] =
        {
            "RTL8139", "RTL8139A", "RTL8139B", "RTL8139C", "RTL8139D", "RTL8100", "RTL8139C+", "RTL8101"
        };
        printf("\n%s\n", rtlVersions[rAdapter->version]);
    }
    else
    {
        printf("\nRTL8139 subversion unknown: %X\n", versionID);
    }

    // now we set the RE and TE bits from the "Command Register" to Enable Receiving and Transmission
    // activate transmitter and receiver: Set bit 2 (TE) and 3 (RE) in control register 0x37 (1 byte).
    *((volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_CHIPCMD)) = RTL8139_CMD_TX_ENABLE | RTL8139_CMD_RX_ENABLE;

    // set TCR (transmit configuration register, 0x40, 4 byte)
    *((volatile uint32_t*)(rAdapter->MMIO_base + RTL8139_TXCONFIG)) = 0x03000700; // TCR

    // set RCR (receive configuration register, RTL8139_RXCONFIG, 4 byte)
    *((volatile uint32_t*)(rAdapter->MMIO_base + RTL8139_RXCONFIG)) = RTL8139_RCR_ACCEPT_PHYSICAL_MATCH | RTL8139_RCR_ACCEPT_MULTICAST |
                                                                      RTL8139_RCR_ACCEPT_BROADCAST      | RTL8139_RCR_ACCEPT_RUNT      |
                                                                      RTL8139_RCR_ACCEPT_ERROR_PACKET   | RTL8139_RCR_DMA_BURST_1024   |
                                                                      RTL8139_RCR_BUFFERLEN_64K         | RTL8139_RCR_ERTH_1_16 ;

    // physical address of the receive buffer has to be written to RBSTART (0x30, 4 byte)
    *((volatile uint32_t*)(rAdapter->MMIO_base + RTL8139_RXBUF)) = paging_getPhysAddr(rAdapter->RxBuffer);

    // set interrupt mask
    *((volatile uint16_t*)(rAdapter->MMIO_base + RTL8139_INTRMASK)) = 0xFFFF; // all interrupts

    for (uint8_t i = 0; i < 6; i++)
    {
        adapter->MAC[i] = *(volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_IDR0 + i);
    }
}

static void rtl8139_resetReceiver(RTL8139_networkAdapter_t* rAdapter)
{
    *((volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_CHIPCMD)) = RTL8139_CMD_RX_ENABLE;

    *((volatile uint32_t*)(rAdapter->MMIO_base + RTL8139_RXCONFIG)) = RTL8139_RCR_ACCEPT_PHYSICAL_MATCH | RTL8139_RCR_ACCEPT_MULTICAST |
                                                                      RTL8139_RCR_ACCEPT_BROADCAST      | RTL8139_RCR_ACCEPT_RUNT      |
                                                                      RTL8139_RCR_ACCEPT_ERROR_PACKET   | RTL8139_RCR_DMA_BURST_1024   |
                                                                      RTL8139_RCR_BUFFERLEN_64K         | RTL8139_RCR_ERTH_1_16 ;

    *((volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_CHIPCMD)) = RTL8139_CMD_TX_ENABLE | RTL8139_CMD_RX_ENABLE;
    rAdapter->RxBufferPointer = 0;
    *((volatile uint16_t*)(rAdapter->MMIO_base + RTL8139_RXBUFTAIL)) = rAdapter->RxBufferPointer - 16;
}

static bool rtl8139_isRxBufEmpty(RTL8139_networkAdapter_t* rAdapter)
{
    return (*((volatile uint8_t*)(rAdapter->MMIO_base + RTL8139_CHIPCMD)) & BIT(0));
}

static void rtl8139_receive(network_adapter_t* adapter)
{
    RTL8139_networkAdapter_t* rAdapter = adapter->data;
    while (!rtl8139_isRxBufEmpty(rAdapter))
    {
        uint32_t status = *(uint32_t*)(rAdapter->RxBuffer + rAdapter->RxBufferPointer);
        uint16_t length = status >> 16;

      #ifdef _NETWORK_DATA_
        // Display RTL8139 specific packet data
        printf("\nRx Status: %xh, packet size: %u", (uint16_t)status, length);
      #endif

        if (!(status & RTL8139_RX_STATUS_OK) || (status & (RTL8139_RX_BAD_SYMBOL | RTL8139_RX_RUNT | RTL8139_RX_TOO_LONG | RTL8139_RX_CRC_ERR | RTL8139_RX_FRAME_ALIGN)) || length < 4)
        {
            printfe("\nRTL8139 Rx error: status=%xh, length=%u", status, length);
            rtl8139_resetReceiver(rAdapter);
            return;
        }

        // Parse packet
        size_t packet_length = length - 4; // Strip CRC from packet
        if (rAdapter->RxBufferPointer + length + 4 > RTL8139_RX_BUFFER_SIZE) // handle wrap-around
        {
            uint8_t temp[packet_length]; // Packet has to be moved to a temporary buffer
            size_t firstPartLength = RTL8139_RX_BUFFER_SIZE - rAdapter->RxBufferPointer - 4;
            memcpy(temp, &rAdapter->RxBuffer[rAdapter->RxBufferPointer] + 4, firstPartLength);
            memcpy(temp + firstPartLength, rAdapter->RxBuffer, packet_length - firstPartLength);
            network_receivedPacket(adapter, temp, packet_length); // Inform network interface about the packet
        }
        else
            network_receivedPacket(adapter, &rAdapter->RxBuffer[rAdapter->RxBufferPointer] + 4, packet_length); // Inform network interface about the packet

        // Increase RxBufferPointer
        rAdapter->RxBufferPointer += (length + 4);
        rAdapter->RxBufferPointer  = alignUp(rAdapter->RxBufferPointer, 4); // packets are DWORD aligned
        rAdapter->RxBufferPointer %= RTL8139_RX_BUFFER_SIZE;                // handle wrap-around

        // set read pointer
        *((volatile uint16_t*)(rAdapter->MMIO_base + RTL8139_RXBUFTAIL)) = rAdapter->RxBufferPointer - 16;

      #ifdef _NETWORK_DIAGNOSIS_
        printf("RXBUFTAIL: %u", *((uint16_t*)(rAdapter->MMIO_base + RTL8139_RXBUFTAIL)));
      #endif
    }
}

/*
The process of transmitting a packet with RTL8139:
1: copy the packet to a physically continuous buffer in memory.
2: Write the descriptor which is functioning
  (1). Fill in start address (physical) of this buffer.
  (2). Fill in Transmit Status: the size of this packet, the early transmit threshold, clear OWN bit in TSD (this starts the PCI operation).
3: As the number of data moved to FIFO meet early transmit threshold, the chip start to move data from FIFO to line..
4: When the whole packet is moved to FIFO, the OWN bit is set to 1.
5: When the whole packet is moved to line, the TOK(in TSD) is set to 1.
6: If TOK(IMR) is set to 1 and TOK(ISR) is set then a interrupt is triggered.
7: Interrupt service routine called, driver should clear TOK(ISR) State Diagram: (TOK,OWN)
*/
bool rtl8139_send(network_adapter_t* adapter, const uint8_t* data, size_t length, uint8_t offloading)
{
    if (length > RTL8139_TX_BUFFER_SIZE)
        return false; // Splitting packets not yet supported

    RTL8139_networkAdapter_t* rAdapter = adapter->data;

    WAIT_FOR_CONDITION(*(volatile uint32_t*)(rAdapter->MMIO_base + RTL8139_TXSTATUS0 + 4 * rAdapter->TxBufferIndex) & BIT(13), 20, 20, "RTL8139: Error - previous packet not yet transmitted");

    memcpy(rAdapter->TxBuffer, data, length); // tx buffer
    if (length < 60) // Fill buffer to a minimal length of 60
    {
        memset(rAdapter->TxBuffer+length, 0, 60-length);
        length = 60;
    }

  #ifdef _NETWORK_DIAGNOSIS_
    printf("\n\n>>> Transmission starts <<<\nPhysical Address of Tx Buffer = %Xh\n", rAdapter->TxBufferPhys);
  #endif

    // set address and size of the Tx buffer
    // reset OWN bit in TASD (REG_TRANSMIT_STATUS) starting transmit
    // set transmit FIFO threshold to 48*32 = 1536 bytes to avoid tx underrun
    *((volatile uint32_t*)(rAdapter->MMIO_base + RTL8139_TXADDR0   + 4 * rAdapter->TxBufferIndex)) = rAdapter->TxBufferPhys;
    *((volatile uint32_t*)(rAdapter->MMIO_base + RTL8139_TXSTATUS0 + 4 * rAdapter->TxBufferIndex)) = length | (48 << 16);

    rAdapter->TxBufferIndex++;
    rAdapter->TxBufferIndex %= 4;
  #ifdef _NETWORK_DIAGNOSIS_
    textColor(LIGHT_BLUE);
    printf("\n>> Packet sent. <<");
  #endif
    return true;
}

/*
* Copyright (c) 2009-2015 The PrettyOS Project. All rights reserved.
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
