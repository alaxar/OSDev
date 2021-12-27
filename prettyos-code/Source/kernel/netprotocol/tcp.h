#ifndef TCP_H
#define TCP_H

#include "network/network.h"
#include "tasking/task.h"

// http://tools.ietf.org/html/rfc793
// http://www.medianet.kent.edu/techreports/TR2005-07-22-tcp-EFSM.pdf

typedef enum {CLOSED, LISTEN, SYN_SENT, SYN_RECEIVED, ESTABLISHED, FIN_WAIT_1, FIN_WAIT_2, CLOSING, CLOSE_WAIT, LAST_ACK, TIME_WAIT, TCP_ANY} TCP_state;
typedef enum {SYN_FLAG, SYN_ACK_FLAG, ACK_FLAG, FIN_FLAG, FIN_ACK_FLAG, RST_FLAG, RST_ACK_FLAG} tcpFlags;

typedef struct
{
    uint16_t sourcePort;
    uint16_t destPort;
    uint32_t sequenceNumber;
    uint32_t acknowledgmentNumber;
    uint8_t  reserved   : 4;
    uint8_t  dataOffset : 4;                    // The number of 32 bit words in the TCP Header

    // Flags (6 Bit)
    uint8_t  FIN : 1;                           // No more data from sender
    uint8_t  SYN : 1;                           // Synchronize sequence numbers
    uint8_t  RST : 1;                           // Reset
    uint8_t  PSH : 1;                           // Push
    uint8_t  ACK : 1;                           // Acknowledgment
    uint8_t  URG : 1;                           // Urgent
    // only shown in wireshark
    uint8_t  ECN : 1;                           // ECN (reserved)
    uint8_t  CWR : 1;                           // CWR (reserved)

    uint16_t window;
    uint16_t checksum;
    uint16_t urgentPointer;
} __attribute__((packed)) tcpPacket_t;

typedef struct
{
    uint32_t SEQ; // Sequence number
    uint32_t ACK; // Acknoledgement number
    uint32_t LEN; // segment length
    uint32_t WND; // segment windows
    tcpFlags CTL; // control bits
} tcpSegment_t;

typedef struct
{
    uint32_t UNA;   // Send Unacknowledged
    uint32_t NXT;   // Send Next
    uint32_t ISS;   // Initial send sequence number
    uint16_t WND;   // Send Window
} tcpSend_t;

typedef struct
{
    uint32_t NXT;   // Sequence number of next received set
    uint32_t IRS;   // Initial receive sequence number
    uint32_t dACK;  // duplicated ACK counter
    uint32_t ACKforDupACK;
    uint16_t WND;   // Receive Window
} tcpRcv_t;

typedef struct
{
    tcpSend_t    SND;
    tcpRcv_t     RCV;
    tcpSegment_t SEG;    // information about segment to be sent next
    uint32_t     srtt;   // (milliseconds)
    uint32_t     rttvar; // (milliseconds)
    uint32_t     rto;    // retransmission timeout (milliseconds)
    uint32_t     msl;    // maximum segment lifetime (milliseconds)
    bool         retrans;
} tcpTransmissionControlBlock_t;

typedef struct
{
    IP4_t    IP;
    uint16_t port;
} tcpSocket_t;

typedef struct
{
    uint32_t                      ID;
    tcpSocket_t                   localSocket;
    tcpSocket_t                   remoteSocket;
    network_adapter_t*            adapter;
    tcpTransmissionControlBlock_t tcb;
    TCP_state                     TCP_PrevState;
    TCP_state                     TCP_CurrState;
    task_t*                       owner;
    list_t                        inBuffer;
    list_t                        OutofOrderinBuffer;
    list_t                        outBuffer;
    list_t                        sendBuffer;
    bool                          passive; // Used to enable output of incoming packets in the kernel console
} tcpConnection_t;

typedef struct
{
    uint32_t connectionID;
    size_t   length;
} __attribute__((packed)) tcpReceivedEventHeader_t;

typedef struct
{
    uint32_t connectionID;
    IP4_t     sourceIP;
    uint16_t sourcePort;
} __attribute__((packed)) tcpConnectedEventHeader_t;

typedef struct
{
    uint32_t  seq;
    tcpReceivedEventHeader_t ev;
} tcpIn_t;

typedef struct
{
    void*        data;
    tcpSegment_t segment;
    uint32_t     time_ms_transmitted;
} tcpOut_t;

typedef struct
{
    void*  data;
    size_t length;
} tcpSendBufferPacket;


tcpConnection_t* tcp_createConnection(void);
void tcp_deleteConnection(tcpConnection_t* connection);
void tcp_cleanup(task_t* task);
void tcp_bind(tcpConnection_t* connection, network_adapter_t* adapter);
void tcp_connect(tcpConnection_t* connection);
void tcp_close(tcpConnection_t* connection);
void tcp_receive(network_adapter_t* adapter, const tcpPacket_t* tcp, size_t length, IP4_t transmittingIP);
void tcp_send(tcpConnection_t* connection, const void* data, uint32_t length);
void tcp_showConnections(void);
tcpConnection_t* tcp_findConnection(IP4_t IP, uint16_t port, network_adapter_t* adapter, TCP_state state);

// User functions
uint32_t tcp_uconnect(IP4_t IP, uint16_t port);
bool     tcp_usend(uint32_t ID, const void* data, size_t length);
bool     tcp_uclose(uint32_t ID);


#endif
