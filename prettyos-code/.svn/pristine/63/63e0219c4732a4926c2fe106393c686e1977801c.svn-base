#ifndef CDI_MEM_H
#define CDI_MEM_H

#include "util/types.h"
#include "cdi.h"

// Describes requirements for a memory area
typedef enum {
    // Mask for a bit field which describes the required physical alignment (the required alignment is 2^x bytes)
    CDI_MEM_ALIGN_MASK      = 0x1f,

    // Physical address isn't required to be valid
    CDI_MEM_VIRT_ONLY       = 0x20,

    // Request physically contiguous memory
    CDI_MEM_PHYS_CONTIGUOUS = 0x40,

    // Request physical memory < 16 MB
    CDI_MEM_DMA_16M         = 0x80,

    // Request physical memory < 4 GB
    CDI_MEM_DMA_4G          = 0x100,

    // Memory doesn't need to be initialised (avoids copy in cdi_mem_require_flags)
    CDI_MEM_NOINIT          = 0x200,
} cdi_mem_flags_t;

// Describes an area of physical memory (an entry for a scatter/gather list)
struct cdi_mem_sg_item {
    uintptr_t   start;
    size_t      size;
};

// Scatter/Gather list which contains the physical addresses for a given area of virtual memory.
struct cdi_mem_sg_list {
    size_t                  num;
    struct cdi_mem_sg_item* items;
};

// Describes an area of virtual memory, depending on the allocation flags with a scatter/gather list of its physical addresses.
struct cdi_mem_area {
    size_t                  size;
    void*                   vaddr;
    struct cdi_mem_sg_list  paddr;

    cdi_mem_osdep           osdep;
};

/* Allocates a memory area.
   size:   Size of the memory area in bytes
   flags:  Flags that describe additional requirements
   return: A cdi_mem_area on success, NULL on failure */
struct cdi_mem_area* cdi_mem_alloc(size_t size, cdi_mem_flags_t flags);

/* Reserves physically contiguous memory at a defined address
   paddr:  Physical address of the requested memory aread
   size:   Size of the requested memory area in bytes
   return: A cdi_mem_area on success, NULL on failure */
struct cdi_mem_area* cdi_mem_map(uintptr_t paddr, size_t size);

// Frees a memory area that was previously allocated by cdi_mem_alloc or cdi_mem_map
void cdi_mem_free(struct cdi_mem_area* area);

/* Returns a memory area that describes the same data as p does, but for which at least the given flags are set.
   This function may use the same virtual and physical memory areas as used in p, or it may even return p itself. In this case it must ensure that
   the commonly used memory is only freed when both p and the return value of this function have been freed by cdi_mem_free.
   Otherwise, a new memory area is allocated and data is copied from p into the newly allocated memory (unless CDI_MEM_NOINIT is set). */
struct cdi_mem_area* cdi_mem_require_flags(struct cdi_mem_area* p, cdi_mem_flags_t flags);

/* Copies data from src to dest. Both memory areas must be of the samesize.
   This does not necessarily involve a physical copy: If both memory areas point to the same physical memory, this function does nothing. It can also
   use other methods to achieve the same visible effect, e.g. by remapping pages.
   return: 0 on success, -1 otherwise */
int cdi_mem_copy(struct cdi_mem_area* dest, struct cdi_mem_area* src);

#endif
