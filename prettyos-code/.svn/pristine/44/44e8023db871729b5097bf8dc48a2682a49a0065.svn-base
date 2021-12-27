/*
 *  license and disclaimer for the use of this source code as per statement below
 *  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
 */

#include "acpi.h"
#include "kheap.h"
#include "paging.h"
#include "parser.h"
#include "util/list.h"
#include "util/util.h"
#ifdef _ACPI_DIAGNOSIS_
#include "video/console.h"
#endif

//////////////////////////////////////////////////////////////////////////
// RSDP

#define EDBA_BEGIN (uint8_t*)0x0009FC00
#define EDBA_END (uint8_t*)0x000A0000 // Pointer to post-last byte of the edba

#define BIOS_BEGIN (uint8_t*)0x000E0000
#define BIOS_END (uint8_t*)0x00100000 // Pointer to post-last byte of the bios rom

#define ACPI_RDSP_SIG_LEN 8 // Root system descriptor pointer signature length

static const char* const ACPI_RSDP_SIG = "RSD PTR ";

// Root system descriptor pointer
// Can be found 16-byte-aligned at
// -> the first 1 KB of the Extended BIOS Data Area
// -> the BIOS read-only memory space between 0E0000h and 0FFFFFh
typedef struct
{
    char sig[ACPI_RDSP_SIG_LEN]; // "RSD PTR "
    uint8_t checksum; // Checksum of the bytes 0-19 (defined in ACPI rev 1); bytes must add to 0
    char oemID[ACPI_OEMID_LEN];
    uint8_t rev;
    void* rootSystemTable;
    // All the fields above are used to calc the checksum
    //////////////////////////////////////////////////////////////////////////

    uint32_t rootSystemTableLen;
    uint64_t extendedRootSystemTableAddr; // 64 bit phys addr
    uint8_t checksumEx; // Checksum of the whole structure including the other checksum field
    uint8_t reserved[3];
} __attribute__((packed)) acpi_rsdp_t;


static acpi_rsdp_t* findRootSystemDescriptorPointer(void* itbeg, void* itend)
{
    for (uint8_t* it = (uint8_t*)itbeg; it < (uint8_t*)itend; it += 16)
    {
        acpi_rsdp_t* rsdp = (acpi_rsdp_t*)it;
        if (memcmp(rsdp->sig, ACPI_RSDP_SIG, 8) == 0)
        {
            uint8_t val = 0;

            void* end = it + 20; // ACPI 1.0 spec size
            for (uint8_t* mem = it; mem != end; ++mem) val += *mem; // First checksum

            if (!val && rsdp->rev == 2) // Test worked && newest ACPI spec
            {
                end = it + sizeof(acpi_rsdp_t) - 3; // Current RDSP size - 3 reserved bytes
                for (uint8_t* mem = it; mem != end; ++mem) val += *mem; // Extended checksum
            }

            if (!val) return rsdp;
        }
    }

    return (0);
}

// RSDP should reside within the mapped address space
static acpi_rsdp_t* getRootSystemDescriptorPointer(void)
{
    uint8_t* ebda = (uint8_t*) ((*(uint16_t*)0x40E) >> 4);
    acpi_rsdp_t* ret = findRootSystemDescriptorPointer(ebda, ebda + 1024);
    if (ret)
        return ret;

    ret = findRootSystemDescriptorPointer(BIOS_BEGIN, BIOS_END);
    if (ret)
        return ret;

    ret = findRootSystemDescriptorPointer(EDBA_BEGIN, EDBA_END);
    return ret;
}

//////////////////////////////////////////////////////////////////////////
// RDST

const char ACPI_TABSIG_RSDT[ACPI_TABLE_SIG_LEN] = "RSDT"; // Root system descriptor table

// Root system descriptor table
typedef struct
{
    acpi_header_t header;
} __attribute__((packed)) acpi_rsdt_t; // Rsdt is only header + n pointers to other descriptor tables
                                       // n = (table lenght - sizeof(header)) / 4

typedef struct
{
    char* signature;
    acpi_header_t* begin;
    uint32_t len;
} acpi_table_t;

static void* acpi_tryAlloc(uintptr_t paddr, size_t pages, bool* dealloc)
{
    paddr = alignDown(paddr, PAGESIZE);
    *dealloc = paging_allocPhysAddr(paddr, pages); // Only dealloc this physical page later if it was not allocated already.
    // TODO: Use paging_allocVirtMem instead of paging_allocMMIO
    return paging_allocMMIO(alignDown(paddr, PAGESIZE), pages);
}

