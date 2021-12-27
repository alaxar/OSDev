/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "paging.h"
#include "memory.h"
#include "util/util.h"
#include "timer.h"
#include "tasking/task.h"
#include "kheap.h"
#include "ipc.h"
#include "video/console.h"
#include "serial.h"
#include "cpu.h"

#define FOUR_GB 0x100000000ull // Highest address + 1


pageDirectory_t* kernelPageDirectory;
pageDirectory_t* currentPageDirectory = 0;

extern char _kernel_beg, _kernel_end; // defined in linker script
extern char _ro_start, _ro_end;       // defined in linker script

static uint32_t  MAX_DWORDS = 0;        // Can be FOUR_GB / PAGESIZE / 32 at maximum;
static uint32_t* phys_reservationTable; // 0: free;      1: used
static uint32_t* phys_blockingTable;    // 0: unblocked; 1: blocked (not movable)
static mutex_t   phys_mutex = mutex_init();
static uint32_t  firstFreeDWORD = PLACEMENT_END / PAGESIZE / 32; // Exclude the first 12 MiB from being allocated (needed for DMA later on)


static uint32_t physMemInit(memoryMapEntry_t* memoryMapBegin, size_t memoryMapLength);


uint32_t paging_install(memoryMapEntry_t* memoryMapBegin, size_t memoryMapLength)
{
    uint32_t ram_available = physMemInit(memoryMapBegin, memoryMapLength);

    // Setup the kernel page directory
    kernelPageDirectory = malloc(sizeof(pageDirectory_t), PAGESIZE, "pag-kernelPD");
    memset(kernelPageDirectory, 0, sizeof(pageDirectory_t) - 4);
    kernelPageDirectory->physAddr = (uintptr_t)kernelPageDirectory;
    mutex_construct(&kernelPageDirectory->mutex);

  #ifdef _DIAGNOSIS_
    printf("\nkernelPageDirectory (virt., phys.): %Xh, %Xh\n", kernelPageDirectory, kernelPageDirectory->physAddr);
  #endif

    // Setup the page tables for 0 MiB - 12 MiB, identity mapping
    uintptr_t addr = 0;
    for (uint8_t i=0; i<IDMAP; i++)
    {
        // Page directory entry, virt=phys due to placement allocation in id-mapped area
        kernelPageDirectory->tables[i] = malloc(sizeof(pageTable_t), PAGESIZE, "pag-kernelPT");
        kernelPageDirectory->codes[i]  = (uint32_t)kernelPageDirectory->tables[i] | MEM_PRESENT | MEM_WRITE;

        // Page table entries, identity mapping
        for (uint32_t j = 0; j < PAGE_COUNT; ++j)
        {
            uint32_t flags;
            if (addr < 0x100000) // The first MiB has to be flushed from TLB, since VM86 adds MEM_USER and breaks identity mapping
                flags = MEM_PRESENT | MEM_WRITE | MEM_ALLOCATED;
            else
                flags = MEM_PRESENT | MEM_WRITE | MEM_NOTLBUPDATE | MEM_ALLOCATED;
            kernelPageDirectory->tables[i]->pages[j] = addr | flags;
            addr += PAGESIZE;
        }
    }
    kernelPageDirectory->tables[0]->pages[0] = 0 | MEM_PRESENT | MEM_ALLOCATED; // Make first page read-only

    // Setup the page tables for kernel heap (3,5 - 4 GiB), unmapped
    size_t kernelpts = min(PT_COUNT / 8, ram_available / PAGESIZE / PAGE_COUNT); // Do not allocate more PTs than necessary (limited by available memory)
    pageTable_t* heap_pts = malloc(kernelpts*sizeof(pageTable_t), PAGESIZE, "kheap_pts");
    memset(heap_pts, 0, kernelpts * sizeof(pageTable_t));
    for (uint32_t i = 0; i < kernelpts; i++)
    {
        kernelPageDirectory->tables[KERNEL_HEAP_START/PAGESIZE/PAGE_COUNT + i] = heap_pts + i;
        kernelPageDirectory->codes [KERNEL_HEAP_START/PAGESIZE/PAGE_COUNT + i] = (uint32_t)(heap_pts + i) | MEM_PRESENT;
    }

    // Make some parts of the kernel (Sections text and rodata) read-only
    uint32_t startpt = ((uintptr_t)&_ro_start)/PAGESIZE/PAGE_COUNT; // Page table, where read-only section starts
    uint32_t startp = ((uintptr_t)&_ro_start)/PAGESIZE%PAGE_COUNT; // Page, where read-only section starts
    if ((uintptr_t)&_ro_start%PAGESIZE != 0)
    {
        startp++;
        if (startp > PAGE_COUNT)
        {
            startpt++;
            startp = 0;
        }
    }
    uint32_t endpt = ((uintptr_t)&_ro_end)/PAGESIZE/PAGE_COUNT; // Page table, where read-only section ends
    uint32_t endp = ((uintptr_t)&_ro_end)/PAGESIZE%PAGE_COUNT; // Page, where read-only section ends
    if (endp > 0)
        endp--;
    else
    {
        endp = PAGE_COUNT-1;
        endpt--;
    }
    for (uint32_t i = startpt; i <= endpt; i++)
    {
        for (uint32_t j = startp; j <= endp; j++)
        {
            kernelPageDirectory->tables[i]->pages[j] &= (~MEM_WRITE); // Forbid writing
        }
    }

    // Tell CPU to enable paging
    paging_switch(kernelPageDirectory);
    uint32_t cr0;
    __asm__("mov %%cr0, %0": "=r"(cr0)); // read CR0
    cr0 |= CR0_PAGINGENABLED | CR0_WRITEPROTECT;
    __asm__("mov %0, %%cr0":: "r"(cr0)); // write CR0

    return (ram_available);
}


