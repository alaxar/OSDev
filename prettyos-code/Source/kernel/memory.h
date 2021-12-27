#ifndef MEMORY_H
#define MEMORY_H

#include "os.h"

/******************************************************************************
*                                                                             *
*                          K E R N E L   A R E A                              *
*                                                                             *
******************************************************************************/

// Kernel is located at 0x100000 // 1 MiB  // cf. kernel.ld

#define IDMAP   3              // 0 MiB - 12 MiB (4 KiB * 1024 * 3), identity mapping

// Placement allocation
#define PLACEMENT_BEGIN   0x600000   // 6 MiB
#define PLACEMENT_END     0xC00000   // 12 MiB

// memory location for MMIO of devices (networking card, EHCI, grafics card, ...)
#define PCI_MEM_START     0xC0000000 // 3 GiB
#define PCI_MEM_END       0xE0000000 // 3,5 GiB

// Where the kernel's private data (heap) is stored (virtual addresses)
#define KERNEL_HEAP_START 0xE0000000 // 3,5 GiB
#define KERNEL_HEAP_END   0xFFFFFFFF // 4 GiB - 1


/******************************************************************************
*                                                                             *
*                          U S E R    A R E A                                 *
*                                                                             *
******************************************************************************/

// User program starts at 0x1400000 // 20 MiB  // cf. user.ld

// User Stack
#define USER_STACK        0x1500000  // nearly 20 MiB

// Area to move data from kernel to userprogram (for example, parameter lists)
#define USER_DATA_BUFFER  USER_STACK // 64 KiB

// User Heap management
#define USER_HEAP_START ((uint8_t*)(USER_DATA_BUFFER  + 0x10000))   // 21 MiB plus 64 KiB
#define USER_HEAP_END   ((uint8_t*)(KERNEL_HEAP_START - 0x1000000)) //  3 GiB minus 16 MiB


#endif
