/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "madt.h"
#include "ipc.h"
#include "parser.h"
#include "util/util.h"

const char* const madtPath = "/madt";

#define MADT_FLAGS_PCAT_COMPAT 1 // System has usual dual PIC set up as well

typedef struct
{
    acpi_header_t header;
    uint32_t localApicAddr;
    uint32_t flags;
} __attribute__((packed)) madt_table_t;

typedef struct
{
    uint8_t type;
    uint8_t length;
} __attribute__((packed)) madt_table_entry_header_t;

#define MADT_ENTRY_LOCAL_APIC_FLAGS_ENABLED 1

typedef struct
{
    madt_table_entry_header_t header;
    uint8_t processorID;
    uint8_t id;
    uint32_t flags;
} __attribute__((packed)) madt_entry_local_apic_t;

typedef struct
{
    madt_table_entry_header_t header;
    uint8_t id;
    uint8_t reserved_;
    uint32_t addr;
    uint32_t interruptBase; // Global int num at wich IO apic's interrupts will begin being mapped
} __attribute__((packed)) madt_entry_io_apic_t;

typedef struct
{
    madt_table_entry_header_t header;
    uint8_t bus; // Will always be 0 (ISA)
    uint8_t picirq; // Bus relative interrupt (e.g. no + 32)
    uint8_t apicirq; // Int no when mapped on IO-APIC
    uint16_t flags; // See madt.h
} __attribute__((packed)) madt_entry_int_override_t;

typedef struct
{
    madt_table_entry_header_t header;
    uint16_t flags; // See madt.h
    uint32_t intno; // Interrupt that will fire
} __attribute__((packed)) madt_entry_nmi_t;

typedef struct
{
    madt_table_entry_header_t header;
    uint8_t processorId; // 0xFF = all Processors on the System
    uint16_t flags; // See madt.h
    uint8_t lintPin; // LINT pin of local apic NMI is connected to
} __attribute__((packed)) madt_entry_lapic_nmi_t;

typedef struct
{
    madt_table_entry_header_t herader;
    uint16_t reserved_;
    uint64_t addr;
} __attribute__((packed)) madt_entry_lapic_addr_override_t;

static void parseLAPICEntry(madt_entry_local_apic_t* entry, unsigned* id)
{
    if ((entry->flags & MADT_ENTRY_LOCAL_APIC_FLAGS_ENABLED) == 0)
        return;

    char buffer[30];
    snprintf(buffer, 30, ">/lapic/%u", (*id)++);
    ipc_setTaskWorkingNode(buffer, true);

    int64_t lapicid = entry->id;
    ipc_setInt(">/id", &lapicid, IPC_UINT);

    int64_t procId = entry->processorID;
    ipc_setInt(">/processor_id", &procId, IPC_UINT);
}

static void parseIOAPICEntry(madt_entry_io_apic_t* entry, unsigned* id)
{
    char buffer[30];
    snprintf(buffer, 30, ">/ioapic/%u", (*id)++);
    ipc_setTaskWorkingNode(buffer, true);

    int64_t ioapicid = entry->id;
    ipc_setInt(">/id", &ioapicid, IPC_UINT);

    int64_t addr = (uint32_t)entry->addr;
    ipc_setInt(">/address", &addr, IPC_QWORD);

    int64_t base = entry->interruptBase;
    ipc_setInt(">/int_base", &base, IPC_UINT);
}

static void parseIntOverrideEntry(madt_entry_int_override_t* entry, unsigned* id)
{
    char buffer[30];
    snprintf(buffer, 30, ">/intoverride/%u", (*id)++);
    ipc_setTaskWorkingNode(buffer, true);

    //int64_t bus = entry->bus; // Will always be 0, so we don't need to write it to IPC

    int64_t picno = entry->picirq;
    ipc_setInt(">/irq", &picno, IPC_UINT);

    int64_t apicno = entry->apicirq;
    ipc_setInt(">/apicmapped", &apicno, IPC_UINT);

    int64_t flags = entry->flags; // Use defines in madt.h to get additional info
    ipc_setInt(">/flags", &flags, IPC_QWORD);
}

static void parseNMIEntry(madt_entry_nmi_t* entry, unsigned* id)
{
    char buffer[30];
    snprintf(buffer, 30, ">/nmi/%u", (*id)++);
    ipc_setTaskWorkingNode(buffer, true);

    int64_t intno = entry->intno;
    ipc_setInt(">/int", &intno, IPC_UINT);

    int64_t flags = entry->flags; // Use defines in madt.h to get additional info
    ipc_setInt(">/flags", &flags, IPC_QWORD);
}