static bool isMemoryMapAvailable(const memoryMapEntry_t* memoryMapBegin, const memoryMapEntry_t* memoryMapEnd, uint64_t beg, uint64_t end)
{
    uint64_t covered = beg;
    for (const memoryMapEntry_t* outerLoop=memoryMapBegin; outerLoop < memoryMapEnd; outerLoop = (memoryMapEntry_t*)((char*)outerLoop + outerLoop->mysize + 4))
    {
        // There must not be an "reserved" entry which reaches into the specified area
        if ((outerLoop->type != 1) && (outerLoop->base < end) && ((outerLoop->base + outerLoop->size) > beg))
        {
            return (false);
        }
        // Check whether the "free" entries cover the whole specified area.
        for (const memoryMapEntry_t* entry=memoryMapBegin; entry < memoryMapEnd; entry = (memoryMapEntry_t*)((char*)entry + entry->mysize + 4))
        {
            if (entry->base <= covered && (entry->base + entry->size) > covered)
            {
                covered = entry->base + entry->size;
            }
        }
    }

    // Return whether the whole area is covered by "free" entries
    return (covered >= end);
}

static void physSetBits(uint32_t addrBegin, uint32_t addrEnd, bool reserved)
{
    // Calculate the bit-numbers
    uint32_t start = alignUp  (addrBegin, PAGESIZE) / PAGESIZE;
    uint32_t end   = alignDown(addrEnd,   PAGESIZE) / PAGESIZE;

    // Set all these bits
    for (uint32_t j=start; j<end; ++j)
    {
        if (reserved)
            SET_BIT(phys_reservationTable[j/32], j%32);
        else
            CLEAR_BIT(phys_reservationTable[j/32], j%32);
    }
}

static uint32_t physMemInit(memoryMapEntry_t* memoryMapBegin, size_t memoryMapLength)
{
    memoryMapEntry_t* memoryMapEnd = (memoryMapEntry_t*)((char*)memoryMapBegin + memoryMapLength);
  #ifdef _PAGING_DIAGNOSIS_
    textColor(HEADLINE);
    printf("\nMemory map (%X -> %X):", memoryMapBegin, memoryMapEnd);
    textColor(TEXT);
  #endif

    // Prepare the memory map entries, since we work with max 4 GB only. The last entry in the entry-array has size 0.
    for (memoryMapEntry_t* entry = memoryMapBegin; entry < memoryMapEnd; entry = (memoryMapEntry_t*)((char*)entry + entry->mysize + 4))
    {
      #ifdef _PAGING_DIAGNOSIS_
        printf("\n  %Xh -> %Xh %u (%u Bytes)", (uint32_t)entry->base, (uint32_t)(entry->base + entry->size), entry->type, (uint32_t)entry->size); // Print the memory map
      #endif

        // We will completely ignore memory above 4 GB or with size of 0, move following entries backward by one then
        if (entry->base < FOUR_GB && entry->size != 0)
        {
            // Eventually reduce the size so the the block doesn't exceed 4 GB
            if (entry->base + entry->size >= FOUR_GB)
            {
                entry->size = FOUR_GB - entry->base;
            }

            if (entry->type == 1)
                MAX_DWORDS = max(MAX_DWORDS, (entry->base+entry->size)/PAGESIZE/32); // Calculate required size of bittables
        }
    }

    // Check that 6 MiB-12 MiB is free for use
    if (!isMemoryMapAvailable(memoryMapBegin, memoryMapEnd, PLACEMENT_BEGIN, IDMAP*PAGE_COUNT*PAGESIZE))
    {
        printfe("The memory between 6 MiB and 12 MiB is not free for use. OS halted!\n");
        cli();
        hlt();
    }

    // We store our data here, initialize all bits to "reserved"
    phys_reservationTable = malloc(MAX_DWORDS * 4, 0, "phys_reservationTable");
    memset(phys_reservationTable, 0xFF, MAX_DWORDS * 4);
    phys_blockingTable = malloc(MAX_DWORDS * 4, 0, "phys_blockingTable");

    // Set the bitmap bits according to the memory map now. "type==1" means "free".
    for (const memoryMapEntry_t* entry = memoryMapBegin; entry < memoryMapEnd; entry = (memoryMapEntry_t*)((char*)entry + entry->mysize + 4))
    {
        if (entry->type == 1 && entry->base < FOUR_GB) // Set bits to "free"
        {
            physSetBits(entry->base, entry->base+entry->size, false);
        }
    }

    // Find the number of dwords we can use, skipping the last, "reserved"-only ones
    uint32_t dwordCount = 0;

    for (uint32_t i=0; i<MAX_DWORDS; i++)
    {
        if (phys_reservationTable[i] != 0xFFFFFFFF)
        {
            dwordCount = i + 1;
        }
    }

    // Reserve first 12 MiB
    physSetBits(0x00000000, PLACEMENT_END-1, true);

    // Reserve the region of the kernel code
    if ((uintptr_t)&_kernel_end >= PLACEMENT_END)
        physSetBits((uint32_t)&_kernel_beg, (uint32_t)&_kernel_end, true);

  #ifdef _DIAGNOSIS_
    printf("Highest available RAM: %Xh\n", dwordCount * 32 * PAGESIZE);
  #endif

    // Return the amount of memory available (or rather the highest address)
    return (dwordCount * 32 * PAGESIZE);
}