static void acpi_tryDealloc(void* vaddr, size_t pages, bool dealloc)
{
    if (dealloc)
        paging_freePhysMem(paging_getPhysAddr(vaddr), pages);
    paging_freeVirtMem(kernelPageDirectory, vaddr, pages);
}

static acpi_table_t* acpi_analyzeRSDTEntry(acpi_header_t* hdr)
{
    // Mapping prologue
    bool dealloc = false;
    uint32_t pages = (alignUp((uint32_t)hdr + sizeof(acpi_header_t), PAGESIZE)
                      - alignDown((uint32_t)hdr, PAGESIZE)) / PAGESIZE;
    uint8_t* mapped = acpi_tryAlloc((uintptr_t)hdr, pages, &dealloc);
    ACPIASSERT(mapped);
    acpi_header_t* header = (void*)(mapped + (uint32_t)hdr % PAGESIZE);
    // Mapping prologue end

    acpi_table_t* entry = malloc(sizeof(acpi_table_t), 0, "acpi_table_t*");
    entry->begin = hdr;
    entry->len = header->len;
    entry->signature = malloc(ACPI_TABLE_SIG_LEN + 1, 0, "acpi_table_t.signature"); // Len + null terminator
    memcpy(entry->signature, &header->signature, ACPI_TABLE_SIG_LEN);
    entry->signature[ACPI_TABLE_SIG_LEN] = '\0';

    acpi_tryDealloc(mapped, pages, dealloc);

    return entry;
}

void acpi_parseRSDT(acpi_rsdt_t* tbl, list_t* list)
{
    // Mapping prologue
    // RSDT not guaranteed to be within mapped space
    // In order to find out, how many pages we have to map for the full RSDT, we first need to map the header...
    bool dealloc = false;
    uint32_t headerPages = (alignUp((uint32_t)tbl + sizeof(acpi_rsdt_t), PAGESIZE)
                            - alignDown((uint32_t)tbl, PAGESIZE)) / PAGESIZE;
    uint8_t* mapped = acpi_tryAlloc((uintptr_t)tbl, headerPages, &dealloc);
    ACPIASSERT(mapped);
    acpi_rsdt_t* table = (void*)(mapped + (uint32_t)tbl % PAGESIZE);
    // If the RSDT is longer than the original header-mapping, remap it
    uint32_t pages = (alignUp((uint32_t)tbl + table->header.len, PAGESIZE)
                      - alignDown((uint32_t)tbl, PAGESIZE)) / PAGESIZE;

    if (pages != headerPages)
    {
        acpi_tryDealloc(mapped, headerPages, dealloc);
        mapped = acpi_tryAlloc((uintptr_t)tbl, pages, &dealloc);
        table = (void*)(mapped + (uint32_t)tbl % PAGESIZE);
    }
    // Mapping prologue end

    if (memcmp(table->header.signature, ACPI_TABSIG_RSDT, ACPI_TABLE_SIG_LEN) != 0) // Signature failed
    {
        acpi_tryDealloc(table, pages, dealloc);
        return;
    }

    // Iterate over the table entries following the header
    for (uint32_t* it = (uint32_t*)((uint8_t*)table + sizeof(acpi_rsdt_t));
            it != (uint32_t*)((uint8_t*)table + table->header.len); ++it)
    {
        acpi_table_t* entry = acpi_analyzeRSDTEntry((acpi_header_t*)*it);
        list_append(list, entry);
    }

    acpi_tryDealloc(mapped, pages, dealloc);
}

//////////////////////////////////////////////////////////////////////////
// General Interface

static acpi_rsdp_t* rdsp;
static list_t tables = list_init();
static list_t parsers = list_init();

