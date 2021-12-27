/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "fadt.h"
#include "ipc.h"
#include "parser.h"
#include "util/util.h"

const char* const fadtPath = "/fadt";

typedef struct
{
    uint8_t AddressSpace;
    uint8_t BitWidth;
    uint8_t BitOffset;
    uint8_t AccessSize;
    uint64_t Address;
} genericAddressStructure_t;

typedef struct
{
    acpi_header_t header;

    uint32_t firmwareCtrl;
    uint32_t dsdt;

    uint8_t  reserved;

    uint8_t  preferredPowerManagementProfile;
    uint16_t sCI_Interrupt;
    uint32_t sMI_CommandPort;
    uint8_t  acpiEnable;
    uint8_t  acpiDisable;
    uint8_t  S4BIOS_REQ;
    uint8_t  PSTATE_Control;
    uint32_t PM1aEventBlock;
    uint32_t PM1bEventBlock;
    uint32_t PM1aControlBlock;
    uint32_t PM1bControlBlock;
    uint32_t PM2ControlBlock;
    uint32_t PMTimerBlock;
    uint32_t GPE0Block;
    uint32_t GPE1Block;
    uint8_t  PM1EventLength;
    uint8_t  PM1ControlLength;
    uint8_t  PM2ControlLength;
    uint8_t  PMTimerLength;
    uint8_t  GPE0Length;
    uint8_t  GPE1Length;
    uint8_t  GPE1Base;
    uint8_t  CStateControl;
    uint16_t worstC2Latency;
    uint16_t worstC3Latency;
    uint16_t flushSize;
    uint16_t flushStride;
    uint8_t  dutyOffset;
    uint8_t  dutyWidth;
    uint8_t  dayAlarm;
    uint8_t  monthAlarm;
    uint8_t  century;

    uint16_t bootArchitectureFlags; // reserved in ACPI 1.0; used since ACPI 2.0+

    uint8_t  reserved2;
    uint32_t flags;

    genericAddressStructure_t resetReg;

    uint8_t  resetValue;
    uint8_t  reserved3[3];

    // 64 bit pointers - available on ACPI 2.0+
    uint64_t                  X_FirmwareControl;
    uint64_t                  X_Dsdt;

    genericAddressStructure_t X_PM1aEventBlock;
    genericAddressStructure_t X_PM1bEventBlock;
    genericAddressStructure_t X_PM1aControlBlock;
    genericAddressStructure_t X_PM1bControlBlock;
    genericAddressStructure_t X_PM2ControlBlock;
    genericAddressStructure_t X_PMTimerBlock;
    genericAddressStructure_t X_GPE0Block;
    genericAddressStructure_t X_GPE1Block;
} __attribute__((packed)) fadt_table_t;


void acpi_parser_fadt(acpi_header_t* table)
{
    // Set up IPC path
    size_t basePathLength = strlen(acpi_parserBasePath) + strlen(fadtPath);
    char path[basePathLength + 1];
    strcpy(path, acpi_parserBasePath);
    strcat(path, fadtPath);

    if (ipc_getNode(path))
        ipc_deleteKey(path);
    ipc_setTaskWorkingNode(path, true);

    fadt_table_t* tbl = (fadt_table_t*)table;

    if (tbl->header.rev > 1)
    {
        int64_t bootArchitecture = tbl->bootArchitectureFlags;
        ipc_setInt(">/boot_architecture", &bootArchitecture, IPC_QWORD);
    }
}

/*
* Copyright (c) 2016 The PrettyOS Project. All rights reserved.
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