// ------------------------------------------------------------------------------
// Helper functions and inline assembly
// ------------------------------------------------------------------------------

void paging_switch(pageDirectory_t* pd)
{
    if (pd != currentPageDirectory) // Switch page directory only if the new one is different from the old one
    {
      #ifdef _PAGING_DIAGNOSIS_
        textColor(MAGENTA);
        printf("\nDEBUG: paging_switch: pd=%X, pd->physAddr=%X", pd, pd->physAddr);
        textColor(TEXT);
      #endif
        currentPageDirectory = pd;
        __asm__ volatile("mov %0, %%cr3" : : "r"(pd->physAddr));
    }
}

static inline void invalidateTLBEntry(void* p)
{
    __asm__ volatile("invlpg (%0)" ::"r" (p) : "memory");
}

static inline uint32_t getFirstFreeBit(uint32_t value)
{
    // Find the number of first free bit.
    // This inline assembler instruction is smaller and faster than a C loop to identify this bit
    uint32_t bitnr;
    __asm__("bsfl %1, %0" : "=r"(bitnr) : "r"(value));
    return bitnr;
}

static bool changeAllowed(pageDirectory_t* pd, size_t i)
{
    return kernelPageDirectory == pd || (pd->tables[i] != kernelPageDirectory->tables[i]);
}

// ------------------------------------------------------------------------------
// Allocates a continuous area of pyhsical memory of a given number of pages. The
// function decides about the physical address. The physical address might change
// internally if the same memory is requested by paging_allocPhysAddr, as long as
// allowMove is true.
// ------------------------------------------------------------------------------
uintptr_t paging_allocPhysMem(size_t pages, bool allowMove)
{
    mutex_lock(&phys_mutex);

    // Search for a continuous block of free pages
    uint32_t dword = firstFreeDWORD;
    for (; dword < MAX_DWORDS; dword++)
    {
        if (phys_reservationTable[dword] != 0xFFFFFFFF)
        {
            bool aquirable = true;
            uint32_t bitnr = getFirstFreeBit(~phys_reservationTable[dword]) + dword * 32;
            uint32_t bitnr_old = bitnr;
            // Check if here is enough space
            for (size_t i = 0; i < pages && aquirable; i++) {
                aquirable = !(phys_reservationTable[bitnr / 32] & BIT(bitnr % 32));
                bitnr++;
            }
            if (!aquirable)
                continue; // Not enough space - continue searching

            bitnr = bitnr_old;
            for (size_t i = 0; i < pages; i++) {
                SET_BIT(phys_reservationTable[bitnr / 32], bitnr % 32); // Reserve memory
                if (allowMove)
                    CLEAR_BIT(phys_blockingTable[bitnr / 32], bitnr % 32); // Don't protect against moving
                else
                    SET_BIT(phys_blockingTable[bitnr / 32], bitnr % 32); // Protect against moving
                bitnr++;
            }

            mutex_unlock(&phys_mutex);
            return (bitnr_old * PAGESIZE);
        }
    }

    // No free page found
    mutex_unlock(&phys_mutex);
    return (0);
}

// ------------------------------------------------------------------------------
// Allocates an area of pyhsical memory of a given number of pages. The address
// is given. If the requested memory is in use, it will be moved away if possible.
// ------------------------------------------------------------------------------
bool paging_allocPhysAddr(uintptr_t addr, size_t pages)
{
    // look for addr
    uint32_t bitnr = addr / PAGESIZE;
    bool aquirable = true;
    mutex_lock(&phys_mutex);
    for (size_t i = 0; i < pages && aquirable; i++) {
        aquirable = !(phys_reservationTable[bitnr / 32] & BIT(bitnr % 32));
        // Later: aquirable = !(phys_reservationTable[bitnr / 32] & BIT(bitnr % 32)) || !(phys_blockingTable[bitnr / 32] & BIT(bitnr % 32));
        // The memory in question could be moved to a different page (if allowed)
        bitnr++;
    }

    if (!aquirable)
    {
        mutex_unlock(&phys_mutex);
        return false;
    }

    // allocate
    bitnr = addr / PAGESIZE;
    for (size_t i = 0; i < pages; i++) {
        // TODO: Move old content before writing, if (phys_reservationTable[bitnr / 32] & BIT(bitnr % 32)).
        SET_BIT(phys_reservationTable[bitnr / 32], bitnr % 32); // Reserve memory
        SET_BIT(phys_blockingTable[firstFreeDWORD], bitnr % 32); // Protect against moving
        bitnr++;
    }
    mutex_unlock(&phys_mutex);
    return true;
}

