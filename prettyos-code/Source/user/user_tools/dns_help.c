/*
 * License and disclaimer for the use of this source code as per statement below.
 * Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten.
 */

/* DNS help functions
 * Todo:  - getHostByName() which returns all hosts for a name.
 *        - Parser for authority, additional. (See dns_header)
 *        - Parser for RR's: NS, MD, MF, SOA, MB, MG, MR, NULL,
 *                           WKS, PTR, HINFO, MINFO, MX, TXT.
 *                           (See dns_qtype)
 */

#include "dns_help.h"
#include "dns.h"
#include "stdio.h"
#include "userlib.h"


IP4_t getAddrByName(const char* name)
{
    char buf[512];
    uint16_t query_id = 1911; /// TODO
    IP4_t host, dns_server;
    host.iIP4 = dns_server.iIP4 = 0;

    size_t query_size = dns_createSimpleQuery(buf, sizeof(buf), name, query_id);

    dns_getServer(&dns_server);
    event_enable(true);

    udp_bind(dns_port);
    if (query_size && dns_server.iIP4 && udp_send(buf, query_size, dns_server, dns_port, dns_port))
    {
        EVENT_t ev;
        while ((ev = event_poll(buf, sizeof(buf), EVENT_NONE)) != EVENT_UDP_RECEIVED)
        {
            switch (ev)
            {
                case EVENT_NONE:
                    waitForEvent(0);
                    break;
                case EVENT_KEY_DOWN:
                    if (*(KEY_t*)buf == KEY_ESC)
                        return host;
                    break;
                default:
                    break;
            }
        }

        udpReceivedEventHeader_t* udp_header = (udpReceivedEventHeader_t*)buf;
        char* data = (char*)(udp_header + 1);
        size_t len = udp_header->length;

        if (len)
        {
            dns_header header;
            dns_question question;
            dns_resource resource;
            const char* p = dns_parseHeader(&header, data, len);
            while (p && header.qdcount--)
            { // discard questions
                p = dns_parseQuestion(&question, data, len, p);
            }
            while (p && header.ancount--)
            { // look for ipv4 addr
                if ((p = dns_parseResource(&resource, data, len, p)))
                {
                    if (resource.type == dns_type_A && //ipv4
                        resource.dns_class == dns_class_IN && // internet
                        resource.rdlength >= 4) // need min 4 bytes
                    {
                        host.iIP4 = *(uint32_t*)resource.rdata;
                        break;
                    }
                }
            }
        }
    }

    udp_unbind(dns_port);
    return host;
}

void printDNSHeader(const dns_header *header)
{
    printf("ID: %u\n""Flags: %u\n""Questions: %u\n"
        "Answers: %u\n""Authoritys: %u\n""Additionals: %u\n",
        header->id, header->flags, header->qdcount,
        header->ancount, header->arcount, header->nscount);
}

void printDNSQuestion(const dns_question *question)
{
    printf("Name: %s\n""Type: %u\n""Class: %u\n",
        question->qname, question->qtype, question->qclass);
}

void printDNSResource(const dns_resource *resource)
{
    printf("Name: %s\n""Type: %u\n""Class: %u\n"
        "TTL: %ul\n""RDLength: %u\n",
        resource->name, resource->type, resource->dns_class,
        resource->ttl, resource->rdlength);
}

void showDNSQuery(const char* name)
{
    char buf[512];
    uint16_t query_id = 1911; /// TODO
    IP4_t dns_server;

    printf("Begin DNS query for %s..\n", name);

    size_t query_size = dns_createSimpleQuery(buf, sizeof(buf), name, query_id);

    dns_getServer(&dns_server);
    printf("DNS Server: %u.%u.%u.%u\n",
        dns_server.IP4[0], dns_server.IP4[1],
        dns_server.IP4[2], dns_server.IP4[3]);

    event_enable(true);
    udp_bind(dns_port);

    if (query_size && dns_server.iIP4 && udp_send(buf, query_size, dns_server, dns_port, dns_port))
    {
        EVENT_t ev;
        while ((ev = event_poll(buf, sizeof(buf), EVENT_NONE)) != EVENT_UDP_RECEIVED)
        {
            switch (ev)
            {
                case EVENT_NONE:
                    waitForEvent(0);
                    break;
                case EVENT_KEY_DOWN:
                    if (*(KEY_t*)buf == KEY_ESC)
                        return;
                    break;
                default:
                    break;
            }
        }

        udpReceivedEventHeader_t* udp_header = (udpReceivedEventHeader_t*)buf;
        char* data = (char*)(udp_header + 1);
        size_t len = udp_header->length;

        if (len)
        {
            dns_header header;
            dns_question question;
            dns_resource resource;
            const char* p = dns_parseHeader(&header, data, len);
            if (p)
            {
                printf(" -- header --\n");
                printDNSHeader(&header);
            }
            while (p && header.qdcount--)
            {
                if ((p = dns_parseQuestion(&question, data, len, p)))
                {
                    printf(" -- question --\n");
                    printDNSQuestion(&question);
                    getchar();
                }
            }
            while (p && header.ancount--)
            {
                if ((p = dns_parseResource(&resource, data, len, p)))
                {
                    printf(" -- answer --\n");
                    printDNSResource(&resource);
                    if (resource.type == dns_type_A &&
                        resource.dns_class == dns_class_IN &&
                        resource.rdlength >= 4)
                    {
                        printf("Found IP: %u.%u.%u.%u\n",
                            (unsigned char)resource.rdata[0],
                            (unsigned char)resource.rdata[1],
                            (unsigned char)resource.rdata[2],
                            (unsigned char)resource.rdata[3]);
                    }
                    else if (resource.type == dns_type_CNAME &&
                        resource.dns_class == dns_class_IN &&
                        resource.rdlength < 256)
                    {
                        char alias[256];
                        if (dns_parseName(alias, data, len, resource.rdata))
                        {
                            printf("Found CNAME: %s\n", alias);
                        }
                    }
                    getchar();
                }
            }
        }
    }

    udp_unbind(dns_port);
    printf(" -- END --\n");

    getchar();
}


/*
 * Copyright (c) 2011-2013 The PrettyOS Project. All rights reserved.
 *
 * http://www.c-plusplus.de/forum/viewforum-var-f-is-62.html
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
