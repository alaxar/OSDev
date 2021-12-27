/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.h"
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ctype.h"

#define _PCI_VEND_PROD_LIST_ // http://www.pcidatabase.com/pci_c_header.php - Increases the size of devmgr significantly
#ifdef _PCI_VEND_PROD_LIST_
#include "pciVendProdList.h"
#endif

static void error(const char* string) {
    textColor(ERROR);
    puts(string);
    textColor(TEXT);
}

static bool parseFolder(const char* ipc_path, void (*parser)(const char*), bool onlyNumbers)
{
    size_t folderLength = 0;
    char* folder = 0;
    switch (ipc_getFolder(ipc_path, (char*)&folderLength, sizeof(folderLength))) {
        case IPC_NOTENOUGHMEMORY:
            folder = malloc(folderLength);
            ipc_getFolder(ipc_path, folder, folderLength);
            break;
        case IPC_SUCCESSFUL:
            folder = malloc(sizeof(folderLength));
            memcpy(folder, &folderLength, sizeof(folderLength));
            break;
        default:
            return false;
    }

    // Parse folder content
    char* last = folder;
    char* current = folder+1;
    for (; *current; current++)
    {
        if (*current == '|')
        {
            *current = 0;
            if (!onlyNumbers || isdigit(*last))
                parser(last);
            last = current + 1;
        }
    }

    free(folder);
    return true;
}

static void showHelp(void) {
    printf("Command line arguments:\n"
           "    --help, -?       Prints available command line arguments\n"
           "    --showpci, -sp   Shows all PCI devices present\n"
           "    --showacpi       Prints ACPI table content\n"
           "\n");
}

static void showPCI_devices(const char* name) {
    char buffer[sizeof("PrettyOS/PCI/") + strlen(name)];
    strcpy(buffer, "PrettyOS/PCI/");
    strcat(buffer, name);
    ipc_setWorkingNode(buffer, false);

    int64_t irq = 255;
    ipc_getInt(">/IRQ", &irq);

    // Screen output
    if (irq != 255)
    {
        int64_t bus, dev, func, vendId, devId, classId, subclassId, interfaceId;
        ipc_getInt(">/Bus", &bus);
        ipc_getInt(">/Device", &dev);
        ipc_getInt(">/Function", &func);
        ipc_getInt(">/VendorID", &vendId);
        ipc_getInt(">/DeviceID", &devId);
        ipc_getInt(">/ClassID", &classId);
        ipc_getInt(">/SubclassID", &subclassId);
        ipc_getInt(">/InterfaceID", &interfaceId);

        printf("%d:%d:%d\t%d", (uint32_t)bus, (uint32_t)dev, (uint32_t)func, (uint32_t)irq);

      #ifdef _PCI_VEND_PROD_LIST_
        // Find Vendor
        bool found = false;
        for (uint32_t j = 0; j < PCI_VENTABLE_LEN; j++)
        {
            if (PciVenTable[j].VenId == vendId)
            {
                printf("\t%s", PciVenTable[j].VenShort); // Found! Display name and break out
                found = true;
                break;
            }
        }
        if (!found)
        {
            printf("\tvend: %xh", (uint32_t)vendId); // Vendor not found, display ID
        }
        else
        {
            // Find Device
            found = false;
            for (uint32_t j = 0; j < PCI_DEVTABLE_LEN; j++)
            {
                if (PciDevTable[j].DevId == devId && PciDevTable[j].VenId == vendId) // VendorID and DeviceID have to fit
                {
                    printf(", %s", PciDevTable[j].ChipDesc); // Found! Display name and break out
                    found = true;
                    break;
                }
            }
        }
        if (!found)
        {
            printf(", dev: %xh", (uint32_t)devId); // Device not found, display ID
        }
      #else
        printf("\tvend: %xh, dev: %xh", (uint32_t)vendId, (uint32_t)devId);
      #endif

        for (size_t i = 0; i < PCI_CLASSCODETABLE_LEN; i++)
        {
            if (PciClassCodeTable[i].BaseClass == classId && PciClassCodeTable[i].SubClass == subclassId &&
                (PciClassCodeTable[i].ProgIf == interfaceId || (PciClassCodeTable[i].ProgIf == 0 && (i + 1 >= PCI_CLASSCODETABLE_LEN ||
                                                                                                     PciClassCodeTable[i+1].BaseClass != PciClassCodeTable[i].BaseClass ||
                                                                                                     PciClassCodeTable[i+1].SubClass != PciClassCodeTable[i].SubClass))))
            {
                textColor(LIGHT_GRAY);
                printf("\n\t\t%s (%s", PciClassCodeTable[i].BaseDesc, PciClassCodeTable[i].SubDesc);
                if (PciClassCodeTable[i].ProgDesc)
                    printf(", %s", PciClassCodeTable[i].ProgDesc);
                printf(")");
                textColor(TEXT);
                break;
            }
        }

        putchar('\n');
    }
}