// ------------------------------------------------------------------------------
// Frees physical memory allocated by paging_allocPhysAddr or paging_allocPhysMem.
// ------------------------------------------------------------------------------
void paging_freePhysMem(uintptr_t addr, size_t pages)
{
    // Calculate the number of the bit
    uint32_t bitnr = addr / PAGESIZE;

    mutex_lock(&phys_mutex);
    // Maybe the affected dword (which has a free bit now) is ahead of firstFreeDWORD?
    if (bitnr / 32 < firstFreeDWORD)
    {
        firstFreeDWORD = bitnr / 32;
    }

    // Set the page to "free"
    CLEAR_BIT(phys_reservationTable[bitnr / 32], bitnr % 32);
    if (pages > 1)
        paging_freePhysMem(addr + PAGESIZE, pages - 1);
    mutex_unlock(&phys_mutex);
}

// ------------------------------------------------------------------------------
// Access MTRRs (if available) to optimize caching behaviour
// ------------------------------------------------------------------------------
static void setFixedMTRR(uint32_t firstMSR, uint16_t counter, size_t pages, MTRR_CACHETYPE behaviour)
{
    uint64_t msrValue = cpu_MSRread(firstMSR + counter / 8);
  #ifdef _PAGING_DIAGNOSIS_
    printf("\nReplace MSR %x: %X%X ", firstMSR + counter / 8, (uint32_t)(msrValue >> 32), (uint32_t)msrValue);
  #endif
    for (size_t i = 0; i < pages; i++)
    {
        msrValue &= ~((uint64_t)0xFF << (counter % 8) * 8); // clear existing behaviour
        msrValue |= ((uint64_t)behaviour) << ((counter % 8) * 8);// and replace it by desired behaviour
        if ((counter+1) % 8 == 0 || i == pages-1)
        {
          #ifdef _PAGING_DIAGNOSIS_
            printf("by %X%X", (uint32_t)(msrValue >> 32), (uint32_t)msrValue);
          #endif
            cpu_MSRwrite(firstMSR + counter / 8, msrValue);
            if (i != pages - 1)
            {
                msrValue = cpu_MSRread(firstMSR + (counter+1) / 8);
              #ifdef _PAGING_DIAGNOSIS_
                printf("\nReplace MSR %x: %X%X ", firstMSR + (counter+1) / 8, (uint32_t)(msrValue >> 32), (uint32_t)msrValue);
              #endif
            }
        }
        counter++;
    }
}

