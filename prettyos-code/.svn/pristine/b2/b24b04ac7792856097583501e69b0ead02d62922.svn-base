#ifndef CDI_BIOS_H
#define CDI_BIOS_H

#include "util/types.h"
#include <cdi/lists.h>

struct cdi_bios_registers {
    uint16_t ax;
    uint16_t bx;
    uint16_t cx;
    uint16_t dx;
    uint16_t si;
    uint16_t di;
    uint16_t ds;
    uint16_t es;
};

// Structure to access memory areas of a 16-bit Process
struct cdi_bios_memory {
    uintptr_t dest; // Virtual adress inside the memory of the 16bit-Processes. Has to be lower than 0xC0000.

    // Pointer to reserved memory for the data of this memory area. Will be used at start to initialize the area and contains the data after BIOS call.
    void *src;

    uint16_t size; // Length of memory area
};

/* Calls the BIOS-Interrupt 0x10
   registers: Pointer to a Register-Struct. It will be load to the tasks registers at call time and contains the values of the task after it has been finished.
   memory:    Memory area copied to BIOS task and back after the task is finished. The list has the type struct cdi_bios_memory.
   return:    0, in case of successful call, -1 otherwise */
int cdi_bios_int10(struct cdi_bios_registers *registers, cdi_list_t memory);

#endif