static void showPCI(void) {
    textColor(TABLE_HEADING);
    printf("\nB:D:F\tIRQ\tDescription\n");
    printf("--------------------------------------------------------------------------------");
    textColor(TEXT);

    parseFolder("PrettyOS/PCI", &showPCI_devices, true);

    textColor(TABLE_HEADING);
    printf("--------------------------------------------------------------------------------\n");
    textColor(TEXT);
}

static void showACPI_intOverrides(const char* name) {
    char buffer[sizeof(">/intoverride/") + strlen(name)];
    strcpy(buffer, ">/intoverride/");
    strcat(buffer, name);
    ipc_setWorkingNode(buffer, false);

    printf("      %s: ", name);
    int64_t value;
    ipc_getInt(">/irq", &value);
    printf("irq=%u, ", (uint32_t)value);
    ipc_getInt(">/apicmapped", &value);
    printf("irq=%u, ", (uint32_t)value);
    ipc_getInt(">/flags", &value);
    printf("flags=%yh\n", (uint8_t)value);

    ipc_setWorkingNode("PrettyOS/ACPI/madt", false);
}

static void showACPI_lapic(const char* name) {
    char buffer[sizeof(">/lapic/") + strlen(name)];
    strcpy(buffer, ">/lapic/");
    strcat(buffer, name);
    ipc_setWorkingNode(buffer, false);

    printf("      %s: ", name);
    int64_t value;
    ipc_getInt(">/id", &value);
    printf("id=%u, ", (uint32_t)value);
    ipc_getInt(">/processor_id", &value);
    printf("processor id=%u\n", (uint32_t)value);

    ipc_setWorkingNode("PrettyOS/ACPI/madt", false);
}

static void showACPI_ioapic(const char* name) {
    char buffer[sizeof(">/ioapic/") + strlen(name)];
    strcpy(buffer, ">/ioapic/");
    strcat(buffer, name);
    ipc_setWorkingNode(buffer, false);

    printf("      %s: ", name);
    int64_t value;
    ipc_getInt(">/id", &value);
    printf("id=%u, ", (uint32_t)value);
    ipc_getInt(">/int_base", &value);
    printf("interrupt base=%u, ", (uint32_t)value);
    ipc_getInt(">/address", &value);
    printf("address=%Xh\n", (uint32_t)value);

    ipc_setWorkingNode("PrettyOS/ACPI/madt", false);
}

static void showACPI(void) {
    textColor(HEADLINE);
    printf("\nACPI tables parsed and stored by PrettyOS:\n\n");
    textColor(TEXT);

    if (ipc_setWorkingNode("PrettyOS/ACPI", false) != IPC_SUCCESSFUL)
    {
        printf("System has no ACPI.");
        return;
    }

    if(ipc_setWorkingNode(">/madt", false) == IPC_SUCCESSFUL)
    {
        printf("  MADT\n");
        textColor(DATA);
        int64_t value = 0;
        ipc_getInt(">/has_dualpic", &value);
        if (value)
            printf("    Dual PIC");
        else
            printf("    Single PIC");

        ipc_getInt(">/lapic_addr", &value);
        printf(", local APIC address: %Xh\n", (uint32_t)value);

        textColor(TEXT);
        printf("    Interrupt overrides:\n");
        textColor(DATA);
        if (!parseFolder(">/intoverride", &showACPI_intOverrides, false))
            printf("     - ");

        textColor(TEXT);
        printf("    Local APICs:\n");
        textColor(DATA);
        if (!parseFolder(">/lapic", &showACPI_lapic, false))
            printf("     - ");

        textColor(TEXT);
        printf("    IO-APICs:\n");
        textColor(DATA);
        if (!parseFolder(">/ioapic", &showACPI_ioapic, false))
            printf("     - ");
    }
}

static bool parseArg(const char* arg) {
    if(strcmp(arg, "--help") == 0 || strcmp(arg, "-?") == 0)
        showHelp();
    else if(strcmp(arg, "--showpci") == 0 || strcmp(arg, "-sp") == 0)
        showPCI();
    else if(strcmp(arg, "--showacpi") == 0)
        showACPI();
    else {
        error("Unknown command.\n\n");
        return false;
    }
    return true;
}

int main(int argc, char* argv[])
{
    printLine("================================================================================", 0, 0x0B);
    printLine("                                 Device Manager",                                  2, 0x0B);
    printLine("--------------------------------------------------------------------------------", 4, 0x0B);

    iSetCursor(0, 6);

    while(argc <= 1) {
        showHelp();
        printf("\nCommand ('exit' to quit): ");
        char cmd[20];
        gets_s(cmd, 20);
        if(strcmp(cmd, "exit") == 0)
            return 0;
        if (parseArg(cmd)) {
            getchar();
            return 0;
        }
    }

    for(size_t i = 1; i < argc; i++) {
        parseArg(argv[i]);
    }

    getchar();
    return (0);
}

/*
* Copyright (c) 2012-2016 The PrettyOS Project. All rights reserved.
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