void paging_setPhysMemCachingBehaviour(uintptr_t start, size_t pages, MTRR_CACHETYPE behaviour)
{
    if (!cpu_supports(CF_MTRR))
        return;

    static uint64_t mtrr_feature = 0;
    if (mtrr_feature == 0)
        mtrr_feature = cpu_MSRread(IA32_MTRRCAP);
    uint64_t mtrr_defType = cpu_MSRread(IA32_MTRR_DEF_TYPE);

  #ifdef _PAGING_DIAGNOSIS_
    printf("\nMTRR feature: %X, MTRR defType: %X", (uint32_t)mtrr_feature, (uint32_t)mtrr_defType);
  #endif

    if (!(mtrr_defType & BIT(11)))
    {
      #ifdef _PAGING_DIAGNOSIS_
        printf("\nMTRRs disabled");
        return;
      #endif
    }

    if (behaviour == MTRR_WRITECOMBINING && !(mtrr_feature & BIT(10)))
        behaviour = MTRR_WRITETHROUGH; // Fall-back to Write-Through caching

    uint32_t cr0, cr4;
    // Disable caching before touching MTRRs
    __asm__ volatile(
        "cli;"
        "mov %%cr0, %%eax;"
        "mov %%eax, %0;"          // Store CR0
        "or  $0x40000000, %%eax;" // disable cache
        "and $0xDFFFFFFF, %%eax;" // disable enforced Write-Through cache
        "mov %%eax, %%cr0;"
        "mov %%cr4, %%eax;"
        "mov %%eax, %1;"          // Store CR4
        "and $0xFFFFFF7F, %%eax;" // deactivate PGE
        "mov %%eax, %%cr4;"
        "mov %%cr3, %%eax;"       // Flush TLB
        "mov %%eax, %%cr3;" : "=r"(cr0), "=r"(cr4) : : "eax");

    // Use fixed MTRRs for low adresses
    // TODO: Make it more flexible to support memory that spans over several (fixed) MTRR length types
    if (mtrr_feature & BIT(8) && mtrr_defType & BIT(10) && start <= 0xFFFFF)
    {
      #ifdef _PAGING_DIAGNOSIS_
        printf("\nUsing fixed MTRR");
      #endif
        if (start < 0x7FFFF && pages%128 == 0)
            setFixedMTRR(IA32_MTRR_FIX64K_00000, start / 0x10000, pages / 16, behaviour);
        else if (start > 0x80000 && start < 0xBFFFF && pages%4 == 0)
            setFixedMTRR(IA32_MTRR_FIX16K_80000, (start-0x80000) / 0x4000, pages / 4, behaviour);
        else if (start > 0xC0000 && start < 0xFFFFF)
            setFixedMTRR(IA32_MTRR_FIX4K_C0000, (start-0xC0000) / 0x1000, pages, behaviour);

        goto end;
    }

  #ifdef _PAGING_DIAGNOSIS_
    printf("\nUsing variable MTRR");
    for (int i = 0; i < BYTE1(mtrr_feature); i++)
    {
        uint64_t physbase = cpu_MSRread(IA32_MTRR_PHYSBASE0 + 2 * i);
        uint64_t physmask = cpu_MSRread(IA32_MTRR_PHYSMASK0 + 2 * i);
        printf("\nMTRR %u: %X %X, %X %X", i, (uint32_t)(physbase >> 32), (uint32_t)physbase, (uint32_t)(physmask >> 32), (uint32_t)physmask);
    }
  #endif

    pages = 0x1 << bsr(pages); // align down page count to power of 2
    // Use variable MTRRs for high adresses or in case fixed MTRRs are unsupported
    for (int i = BYTE1(mtrr_feature)-1; i > 0; i--)
    {
        uint64_t physmask = cpu_MSRread(IA32_MTRR_PHYSMASK0 + 2 * i);
        if (!(physmask & BIT(11))) // Free MTRR
        {
            uint64_t physbase = (start & 0xFFFFF000) | behaviour;
            physmask = BIT(11) | (~(pages*PAGESIZE - 1) & 0xFFFFF000);
            cpu_MSRwrite(IA32_MTRR_PHYSBASE0 + 2 * i, physbase);
            cpu_MSRwrite(IA32_MTRR_PHYSMASK0 + 2 * i, physmask);
          #ifdef _PAGING_DIAGNOSIS_
            printf("\nWriting:");
            printf("\nMTRR %u: %X %X, %X %X", i, (uint32_t)(physbase >> 32), (uint32_t)physbase, (uint32_t)(physmask >> 32), (uint32_t)physmask);
          #endif
            break;
        }
    }

end:
    // Enable caching again
    __asm__ volatile(
        "mov %%eax, %%cr0;"
        "mov %%ebx, %%cr4;"
        "sti": : "a"(cr0), "b"(cr4) : "ecx");
}

// ------------------------------------------------------------------------------
// Allocates an area of virtual memory of a given number of pages in the given
// page directory. The function decides about the address, which is returned.
// ------------------------------------------------------------------------------
void* paging_allocVirtMem(pageDirectory_t* pd, size_t pages)
{
    return 0; // TODO
}

// ------------------------------------------------------------------------------
// Allocates an area of virtual memory of a given number of pages in the given
// page directory. The address is given.
// ------------------------------------------------------------------------------
bool paging_allocVirtAddr(pageDirectory_t* pd, void* addr, size_t pages)
{
    // "addr" must be page-aligned
    ASSERT(((uintptr_t)addr) % PAGESIZE == 0);

    // We repeat allocating one page at once
    for (uint32_t done = 0; done < pages; done++)
    {
        uint32_t pagenr = (uintptr_t)addr / PAGESIZE + done;

        mutex_lock(&pd->mutex);
        // Get the page table
        pageTable_t* pt = pd->tables[pagenr / PAGE_COUNT];
        if (!pt)
        {
            // Allocate the page table
            pt = malloc(sizeof(pageTable_t), PAGESIZE, "pageTable");
            if (!pt)
            {
                // Undo all allocations and return an error
                paging_freeVirtMem(pd, addr, done*PAGESIZE);
                mutex_unlock(&pd->mutex);
                return false;
            }
            memset(pt, 0, sizeof(pageTable_t));
            pd->tables[pagenr / PAGE_COUNT] = pt;

            // Set physical address and flags
            pd->codes[pagenr / PAGE_COUNT] = paging_getPhysAddr(pt) | MEM_PRESENT | MEM_WRITE;
        }
        else
        {
            // Maybe there is already memory allocated?
            if (pt->pages[pagenr%PAGE_COUNT] & MEM_ALLOCATED)
            {
                printfe("Page already allocated: %u\n", pagenr);
                paging_freeVirtMem(pd, addr, done*PAGESIZE);
                mutex_unlock(&pd->mutex);
                return false;
            }
            ASSERT(changeAllowed(pd, pagenr / PAGE_COUNT));
        }

        // Setup the page
        pt->pages[pagenr%PAGE_COUNT] = MEM_ALLOCATED;
        mutex_unlock(&pd->mutex);
    }
    return true;
}

