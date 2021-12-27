#ifndef DNS_H
#define DNS_H

#include "stdint.h"
#include "stddef.h"

/* Ref: tools.ietf.org/html/rfc1035
 * Todo: Rename dns_type, dns_class to dns_qtype, dns_qclass
 *       and insert the missing values.
 */

extern const uint16_t dns_port; // 53

typedef enum
{ // Ref: 3.2.2. TYPE values, 3.2.3. QTYPE values
    dns_type_A = 1, // a host address
    dns_type_NS, // an authoritative name server
    dns_type_MD, // a mail destination (Obsolete - use MX)
    dns_type_MF, // a mail forwarder (Obsolete - use MX)
    dns_type_CNAME, // the canonical name for an alias
    dns_type_SOA, // marks the start of a zone of authority
    dns_type_MB, // a mailbox domain name (EXPERIMENTAL)
    dns_type_MG, // a mail group member (EXPERIMENTAL)
    dns_type_MR, // a mail rename domain name (EXPERIMENTAL)
    dns_type_NULL, // a null RR (EXPERIMENTAL)
    dns_type_WKS, // a well known service description
    dns_type_PTR, // a domain name pointer
    dns_type_HINFO, // host information
    dns_type_MINFO, // mailbox or mail list information
    dns_type_MX, // mail exchange
    dns_type_TXT, // text strings
} dns_type;

typedef enum
{ // Ref: 3.2.4. CLASS values, 3.2.5. QCLASS values
    dns_class_IN = 1, // the Internet
    dns_class_CS, // the CSNET class (Obsolete)
    dns_class_CH, // the CHAOS class
    dns_class_HS, // Hesiod [Dyer 87]
} dns_class;

typedef struct
{ // Ref: 4.1.1. Header section format
    unsigned QR : 1; // QR => 0 query, 1 response
    unsigned OPCODE : 4; // OPCODE => 0 standard, 1 inverse, 2, status request
    unsigned AA : 1; // AA => specifies that the responding name server is an
                     //       authority for the domain name in question section.
    unsigned TC : 1; // TC => this message was truncated
    unsigned RD : 1; // RD => recursion disired
    unsigned RA : 1; // RA => recursion aviable
    unsigned Z : 3; // Z => reserved
    unsigned RCODE : 4; // RCODE => 0 no error, 1 format error, 2 server failure,
                        //          3 name error, 4 not implemented, 5 refused
} __attribute__((packed)) dns_flags;

typedef struct
{ // Ref: 4.1.1. Header section format
    uint16_t id; // will copied to the answer
    uint16_t flags; // see above
    uint16_t qdcount; // question counter
    uint16_t ancount; // answer counter
    uint16_t nscount; // authority counter
    uint16_t arcount; // additional counter
} __attribute__((packed)) dns_header;

typedef struct
{ // Ref: 4.1.2. Question section format
    char qname[256]; // max size for dns name is 255
    uint16_t qtype; // see dns_qtype
    uint16_t qclass; // see dns_qclass
} __attribute__((packed)) dns_question;

typedef struct
{ // Ref: 4.1.3. Resource record format
    char name[256]; // Ref: 4.1.4. Message compression
    uint16_t type; // see dns_qtype (this is only type, not qtype)
    uint16_t dns_class; // class is keyword, see above
    uint32_t ttl; // time to life
    uint16_t rdlength; // length of rdata
    const char* rdata; // Ref: 3.3. Standard RRs
} __attribute__((packed)) dns_resource;


// Use this function, _not_ memcpy().
void dns_fillHeaderWithFlags(dns_header* header, const dns_flags* flags);

// Return value: Bytes written into buf.
// 0 = failure, content of buf is undefined.
size_t dns_writeHeaderToBuffer(char* buf, size_t buf_size, const dns_header* header);

// Return value: Bytes written into buf.
// 0 = failure, content of buf is undefined.
size_t dns_writeQuestionToBuffer(char* buf, size_t buf_size, const dns_question* question);

// Return value: Bytes written into buf.
// 0 = failure, content of buf is undefined.
size_t dns_createSimpleQueryBuffer(char* buf, size_t buf_size, const dns_header* header, const dns_question* question);

// Use this function to create a simple DNS query for a name.
// The buffer is filled with <return value> bytes,
// you can send them to a DNS server as a query.
// 0 = failure
size_t dns_createSimpleQuery(char* buf, size_t buf_size, const char* url, uint16_t id);

// Return value: Byte behind the last byte of header.
// 0 = failure, content of header is undefined.
const char* dns_parseHeader(dns_header* header, const char* buf, size_t buf_size);

// Return value: Byte behind the last byte of buf.
// 0 = failure, content of buf is undefined.
const char* dns_parseName(char* dst, const char* buf, size_t buf_size, const char* pos);

// Return value: Byte behind the last byte of question.
// 0 = failure, content of question is undefined.
const char* dns_parseQuestion(dns_question* question, const char* buf, size_t buf_size, const char* pos);

// Return value: Byte behind the last byte of resource.
// 0 = failure, content of resource is undefined.
const char* dns_parseResource(dns_resource* resource, const char* buf, size_t buf_size, const char* pos);


#endif
