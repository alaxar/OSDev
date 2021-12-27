#ifndef PAGING_H
#define PAGING_H

#include "util/util.h"
#include "tasking/synchronisation.h"

#define PAGESIZE   0x1000 // Size of one page in bytes
#define PAGE_COUNT 1024   // Number of pages per page table
#define PT_COUNT   1024   // Number of page tables per page directory

#define CR0_PAGINGENABLED    BIT(31)
#define CR0_WRITEPROTECT     BIT(16);

#define IA32_MTRRCAP 0xFE
#define IA32_MTRR_DEF_TYPE 0x2FF
#define IA32_MTRR_FIX64K_00000 0x250
#define IA32_MTRR_FIX16K_80000 0x258
#define IA32_MTRR_FIX4K_C0000 0x268
#define IA32_MTRR_PHYSBASE0 0x200
#define IA32_MTRR_PHYSMASK0 0x201

typedef enum
{
    MEM_PRESENT      = BIT(0),
    MEM_WRITE        = BIT(1),
    MEM_USER         = BIT(2),
    MEM_WRITETHROUGH = BIT(3),
    MEM_NOCACHE      = BIT(4),
    MEM_NOTLBUPDATE  = BIT(8),
    MEM_ALLOCATED    = BIT(9), // PrettyOS-specific. Indicates if a page is allocated or not
    MEM_CONTINUOUS   = BIT(10) // PrettyOS-specific. Used as function argument to request physically continuous memory
} MEMFLAGS_t;

typedef enum {
    MTRR_UNCACHABLE     = 0x00,
    MTRR_WRITECOMBINING = 0x01,
    MTRR_WRITETHROUGH   = 0x04,
    MTRR_WRITEPROTECTED = 0x05,
    MTRR_WRITEBACK      = 0x06
} MTRR_CACHETYPE;

// Memory Map
typedef struct
{
    uint32_t mysize; // Size of this entry
    uint64_t base;   // The region's address
    uint64_t size;   // The region's size
    uint32_t type;   // Is "1" for "free"
} __attribute__((packed)) memoryMapEntry_t;

// Paging
typedef struct
{
    uint32_t pages[PAGE_COUNT];
} __attribute__((packed)) pageTable_t;

typedef struct
{
    uint32_t     codes[PT_COUNT];
    pageTable_t* tables[PT_COUNT];
    uint32_t     physAddr;
    mutex_t      mutex;
} __attribute__((packed)) pageDirectory_t;


extern pageDirectory_t* kernelPageDirectory;


uint32_t paging_install(memoryMapEntry_t* memoryMapBegin, size_t memoryMapLength);

// Management of physical memory
uintptr_t paging_allocPhysMem(size_t pages, bool allowMove);
bool      paging_allocPhysAddr(uintptr_t addr, size_t pages);
void      paging_freePhysMem(uintptr_t addr, size_t pages);
void      paging_setPhysMemCachingBehaviour(uintptr_t start, size_t pages, MTRR_CACHETYPE);

// Management of virtual memory
void* paging_allocVirtMem(pageDirectory_t* pd, size_t pages);
bool  paging_allocVirtAddr(pageDirectory_t* pd, void* addr, size_t pages);
void  paging_freeVirtMem(pageDirectory_t* pd, void* addr, size_t pages);

// Linkage betweeen physical and virtual memory
bool      paging_mapVirtToPhysAddr(pageDirectory_t* pd, void* vaddr, uintptr_t paddr, size_t pages, MEMFLAGS_t flags);
void      paging_setFlags(pageDirectory_t* pd, void* addr, uint32_t size, MEMFLAGS_t flags);
uintptr_t paging_getPhysAddr(const void* virtAddress);

// High level functions
bool  paging_alloc(pageDirectory_t* pd, void* addr, uint32_t size, MEMFLAGS_t flags);
bool  paging_allocNew(pageDirectory_t* pd, void* addr, uint32_t size, MEMFLAGS_t flags);
void  paging_free(pageDirectory_t* pd, void* addr, uint32_t size);
void* paging_allocMMIO(uintptr_t paddr, size_t pages);

// Page directory management
pageDirectory_t* paging_createPageDirectory(void);
void             paging_destroyPageDirectory(pageDirectory_t* pd);
void             paging_switch(pageDirectory_t* pd);

// Debugging
void* paging_getVirtAddr(uintptr_t physAddress); // Slow - only for debugging purposes.
void  paging_analyzeBitTable(void);


#endif