// ------------------------------------------------------------------------------
// Frees virtual memory allocated by paging_allocVirtMem or paging_allocVirtAddr.
// ------------------------------------------------------------------------------
void paging_freeVirtMem(pageDirectory_t* pd, void* addr, size_t pages)
{
    // "addr" must be page-aligned
    ASSERT(((uintptr_t)addr) % PAGESIZE == 0);

    for (uint32_t pg = 0; pg < pages; ++pg)
    {
        uint32_t pagenr = (uintptr_t)addr / PAGESIZE + pg;

        mutex_lock(&pd->mutex);
        ASSERT(pd->tables[pagenr / PAGE_COUNT]);
        ASSERT(changeAllowed(pd, pagenr / PAGE_COUNT));

        // Get the physical address and invalidate the page
        uint32_t* page = &pd->tables[pagenr / PAGE_COUNT]->pages[pagenr%PAGE_COUNT];
        *page = 0;

        if (pd->tables[pagenr / PAGE_COUNT] == currentPageDirectory->tables[pagenr / PAGE_COUNT])
            invalidateTLBEntry((void*)(pagenr*PAGESIZE));

        mutex_unlock(&pd->mutex);
    }
}

// ------------------------------------------------------------------------------
// Creates the mapping between a physical and a virtual address in the given page
// directory. Both vaddr and paddr should have been allocated previously.
// ------------------------------------------------------------------------------
bool paging_mapVirtToPhysAddr(pageDirectory_t* pd, void* vaddr, uintptr_t paddr, size_t pages, MEMFLAGS_t flags)
{
    // "vaddr" must be page-aligned
    ASSERT(((uintptr_t)vaddr) % PAGESIZE == 0);

    for (uint32_t pg = 0; pg < pages; ++pg)
    {
        uint32_t pagenr = (uintptr_t)vaddr / PAGESIZE + pg;

        // Get the page table
        pageTable_t* pt = pd->tables[pagenr / PAGE_COUNT];

        mutex_lock(&pd->mutex);
        // Is memory already allocated?
        if (!pt || !(pt->pages[pagenr%PAGE_COUNT] & MEM_ALLOCATED))
        {
            mutex_unlock(&pd->mutex);
            printfe("Page not allocated: %u\n", pagenr);
            return false;
        }

        ASSERT(changeAllowed(pd, pagenr / PAGE_COUNT));

        pd->codes[pagenr / PAGE_COUNT] = paging_getPhysAddr(pt) | MEM_PRESENT | MEM_WRITE | (flags&(~MEM_NOTLBUPDATE)); // Update codes

        // Setup the page
        pt->pages[pagenr % PAGE_COUNT] = (paddr + pg * PAGESIZE) | flags | MEM_PRESENT | MEM_ALLOCATED;
        mutex_unlock(&pd->mutex);

        if (pt == currentPageDirectory->tables[pagenr / PAGE_COUNT])
            invalidateTLBEntry(vaddr + pg * PAGESIZE);

      #ifdef _DIAGNOSIS_
        if (flags & MEM_USER)
            printf("page %u now associated to physAddress %Xh\n", pagenr, paddr + pg * PAGESIZE);
      #endif
    }
    return true;
}


// ------------------------------------------------------------------------------
// Allocates memory of given size at given virtual address, allocates physical
// memory and creates mapping.
// ------------------------------------------------------------------------------
bool paging_alloc(pageDirectory_t* pd, void* addr, uint32_t size, MEMFLAGS_t flags)
{
    // "virtAddress" and "size" must be page-aligned
    ASSERT(((uintptr_t)addr) % PAGESIZE == 0);
    ASSERT(size % PAGESIZE == 0);

    size_t pages = size / PAGESIZE;

    // Allocate virtual memory
    if (!paging_allocVirtAddr(pd, addr, pages))
    {
        printfe("paging_allocVirtAddr(%X, %X, %u) failed.", pd, addr, pages);
        return false;
    }

    // We repeat allocating one page at once
    for (size_t done = 0; done < pages; done++)
    {
        // Allocate physical memory
        uintptr_t paddr = paging_allocPhysMem(1, true);

        // Link physical with virtual memory
        if (!paging_mapVirtToPhysAddr(pd, addr + done*PAGESIZE, paddr, 1, flags))
        {
            paging_freePhysMem(paddr, 1);
            printfe("paging_mapVirtToPhysAddr(%X, %X, %X, %u, %X) failed.", pd, addr + done*PAGESIZE, paddr, 1, flags);
            return false;
        }

      #ifdef _DIAGNOSIS_
        if (flags & MEM_USER)
            printf("pagenumber now allocated: %u physAddress: %Xh\n", done, paddr);
      #endif
    }
    return true;
}