#ifdef _ACPI_DIAGNOSIS_
static inline void acpi_diagnostics(const char* func)
{
    uint8_t oldCol = getTextColor();

    textColor(YELLOW);
    printf("\n\nACPI-Diagnostics [%s]:\n", func);
    printf("\tRDSP: %X\n", rdsp);
    if (rdsp)
    {
        puts("\t\tSignature:\t");
        for (unsigned i = 0; i < ACPI_RDSP_SIG_LEN; ++i)
            putch(rdsp->sig[i]);
        putch('\n');

        printf("\t\tChecksum:\t%y\n", rdsp->checksum);

        puts("\t\tOEM-ID:\t");
        for (unsigned i = 0; i < ACPI_OEMID_LEN; ++i)
            putch(rdsp->oemID[i]);
        putch('\n');

        printf("\t\tRevision:\t%y\n", rdsp->rev);
        printf("\t\tRDST-Addr.:\t%X\n", rdsp->rootSystemTable);
    }
    printf("\tTables:\n");
    for (dlelement_t* it = tables.head; it != 0; it = it->next)
    {
        acpi_table_t* table = it->data;
        printf("\t\t%s\t%X\t%X\n", table->signature, table->begin, table->len);
    }
    printf("\tParsers:\n");
    for (dlelement_t* it = parsers.head; it != 0; it = it->next)
    {
        acpi_parser_t* parser = it->data;
        printf("\t\t%s\t%X\n", parser->signature, parser->parser);
    }

    textColor(oldCol);
}
#define DIAGNOSIS() acpi_diagnostics(__FUNCTION__)
#else
#define DIAGNOSIS()
#endif

bool acpi_available(void)
{
    rdsp = getRootSystemDescriptorPointer();
    return rdsp;
}

void acpi_install(void)
{
    for (acpi_parser_t* it = acpi_baseParsers; it->parser != 0; ++it)
        list_append(&parsers, it);

    if (rdsp != 0)
    {
        acpi_parseRSDT(rdsp->rootSystemTable, &tables);
    }

    acpi_reparse();

    DIAGNOSIS();
}

bool acpi_registerParser(const char* signature, void(*parser)(acpi_header_t* table))
{
    acpi_parser_t entry = {signature, parser};

    // Avoid double-registering the same parser
    for (dlelement_t* it = parsers.head; it; it = it->next)
        if (memcmp(it->data, &entry, sizeof(acpi_parser_t)) == 0)
            return false;

    acpi_parser_t* lstentry = malloc(sizeof(acpi_parser_t), 0, "acpi_parser_t");
    memcpy(lstentry, &entry, sizeof(acpi_parser_t));
    list_append(&parsers, lstentry);
    DIAGNOSIS();
    return true;
}

bool acpi_unregisterParser(const char* signature, void(*parser)(acpi_header_t*))
{
    acpi_parser_t entry = {signature, parser};

    // Find the correct element
    for (dlelement_t* it = parsers.head; it; it = it->next)
        if (memcmp(it->data, &entry, sizeof(acpi_parser_t)) == 0)
        {
            free(it->data);
            list_delete(&parsers, it);
            DIAGNOSIS();
            return true;
        }

    return false;
}

void acpi_reload(void)
{
    // Free the elements within the list
    for (dlelement_t* it = tables.head; it != 0; it = it->next)
    {
        acpi_table_t* tbl = it->data;
        free(tbl->signature);
        free(tbl);
    }

    // Create a new empty list
    list_destruct(&tables);
    list_construct(&tables);

    // Fill the list again
    acpi_parseRSDT(rdsp->rootSystemTable, &tables);
    DIAGNOSIS();

    acpi_reparse();
}

static inline void passToParser(acpi_table_t* table)
{
    // Mapping prologue (again)
    bool dealloc = false;
    uint32_t pages = (alignUp((uint32_t)table->begin + table->len, PAGESIZE)
                      - alignDown((uint32_t)table->begin, PAGESIZE)) / PAGESIZE;
    uint8_t* mapped = acpi_tryAlloc((uintptr_t)table->begin, pages, &dealloc);
    ACPIASSERT(mapped);
    acpi_header_t* hdr = (void*)(mapped + (uint32_t)table->begin % PAGESIZE);
    // Mapping prologue end

    for (dlelement_t* it = parsers.head; it; it = it->next)
    {
        acpi_parser_t* parser = it->data;
        if (strcmp(table->signature, parser->signature) == 0)
            parser->parser(hdr);
    }

    acpi_tryDealloc(mapped, pages, dealloc);
}

void acpi_reparse(void)
{
    for (dlelement_t* it = tables.head; it != 0; it = it->next)
    {
        passToParser(it->data);
    }
}

void acpi_parseSpecific(const char* signtature)
{
    for (dlelement_t* it = tables.head; it != 0; it = it->next)
    {
        acpi_table_t* table = it->data;
        if (strcmp(table->signature, signtature) == 0)
            passToParser(table);
    }
}

/*
* Copyright (c) 2014-2015 The PrettyOS Project. All rights reserved.
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
