#ifndef PARSER_H
#define PARSER_H

#include "acpi.h"

typedef struct
{
    const char* signature;
    void(*parser)(acpi_header_t*);
} acpi_parser_t;

// Hardcoded parsers to be loaded on initializing ACPI
extern acpi_parser_t acpi_baseParsers[];

extern const char* const acpi_parserBasePath;

#endif