// TODO: Delete
// Allocates memory memory using paging_alloc, but fails if there are allocated pages within the area
bool paging_allocNew(pageDirectory_t* pd, void* virtAddress, uint32_t size, MEMFLAGS_t flags)
{
    // "virtAddress" and "size" must be page-aligned
    ASSERT(((uintptr_t) virtAddress) % PAGESIZE == 0);
    ASSERT(size % PAGESIZE == 0);

    // Check whether a page is allocated within the area
    for (uint32_t done = 0; done < size / PAGESIZE; done++)
    {
        uint32_t pagenr = (uintptr_t) virtAddress / PAGESIZE + done;
        if (pd->tables[pagenr / PAGE_COUNT] && (pd->tables[pagenr / PAGE_COUNT]->pages[pagenr%PAGE_COUNT] & MEM_ALLOCATED))
            return false;
    }

    return paging_alloc(pd, virtAddress, size, flags);
}

// ------------------------------------------------------------------------------
// Frees virtual and physical memory at given virtual address and deletes mapping.
// ------------------------------------------------------------------------------
void paging_free(pageDirectory_t* pd, void* virtAddress, uint32_t size)
{
    // "virtAddress" and "size" must be page-aligned
    ASSERT(((uintptr_t)virtAddress) % PAGESIZE == 0);
    ASSERT(size % PAGESIZE == 0);

    // Go through all pages and free them
    size_t pagenr = (uintptr_t)virtAddress / PAGESIZE;

    while (size)
    {
        ASSERT(pd->tables[pagenr / PAGE_COUNT] && changeAllowed(pd, pagenr / PAGE_COUNT));

        // Get the physical address
        uintptr_t physAddress = paging_getPhysAddr(virtAddress);

        // Free virtual and physical memory
        paging_freeVirtMem(pd, virtAddress, 1);
        paging_freePhysMem(physAddress, 1);

        // Adjust variables for next loop run
        size -= PAGESIZE;
        pagenr++;
    }
}

// ------------------------------------------------------------------------------
// Sets the flags for a given area of memory in a given page directory.
// ------------------------------------------------------------------------------
void paging_setFlags(pageDirectory_t* pd, void* virtAddress, uint32_t size, MEMFLAGS_t flags)
{
    // "virtAddress" and "size" must be page-aligned
    ASSERT(((uintptr_t) virtAddress) % PAGESIZE == 0);
    ASSERT(size % PAGESIZE == 0);

    // Check whether a page is allocated within the area
    for (uint32_t done = 0; done < size / PAGESIZE; done++)
    {
        uint32_t pagenr = (uintptr_t) virtAddress / PAGESIZE + done;
        mutex_lock(&pd->mutex);
        ASSERT(pd->tables[pagenr / PAGE_COUNT] && (pd->tables[pagenr / PAGE_COUNT]->pages[pagenr%PAGE_COUNT] & MEM_ALLOCATED) && changeAllowed(pd, pagenr / PAGE_COUNT));

        uint32_t* page = &pd->tables[pagenr / PAGE_COUNT]->pages[pagenr%PAGE_COUNT];
        *page = (*page & 0xFFFFF000) | flags | MEM_PRESENT | MEM_ALLOCATED;

        if (pd->tables[pagenr / PAGE_COUNT] == currentPageDirectory->tables[pagenr / PAGE_COUNT])
            invalidateTLBEntry((void*)(pagenr*PAGESIZE));
        mutex_unlock(&pd->mutex);
    }
}

// ------------------------------------------------------------------------------
// Destroys given page directory and frees all physical memory associated with it.
// ------------------------------------------------------------------------------
pageDirectory_t* paging_createPageDirectory(void)
{
    // Allocate memory for the page directory
    pageDirectory_t* pd = (pageDirectory_t*)malloc(sizeof(pageDirectory_t), PAGESIZE, "pageDirectory_t");

    if (!pd)
        return (0);

    // Each user's page directory contains the same mapping as the kernel
    memcpy(pd, kernelPageDirectory, sizeof(pageDirectory_t));
    pd->physAddr = paging_getPhysAddr(pd->codes);
    mutex_construct(&pd->mutex);

    return (pd);
}

// ------------------------------------------------------------------------------
// Destroys given page directory and frees all physical memory associated with it.
// ------------------------------------------------------------------------------
void paging_destroyPageDirectory(pageDirectory_t* pd)
{
    ASSERT(pd != kernelPageDirectory); // The kernel's page directory must not be destroyed
    if (pd == currentPageDirectory)
        paging_switch(kernelPageDirectory); // Leave current PD, if we attempt to delete it

    // Free all memory that is not from the kernel
    for (uint32_t i=0; i<PT_COUNT; i++)
    {
        if (pd->tables[i] && changeAllowed(pd, i))
        {
            for (uint32_t j=0; j<PAGE_COUNT; j++)
            {
                uint32_t physAddress = pd->tables[i]->pages[j] & 0xFFFFF000;

                if (physAddress)
                {
                    paging_freePhysMem(physAddress, 1);
                }
            }
            free(pd->tables[i]);
        }
    }

    free(pd);
}