static void parseLAPICNMIEntry(madt_entry_lapic_nmi_t* entry, unsigned* id)
{
    char buffer[30];
    snprintf(buffer, 30, ">/lapic_nmi/%u", (*id)++);
    ipc_setTaskWorkingNode(buffer, true);

    int64_t processor = entry->processorId;
    ipc_setInt(">/processor", &processor, IPC_UINT);

    int64_t flags = entry->flags; // Use defines in madt.h to get additional info
    ipc_setInt(">/flags", &flags, IPC_QWORD);

    int64_t lint = entry->lintPin;
    ipc_setInt(">/lint", &lint, IPC_UINT);
}

void acpi_parser_madt(acpi_header_t* table)
{
    // Set up IPC path
    size_t basePathLength = strlen(acpi_parserBasePath) + strlen(madtPath);
    char path[basePathLength + 1];
    strcpy(path, acpi_parserBasePath);
    strcat(path, madtPath);

    if (ipc_getNode(path))
        ipc_deleteKey(path);
    ipc_setTaskWorkingNode(path, true);

    madt_table_t* tbl = (void*)table;

    int64_t localAddr = (uint32_t)tbl->localApicAddr;
    ipc_setInt(">/lapic_addr", &localAddr, IPC_QWORD);

    // Has PC/AT-compatible setup
    int64_t isCompatible = (tbl->flags & MADT_FLAGS_PCAT_COMPAT) ? 1 : 0;
    ipc_setInt(">/has_dualpic", &isCompatible, IPC_UINT);

    unsigned ioapicid = 0, lapicid = 0, intoverrideid = 0, nmiid = 0, lapicnmiid = 0;

    uint8_t* it = (uint8_t*)table + sizeof(madt_table_t);
    for (; it < (uint8_t*)table + table->len; it += ((madt_table_entry_header_t*)it)->length)
    {
        switch (((madt_table_entry_header_t*)it)->type)
        {
        case MADT_LOCAL_APIC:
            // ACPI Spec 5.2.12.2 Processor Local APIC Structure
            ACPIASSERT(((madt_table_entry_header_t*)it)->length == 8);
            parseLAPICEntry((madt_entry_local_apic_t*)it, &lapicid);
            ipc_setTaskWorkingNode(path, false);
            break;
        case MADT_IO_APIC:
            // ACPI Spec 5.2.12.3 I/O APIC Structure
            ACPIASSERT(((madt_table_entry_header_t*)it)->length == 12);
            parseIOAPICEntry((madt_entry_io_apic_t*)it, &ioapicid);
            ipc_setTaskWorkingNode(path, false);
            break;
        case MADT_INTERRUPT_SOURCE_OVERRIDE:
            // ACPI Spec 5.2.12.5 Interrupt Source Override Structure
            ACPIASSERT(((madt_table_entry_header_t*)it)->length == 10);
            parseIntOverrideEntry((madt_entry_int_override_t*)it, &intoverrideid);
            ipc_setTaskWorkingNode(path, false);
            break;
        case MADT_NMI:
            // ACPI Spec 5.2.12.6 Non-Maskable Interrupt Source Structure
            ACPIASSERT(((madt_table_entry_header_t*)it)->length == 8);
            parseNMIEntry((madt_entry_nmi_t*)it, &nmiid);
            ipc_setTaskWorkingNode(path, false);
            break;
        case MADT_LOCAL_APIC_NMI:
            // ACPI Spec 5.2.12.7 Local APIC NMI Structure
            ACPIASSERT(((madt_table_entry_header_t*)it)->length == 6);
            parseLAPICNMIEntry((madt_entry_lapic_nmi_t*)it, &lapicnmiid);
            ipc_setTaskWorkingNode(path, false);
            break;
        case MADT_LOCAL_APIC_ADDRESS_OVERRIDE:
            // ACPI Spec 5.2.12.8 Local APIC Address Override Structure
            ACPIASSERT(((madt_table_entry_header_t*)it)->length == 12);
            {
                int64_t newaddr = ((madt_entry_lapic_addr_override_t*)it)->addr;
                ipc_setInt(">/lapic_addr", &newaddr, IPC_QWORD);
            }
            break;
        }
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