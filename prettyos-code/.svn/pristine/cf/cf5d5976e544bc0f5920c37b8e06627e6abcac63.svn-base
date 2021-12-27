/*
 * License and disclaimer for the use of this source code as per statement below.
 * Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten.
 */

#include "dns.h"
#include "string.h"
#include "userlib.h"

const uint16_t dns_port = 53;

void dns_fillHeaderWithFlags(dns_header* header, const dns_flags* flags)
{
    header->flags = (flags->QR << 15) | (flags->OPCODE << 11) |
                    (flags->AA << 10) | (flags->TC << 9) |
                    (flags->RD << 8)  | (flags->RA << 7) |
                    (flags->Z << 4)   | (flags->RCODE << 0);
}

size_t dns_writeHeaderToBuffer(char* buf, size_t buf_size, const dns_header* header)
{
    if (buf_size >= 12)
    {
        *(uint16_t*)(buf + 0)  = htons(header->id);
        *(uint16_t*)(buf + 2)  = htons(header->flags);
        *(uint16_t*)(buf + 4)  = htons(header->qdcount);
        *(uint16_t*)(buf + 6)  = htons(header->ancount);
        *(uint16_t*)(buf + 8)  = htons(header->nscount);
        *(uint16_t*)(buf + 10) = htons(header->arcount);
        return (12);
    }
    return (0);
}

size_t dns_writeQuestionToBuffer(char* buf, size_t buf_size, const dns_question* question)
{
    size_t need_space = strlen(question->qname) + 6;
    if (buf_size >= need_space && need_space < 256 + 4)
    {
        // "www.henkessoft.de" -> "\x03www\x0Ahenkessoft\0x02de"
        // Ref: 4.1.2. Question section format, -> QNAME
        //
        char* p;
        strcpy(buf + 1, question->qname);
        while ((p = strchr(buf + 1, '.')))
        {
            if (p - buf - 1 < 0x100)
            {
                *buf = p - buf - 1;
                buf = p;
            }
            else
            {
                return (0);
            }
        }
        uint16_t n = strlen(buf + 1);
        if (n < 64)
        {
            *buf = (char)n;
            buf += n + 1;
            *buf++ = '\0';
            *(uint16_t*)(buf + 0) = htons(question->qtype);
            *(uint16_t*)(buf + 2) = htons(question->qclass);
            return need_space;
        }
    }
    return (0);
}

size_t dns_createSimpleQueryBuffer(char* buf, size_t buf_size, const dns_header* header, const dns_question* question)
{
    size_t w = dns_writeHeaderToBuffer(buf, buf_size, header);
    if (w)
    {
        int v = dns_writeQuestionToBuffer(buf + w, buf_size - w, question);
        if (v)
        {
            return w + v;
        }
    }
    return (0);
}

size_t dns_createSimpleQuery(char* buf, size_t buf_size, const char* url, uint16_t id)
{
    if (strlen(url) < 256)
    {
        dns_flags flags;
        dns_header header;
        dns_question question;
        flags.AA = 0;
        flags.OPCODE = 0; // standard query
        flags.QR = 0;
        flags.RA = 0;
        flags.RCODE = 0;
        flags.RD = 1; // recursion desired
        flags.TC = 0;
        flags.Z = 0;
        header.ancount = 0;
        header.arcount = 0;
        dns_fillHeaderWithFlags(&header, &flags);
        header.id = id;
        header.nscount = 0;
        header.qdcount = 1; // one query
        question.qclass = dns_class_IN; // internet
        strcpy(question.qname, url);
        question.qtype = dns_type_A; // IPv4 addr
        return dns_createSimpleQueryBuffer(buf, buf_size, &header, &question);
    }
    return (0);
}

const char* dns_parseHeader(dns_header* header, const char* buf, size_t buf_size)
{
    if (buf_size >= 12)
    {
        header->id      = htons(*(uint16_t*)(buf + 0));
        header->flags   = htons(*(uint16_t*)(buf + 2));
        header->qdcount = htons(*(uint16_t*)(buf + 4));
        header->ancount = htons(*(uint16_t*)(buf + 6));
        header->nscount = htons(*(uint16_t*)(buf + 8));
        header->arcount = htons(*(uint16_t*)(buf + 10));
        return buf + 12;
    }
    return (0);
}

const char* dns_parseName(char* dst, const char* buf, size_t buf_size, const char* pos)
{
    size_t size = buf_size - (size_t)(pos - buf);
    size_t written = 0;
    const char *p = 0;
    while (size)
    {
        unsigned char i = *pos++;
        for (; i != 0 && i < size && i < 64 && i + written < 256; i = *pos++)
        {
            size -= i + 1;
            written += i + 1;
            while (i--)
                *dst++ = *pos++;
            *dst++ = '.';
        }
        if (i == 0)
        {
            *(--dst) = '\0';
            return p != 0 ? p : pos;
        }
        if (((i & 192) == 192) && size >= 2 )
        { // pointer found
            p = p ? p : pos + 1; // only the first time
            pos = buf + (((i << 8) | *pos) & 0x3FFF);
            if (pos < buf + buf_size)
            {
                size = buf_size - (pos - buf);
                continue;
            }
        }
        break;
    }
    return (0);
}

const char* dns_parseQuestion(dns_question* question, const char* buf, size_t buf_size, const char* pos)
{
    if (buf_size)
    {
        const char* p = dns_parseName(question->qname, buf, buf_size, pos);
        if (p && buf_size - (p - buf) >= 4)
        {
            question->qtype  = htons(*(uint16_t*)(p + 0));
            question->qclass = htons(*(uint16_t*)(p + 2));
            return p + 4;
        }
    }
    return (0);
}

const char* dns_parseResource(dns_resource* resource, const char* buf, size_t buf_size, const char* pos)
{
    if (buf_size)
    {
        const char* p = dns_parseName(resource->name, buf, buf_size, pos);
        if (p && buf_size - (p - buf) >= 10)
        {
            resource->type      = htons(*(uint16_t*)(p + 0));
            resource->dns_class = htons(*(uint16_t*)(p + 2));
            resource->ttl       = htonl(*(uint32_t*)(p + 4));
            resource->rdlength  = htons(*(uint16_t*)(p + 8));
            p += 10;
            if (buf_size - (p - buf) >= resource->rdlength)
            {
                resource->rdata = p;
                return p + resource->rdlength;
            }
        }
    }
    return (0);
}

/*
 * Copyright (c) 2011-2016 The PrettyOS Project. All rights reserved.
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