// ------------------------------------------------------------------------------
// Allocates physical and virtual memory with given physical address, as commonly
// required for MMIO.
// ------------------------------------------------------------------------------
void* paging_allocMMIO(uintptr_t paddr, size_t pages)
{
    static mutex_t mutex = mutex_init();
    static void* head = (void*)PCI_MEM_START;

    if (!paging_allocPhysAddr(paddr, pages))
    {
        // TODO: Enable this.
        //printfe("\nPhysical address not available for MMIO.");
        //return (0);
    }

    mutex_lock(&mutex);
    void* vaddr = head;
    head += pages*PAGESIZE;

    // Allocate virtual memory
    // TODO: Use paging_allocVirtMem instead of paging_allocVirtAddr to get rid of PCI_MEM-area
    if (!paging_allocVirtAddr(currentPageDirectory, vaddr, pages))
    {
        printfe("\nNot enough virtual memory for MMIO available.");

        mutex_unlock(&mutex);
        return (0);
    }
    paging_mapVirtToPhysAddr(currentPageDirectory, vaddr, paddr, pages, MEM_WRITE);

    mutex_unlock(&mutex);
    return vaddr;
}

// ------------------------------------------------------------------------------
// Calculates physical address the virtual address is mapped to in current pd.
// ------------------------------------------------------------------------------
uintptr_t paging_getPhysAddr(const void* virtAddress)
{
    // Find the page table
    uint32_t pageNumber = (uintptr_t)virtAddress / PAGESIZE;
    pageTable_t* pt = currentPageDirectory->tables[pageNumber/PAGE_COUNT];

  #ifdef _PAGING_DIAGNOSIS_
    printf("\nvirt-->phys: pagenr: %u, pt: %Xh\n", pageNumber, pt);
  #endif

    if (pt)
    {
        // Read address, cut off flags, append odd part of virtual address
        return ((pt->pages[pageNumber % PAGE_COUNT] & 0xFFFFF000) + ((uintptr_t)virtAddress & 0x00000FFF));
    }
    else
    {
        return (0); // not mapped
    }
}


// ------------------------------------------------------------------------------
// Debugging functions.
// ------------------------------------------------------------------------------

static void* lookForVirtAddr(uintptr_t physAddr, uintptr_t start, uintptr_t end)
{
    for (uintptr_t i = start; (i < end) && (i >= start); i+=PAGESIZE)
    {
        if (paging_getPhysAddr((void*)i) == (physAddr & 0xFFFFF000))
        {
            return (void*)(i + (physAddr & 0x00000FFF));
        }
    }
    return (0); // Not mapped between start and end
}

void* paging_getVirtAddr(uintptr_t physAddress)
{
    // check idendity mapping area
    if (physAddress < IDMAP * 1024 * PAGESIZE)
    {
        return (void*)physAddress;
    }

    // check current used heap
    void* virtAddr = lookForVirtAddr(physAddress, (uintptr_t)KERNEL_HEAP_START, (uintptr_t)heap_getCurrentEnd());
    if (virtAddr)
    {
        return (virtAddr);
    }

    // check pci memory
    lookForVirtAddr(physAddress, (uintptr_t)PCI_MEM_START, (uintptr_t)PCI_MEM_END);
    if (virtAddr)
    {
        return (virtAddr);
    }

    // check between idendity mapping area and heap start
    lookForVirtAddr(physAddress, IDMAP * 1024 * PAGESIZE, (uintptr_t)KERNEL_HEAP_START);
    if (virtAddr)
    {
        return (virtAddr);
    }

    // check between current heap end and theoretical heap end
    lookForVirtAddr(physAddress, (uintptr_t)heap_getCurrentEnd(), (uintptr_t)KERNEL_HEAP_END);
    return (virtAddr);
}


void paging_analyzeBitTable(void)
{
    uint8_t k = 1, k_old = 1;
    int64_t ramsize = 0;
    ipc_getInt("PrettyOS/RAM", &ramsize);
    uint32_t maximum = min((uint32_t)(ramsize/PAGESIZE/32), MAX_DWORDS);

    mutex_lock(&phys_mutex);
    for (uint32_t i=0; i < maximum; i++)
    {
        textColor(TEXT);
        printf("\n%Xh: ", 32 * i * PAGESIZE);
        serial_log(SER_LOG_PAGE,"\n%Xh: ", 32 * i * PAGESIZE);

        for (uint32_t offset=0; offset<32; offset++)
        {
            if (!(phys_reservationTable[i] & BIT(offset)))
            {
                textColor(GREEN);
                putch('0');
                serial_log(SER_LOG_PAGE,"0");

                if (offset == 31)
                {
                    k_old = k;
                    k = 0;
                }
            }
            else
            {
                textColor(LIGHT_GRAY);
                putch('1');
                serial_log(SER_LOG_PAGE,"1");

                if (offset == 31)
                {
                    k_old = k;
                    k = 1;
                }
            }
        }

        if (k != k_old)
        {
            sleepSeconds(3); // wait a little bit at changes
        }

    }
    mutex_unlock(&phys_mutex);
    textColor(TEXT);
}


/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
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
