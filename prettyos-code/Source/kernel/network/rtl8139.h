#ifndef RTL8139_H
#define RTL8139_H

#include "network.h"


// RTL8139C register definitions
#define RTL8139_IDR0                0x00        // Mac address
#define RTL8139_MAR0                0x08        // Multicast filter
#define RTL8139_TXSTATUS0           0x10        // Transmit status (4 32bit regs)
#define RTL8139_TXADDR0             0x20        // Tx descriptors (also 4 32bit)
#define RTL8139_RXBUF               0x30        // Receive buffer start address
#define RTL8139_RXEARLYCNT          0x34        // Early Rx byte count
#define RTL8139_RXEARLYSTATUS       0x36        // Early Rx status
#define RTL8139_CHIPCMD             0x37        // Command register
#define RTL8139_RXBUFTAIL           0x38        // Current address of packet read (queue tail)
#define RTL8139_RXBUFHEAD           0x3A        // Current buffer address (queue head)
#define RTL8139_INTRMASK            0x3C        // Interrupt mask
#define RTL8139_INTRSTATUS          0x3E        // Interrupt status
#define RTL8139_TXCONFIG            0x40        // Tx config
#define RTL8139_RXCONFIG            0x44        // Rx config
#define RTL8139_TIMER               0x48        // A general purpose counter
#define RTL8139_RXMISSED            0x4C        // 24 bits valid, write clears
#define RTL8139_CFG9346             0x50        // 93C46 command register
#define RTL8139_CONFIG0             0x51        // Configuration reg 0
#define RTL8139_CONFIG1             0x52        // Configuration reg 1
#define RTL8139_TIMERINT            0x54        // Timer interrupt register (32 bits)
#define RTL8139_MEDIASTATUS         0x58        // Media status register
#define RTL8139_CONFIG3             0x59        // Config register 3
#define RTL8139_CONFIG4             0x5A        // Config register 4
#define RTL8139_MULTIINTR           0x5C        // Multiple interrupt select
#define RTL8139_MII_TSAD            0x60        // Transmit status of all descriptors (16 bits)
#define RTL8139_MII_BMCR            0x62        // Basic Mode Control Register (16 bits)
#define RTL8139_MII_BMSR            0x64        // Basic Mode Status Register (16 bits)
#define RTL8139_AS_ADVERT           0x66        // Auto-negotiation advertisement reg (16 bits)
#define RTL8139_AS_LPAR             0x68        // Auto-negotiation link partner reg (16 bits)
#define RTL8139_AS_EXPANSION        0x6A        // Auto-negotiation expansion reg (16 bits)

// RTL8139C command bits
#define RTL8139_CMD_RESET           0x10
#define RTL8139_CMD_RX_ENABLE       0x08
#define RTL8139_CMD_TX_ENABLE       0x04
#define RTL8139_CMD_RX_BUF_EMPTY    0x01

// RTL8139C interrupt status bits
#define RTL8139_INT_PCIERR          0x8000        // PCI Bus error
#define RTL8139_INT_TIMEOUT         0x4000        // Set when TCTR reaches TimerInt value
#define RTL8139_INT_CABLE           0x2000        // Set when Cable Length Change
#define RTL8139_INT_RXFIFO_OVERFLOW 0x0040        // Rx FIFO overflow
#define RTL8139_INT_RXFIFO_UNDERRUN 0x0020        // Packet underrun / link change
#define RTL8139_INT_RXBUF_OVERFLOW  0x0010        // Rx BUFFER overflow
#define RTL8139_INT_TX_ERR          0x0008
#define RTL8139_INT_TX_OK           0x0004
#define RTL8139_INT_RX_ERR          0x0002
#define RTL8139_INT_RX_OK           0x0001

// RTL8139C transmit status bits
#define RTL8139_TX_CARRIER_LOST     0x80000000    // Carrier sense lost
#define RTL8139_TX_ABORTED          0x40000000    // Transmission aborted
#define RTL8139_TX_OUT_OF_WINDOW    0x20000000    // Out of window collision
#define RTL8139_TX_STATUS_OK        0x00008000    // Status ok: a good packet was transmitted
#define RTL8139_TX_UNDERRUN         0x00004000    // Transmit FIFO underrun
#define RTL8139_TX_HOST_OWNS        0x00002000    // Set to 1 when DMA operation is completed
#define RTL8139_TX_SIZE_MASK        0x00001FFF    // Descriptor size mask

// RTL8139C receive status bits
#define RTL8139_RX_MULTICAST        0x00008000    // Multicast packet
#define RTL8139_RX_PAM              0x00004000    // Physical address matched
#define RTL8139_RX_BROADCAST        0x00002000    // Broadcast address matched
#define RTL8139_RX_BAD_SYMBOL       0x00000020    // Invalid symbol in 100TX packet
#define RTL8139_RX_RUNT             0x00000010    // Packet size is <64 bytes
#define RTL8139_RX_TOO_LONG         0x00000008    // Packet size is >4K bytes
#define RTL8139_RX_CRC_ERR          0x00000004    // CRC error
#define RTL8139_RX_FRAME_ALIGN      0x00000002    // Frame alignment error
#define RTL8139_RX_STATUS_OK        0x00000001    // Status ok: a good packet was received

// RTL8139C receive config register bits (offset 0x44 - 0x47)
#define RTL8139_RCR_ACCEPT_ALL_PACKAGES    0x00000001
#define RTL8139_RCR_ACCEPT_PHYSICAL_MATCH  0x00000002
#define RTL8139_RCR_ACCEPT_MULTICAST       0x00000004
#define RTL8139_RCR_ACCEPT_BROADCAST       0x00000008
#define RTL8139_RCR_ACCEPT_RUNT            0x00000010
#define RTL8139_RCR_ACCEPT_ERROR_PACKET    0x00000020
#define RTL8139_RCR_WRAP                   0x00000080
#define RTL8139_RCR_DMA_BURST_1024         0x00000600 // bit 10,9
#define RTL8139_RCR_MAX_DMA_BURST          0x00000700 // bit 10,9,8
#define RTL8139_RCR_BUFFERLEN_8K           0x00000000
#define RTL8139_RCR_BUFFERLEN_16K          0x00000800
#define RTL8139_RCR_BUFFERLEN_32K          0x00001000
#define RTL8139_RCR_BUFFERLEN_64K          0x00001800
#define RTL8139_RCR_NO_RX_THRESHOLD        0x00380000
#define RTL8139_RCR_ERTH_1_16              0x01000000


typedef struct
{
    network_adapter_t* device;
    void*     MMIO_base;
    uint8_t*  RxBuffer;
    uint32_t  RxBufferPointer;
    uint8_t*  TxBuffer;
    uintptr_t TxBufferPhys;
    uint8_t   TxBufferIndex;
    uint8_t   version;
} RTL8139_networkAdapter_t;


// functions
void rtl8139_install(network_adapter_t* adapter);
bool rtl8139_send(network_adapter_t* adapter, const uint8_t* data, size_t length, uint8_t offloading);
void rtl8139_handler(registers_t* r, pciDev_t* device);


#endif
