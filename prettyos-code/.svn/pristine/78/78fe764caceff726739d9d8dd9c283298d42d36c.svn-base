#ifndef RTL8168_H
#define RTL8168_H

#include "network.h"


// RTL8168 register definitions
#define RTL8168_IDR0                0x00        // Mac address
#define RTL8168_TXDESC              0x20        // Tx descriptors (32bit)
#define RTL8168_RXDESC              0xE4        // Tx descriptors (32bit)
#define RTL8168_CHIPCMD             0x37        // Command register
#define RTL8168_TPPOLL              0x38        // Transmit Priority Polling
#define RTL8168_INTRMASK            0x3C        // Interrupt mask
#define RTL8168_INTRSTATUS          0x3E        // Interrupt status
#define RTL8168_TXCONFIG            0x40        // Tx config
#define RTL8168_RXCONFIG            0x44        // Rx config
#define RTL8168_CFG9346             0x50        // 93C46 command register
#define RTL8168_RPS                 0xDA        // Max Receive Packet Size
#define RTL8168_CPCMD               0xE0        // C+ Command
#define RTL8168_ETTHR               0xEC        // Early transmit threshold

// RTL8168 command bits
#define RTL8168_CMD_RESET           0x10
#define RTL8168_CMD_RX_ENABLE       0x08
#define RTL8168_CMD_TX_ENABLE       0x04

// RTL8168 interrupt status bits
#define RTL8168_INT_TIMEOUT         0x4000
#define RTL8168_INT_RX_FIFO_EMPTY   0x0200
#define RTL8168_INT_SOFTWARE_INT    0x0100
#define RTL8168_INT_TXDESC_UNAVAIL  0x0080
#define RTL8168_INT_RXFIFO_OVERFLOW 0x0040
#define RTL8168_INT_LINK_CHANGE     0x0020
#define RTL8168_INT_RXDESC_UNAVAIL  0x0010
#define RTL8168_INT_TX_ERR          0x0008
#define RTL8168_INT_TX_OK           0x0004
#define RTL8168_INT_RX_ERR          0x0002
#define RTL8168_INT_RX_OK           0x0001

#define RTL8168_OWN                 0x80000000
#define RTL8168_EOR                 0x40000000


#define RTL8168_DESCRIPTORS         32
#define RTL8168_BUFFER_LENGTH       2048


typedef struct
{
    uint32_t command;  // command/status dword
    uint32_t vlan;     // currently unused
    uint32_t low_buf;  // low 32-bits of physical buffer address
    uint32_t high_buf; // high 32-bits of physical buffer address
} __attribute__((packed)) RTL8168_Desc;

typedef struct
{
    network_adapter_t* device;
    void*              MMIO_base;
    RTL8168_Desc*      Rx_Descriptors;
    RTL8168_Desc*      Tx_Descriptors;
    uint8_t*           RxBuffers;
    uint8_t*           TxBuffers;
    uint16_t           TxIndex;
} RTL8168_networkAdapter_t;


void rtl8168_install(network_adapter_t* adapter);
bool rtl8168_send(network_adapter_t* adapter, const uint8_t* data, size_t length, uint8_t offloading);
void rtl8168_handler(registers_t* r, pciDev_t* device);


#endif
