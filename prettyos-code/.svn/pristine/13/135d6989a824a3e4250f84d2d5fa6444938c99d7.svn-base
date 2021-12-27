#ifndef MADT_H
#define MADT_H

#include "acpi.h"

#define MADT_LOCAL_APIC                     0
#define MADT_IO_APIC                        1
#define MADT_INTERRUPT_SOURCE_OVERRIDE      2
#define MADT_NMI                            3
#define MADT_LOCAL_APIC_NMI                 4
#define MADT_LOCAL_APIC_ADDRESS_OVERRIDE    5
#define MADT_IO_SAPIC                       6 // Itanium stuff
#define MADT_LOCAL_SAPIC                    7 // Itanium stuff again
#define MADT_PLATFORM_INTERRUPT_SOURCES     8 // Even more Itanium
#define MADT_LOCAL_X2APIC                   9
#define MADT_IO_X2APIC_NMI                 10
#define MADT_GIC                           11 // ARM stuff
#define MADT_GICD                          12 // More ARM stuff

// 0b0000 0000 0000 TTPP
#define MADT_INTSOURCE_FLAGS_POL_MASK       3 // PP
#define MADT_INTSOURCE_FLAGS_POL_BUS        0
#define MADT_INTSOURCE_FLAGS_POL_HIGH       1
#define MADT_INTSOURCE_FLAGS_POL_LOW        3
#define MADT_INTSOURCE_FLAGS_TRIG_MASK     12 // TT
#define MADT_INTSOURCE_FLAGS_TRIG_BUS      (0 << 2)
#define MADT_INTSOURCE_FLAGS_TRIG_EDGE     (1 << 2)
#define MADT_INTSOURCE_FLAGS_TRIG_LEVEL    (3 << 2)

void acpi_parser_madt(acpi_header_t* table);

extern const char* const madtPath;

#endif
