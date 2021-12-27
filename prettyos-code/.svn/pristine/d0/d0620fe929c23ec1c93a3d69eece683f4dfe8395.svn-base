#ifndef DHCP_H
#define DHCP_H

#include "os.h"
#include "network/netutils.h"

// http://tools.ietf.org/html/rfc2131 <--- Dynamic Host Configuration Protocol
// ftp://ftp.efo.ru/pub/wiznet/W5100_App%20note_DHCP.pdf

#define UNICAST      0
#define BROADCAST  128 // 1....... ........
#define OPTIONSIZE 340 // results in size of 576


typedef enum { DHCP_START, DHCP_OFFER, DHCP_ACK, DHCP_NAK } DHCP_state;

typedef struct          // complete length: 576 (0x0240)
{
    uint8_t  op;        // DHCP_BOOTREQEUST or DHCP_BOOTREPLY
    uint8_t  htype;     // DHCP_HTYPE10MB
    uint8_t  hlen;      // DHCP_HLENETHERNET
    uint8_t  hops;      // DHCP_HOPS
    uint32_t xid;       // DHCP_XID
    uint16_t secs;      // DHCP_SECS
    uint16_t flags;     // DHCP_FLAGSBROADCAST
    IP4_t     ciaddr;
    IP4_t     yiaddr;
    IP4_t     siaddr;
    IP4_t     giaddr;
    uint8_t  chaddr[16];
    char     sname[64];
    char     file[128];
    uint8_t  options[OPTIONSIZE];
} __attribute__((packed)) dhcp_t;

/*
   Message         Use
   --------------------------------------------------------------------

   DHCPDISCOVER -  Client broadcast to locate available servers.

   DHCPOFFER    -  Server to client in response to DHCPDISCOVER with
                   offer of configuration parameters.

   DHCPREQUEST  -  Client message to servers either (a) requesting
                   offered parameters from one server and implicitly
                   declining offers from all others, (b) confirming
                   correctness of previously allocated address after,
                   e.g., system reboot, or (c) extending the lease on a
                   particular network address.

   DHCPACK      -  Server to client with configuration parameters,
                   including committed network address.

   DHCPNAK      -  Server to client indicating client's notion of network
                   address is incorrect (e.g., client has moved to new
                   subnet) or client's lease as expired

   DHCPDECLINE  -  Client to server indicating network address is already
                   in use.

   DHCPRELEASE  -  Client to server relinquishing network address and
                   cancelling remaining lease.

   DHCPINFORM   -  Client to server, asking only for local configuration
                   parameters; client already has externally configured
                   network address.
*/

struct network_adapter;

void DHCP_Discover(struct network_adapter* adapter);
void DHCP_Request(struct network_adapter* adapter, IP4_t requestedIP);
void DHCP_Inform  (struct network_adapter* adapter);
void DHCP_Release (struct network_adapter* adapter);
void DHCP_AnalyzeServerMessage(struct network_adapter* adapter, const dhcp_t* dhcp, IP4_t sourceIP);

/*
When a client is initialized for the first time after it is configured
to receive DHCP information, it initiates a conversation with the server.
Below is a summary table of the conversation between client and server,
which is followed by a packet-level description of the process:

   Source     Dest        Source     Dest              Packet
   MAC addr   MAC addr    IP addr    IP addr           Description
   -----------------------------------------------------------------
   Client     Broadcast   0.0.0.0    255.255.255.255   DHCP Discover
   DHCPsrvr   Broadcast   DHCPsrvr   255.255.255.255   DHCP Offer
   Client     Broadcast   0.0.0.0    255.255.255.255   DHCP Request
   DHCPsrvr   Broadcast   DHCPsrvr   255.255.255.255   DHCP ACK
*/

#endif
