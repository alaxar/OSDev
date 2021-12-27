#ifndef ACPI_H
#define ACPI_H

#include "os.h"
#include "util/types.h"
#include "util/util.h"

#ifdef _ACPI_DIAGNOSIS_
#define ACPIASSERT(x) ASSERT(x)
#else
#define ACPIASSERT(x)
#endif

#define ACPI_TABLE_SIG_LEN 4 // Generic table signature length
#define ACPI_OEMID_LEN     6 // OemID length
#define ACPI_OEMTBLID_LEN  8 // Oem table ID length

// Generic header of every table
typedef struct
{
    char signature[ACPI_TABLE_SIG_LEN]; // Table-specific ASCII-Signature (e.g. 'APIC')
    uint32_t len; // Size of the whole table including the header
    uint8_t rev;
    uint8_t checksum; // Whole table must add up to 0 to be valid
    char oemID[ACPI_OEMID_LEN];
    char oemTableID[ACPI_OEMTBLID_LEN];
    uint32_t oemRev;
    uint32_t creatorId;
    uint32_t creatorRev;
} __attribute__((packed)) acpi_header_t; // Generic header for every table


bool acpi_available(void);
void acpi_install(void);
bool acpi_registerParser(const char* signature, void(*parser)(acpi_header_t* table));
bool acpi_unregisterParser(const char* signature, void(*parser)(acpi_header_t*));
void acpi_reload(void); // Reloads the ACPI completely including re-running the RSDT parser
void acpi_reparse(void); // Re-runs the parsers for the sub-tables only
void acpi_parseSpecific(const char* signtature); // Re-Parses a specific table by signature

#endif
