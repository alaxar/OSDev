/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "kheap.h"
#include "util/util.h"
#include "memory.h"
#include "video/console.h"
#include "paging.h"
#include "serial.h"

/* The heap provides the malloc/free-functionality, i.e. dynamic allocation of memory.
   It manages a certain amount of continuous virtual memory, starting at "heapStart".
   Whenever more memory is requested than there is available, the heap expands.
   For expansion, the heap asks the paging module to map physical memory to the following virtual addresses
   and increases its "heapSize" variable (but at least by "HEAP_MIN_GROWTH") afterwards.

   To manage the free and reserved (allocated) areas of the heap an array of "region" elements is kept.
   Each region specifies its size and reservation status.
   Free regions always get merged. Regions don't store their addresses.
   The third region address is calculated by adding the first and second region size to "heapStart":
   region_3_addr = heapStart + regions[0].size + regions[1].size

   Before the heap is set up, memory is allocated on a "placement address".
   This is an identity mapped area of continuous memory,
   the allocation just moves a pointer forward by the requested size and returns its previous value.

   The heap's management data is placed at this placement address, too.
   Since this area cannot grow, the heap has a maximum amount of region objects ("regionMaxCount").*/

// TODO: Ensure the heap will not overflow (above KERNEL_heapEnd, cf. memory.h)


typedef struct
{
    uint32_t size;
    uint32_t number;
    bool     reserved;
    char     comment[21];
} __attribute__((packed)) region_t;


static region_t*      regions         = 0;
static uint32_t       regionCount     = 0;
static uint32_t       regionMaxCount  = 0;
static uint32_t       firstFreeRegion = 0;
static void*          firstFreeAddr   = (void*)KERNEL_HEAP_START;
static uint8_t* const heapStart       = (void*)KERNEL_HEAP_START;
static uint32_t       heapSize        = 0;
static const uint32_t HEAP_MIN_GROWTH = 0x10000;

static mutex_t mutex = mutex_init();

#ifdef _MEMLEAK_FIND_
static uint32_t counter = 0;
#endif


static void* placementMalloc(size_t size, uint32_t alignment);


void heap_install(void)
{
    // This gets us the current placement address
    regions = placementMalloc(0, 0);

    // We take the rest of the placement area
    regionCount = 0;
    regionMaxCount = (PLACEMENT_END - (uintptr_t)regions) / sizeof(region_t);
}

void* heap_getCurrentEnd(void)
{
    return (heapStart + heapSize);
}

static bool heap_grow(size_t size, uint8_t* heapEnd, bool continuous)
{
    // We will have to append another region-object to our array if we can't merge with the last region - check whether there would be enough space to insert the region-object
    if ((regionCount > 0) && regions[regionCount-1].reserved && (regionCount >= regionMaxCount))
    {
        return (false);
    }

    mutex_lock(&mutex);
    // Enhance the memory
    if (!paging_alloc(kernelPageDirectory, heapEnd, size, MEM_WRITE))
    {
        mutex_unlock(&mutex);
        return (false);
    }

    // Maybe we can merge with the last region object?
    if ((regionCount > 0) && !regions[regionCount-1].reserved)
    {
        regions[regionCount-1].size += size;
    }
    // Otherwise insert a new region object
    else
    {
        regions[regionCount].reserved = false;
        regions[regionCount].size = size;
        regions[regionCount].number = 0;

        ++regionCount;
    }

    heapSize += size;
    mutex_unlock(&mutex);
    return (true);
}

static void* placementMalloc(size_t size, uint32_t alignment)
{
    static void* nextPlacement = (void*)PLACEMENT_BEGIN;

    // Avoid odd addresses
    size = alignUp(size, 4);

    // Ensure alignment
    void* currPlacement = (void*)alignUp((uintptr_t)nextPlacement, alignment);

    // Check if there is enough space in placement area
    if ((uintptr_t)currPlacement + size > PLACEMENT_END)
        return (0);

    // Do simple placement allocation
    mutex_lock(&mutex);
    nextPlacement = currPlacement + size;
    mutex_unlock(&mutex);

    return (currPlacement);
}

void* malloc(size_t size, uint32_t alignment, const char* comment)
{
    // consecutive number for detecting the sequence of mallocs at the heap
    static uint32_t consecutiveNumber = 0;

    // Analyze alignment and other requirements
    size_t within = 0xFFFFFFFF;
    if (alignment&HEAP_WITHIN_PAGE)
    {
        ASSERT(size <= PAGESIZE);
        within = PAGESIZE;
    }
    else if (alignment&HEAP_WITHIN_64K)
    {
        ASSERT(size <= 0x10000);
        within = 0x10000;
    }
    bool continuous = alignment&HEAP_CONTINUOUS;

    alignment &= HEAP_ALIGNMENT_MASK;

    // If the heap is not set up, do placement malloc
    if (regions == 0)
    {
        return (placementMalloc(size, alignment));
    }

    // Avoid odd addresses
    size = alignUp(size, 4);

    mutex_lock(&mutex);
    // Walk the regions and find one being suitable
    bool foundFree = false;
    uint8_t* regionAddress = firstFreeAddr;
    for (uint32_t i = firstFreeRegion; i < regionCount; i++)
    {
        // Manage caching of first free region
        if (!regions[i].reserved)
            foundFree = true;
        else if (!foundFree)
        {
            firstFreeRegion = i;
            firstFreeAddr = regionAddress;
        }

        // Calculate aligned address and the additional size needed due to alignment
        uint8_t* alignedAddress = (uint8_t*)alignUp((uintptr_t)regionAddress, alignment);
        uintptr_t additionalSize = (uintptr_t)alignedAddress - (uintptr_t)regionAddress;

        // Check whether this region is free, big enough and fits page requirements
        if (!regions[i].reserved && (regions[i].size >= size + additionalSize) &&
            (within - (uintptr_t)regionAddress%within >= additionalSize))
        {
            // Check if the region consists of continuous physical memory if required
            if (continuous)
            {
                bool iscontinuous = true;
                for (void* virt1 = (void*)alignDown((uintptr_t)alignedAddress, PAGESIZE); (uintptr_t)(virt1+PAGESIZE) <= (uintptr_t)(alignedAddress + size); virt1+=PAGESIZE) {
                    uintptr_t phys1 = paging_getPhysAddr(virt1);
                    uintptr_t phys2 = paging_getPhysAddr(virt1 + PAGESIZE);
                    if (phys1 + PAGESIZE != phys2)
                    {
                        iscontinuous = false;
                        break;
                    }
                }
                if (!iscontinuous)
                    continue;
            }

            // We will split up this region ...
            // +--------------------------------------------------------+
            // |                      Current Region                    |
            // +--------------------------------------------------------+
            //
            // ... into three, the first and third remain free,
            // while the second gets reserved, and its address is returned
            //
            // +------------------+--------------------------+----------+
            // | Before Alignment | Aligned Destination Area | Leftover |
            // +------------------+--------------------------+----------+

            // Split the pre-alignment area
            if (alignedAddress != regionAddress)
            {
                // Check whether we are able to expand
                if (regionCount >= regionMaxCount)
                {
                    mutex_unlock(&mutex);
                    return (0);
                }

                // Move all following regions ahead to get room for a new one
                memmove(regions + i+1, regions + i, (regionCount-i) * sizeof(region_t));

                ++regionCount;

                // Setup the regions
                regions[i].size     = alignedAddress - regionAddress;
                regions[i].reserved = false;

                regions[i+1].size  -= regions[i].size;

                // "Aligned Destination Area" becomes the "current" region
                regionAddress += regions[i].size;
                i++;
            }

            // Split the leftover
            if (regions[i].size > size + additionalSize)
            {
                // Check whether we are able to expand
                if (regionCount+1 > regionMaxCount)
                {
                    mutex_unlock(&mutex);
                    return (0);
                }

                // Move all following regions ahead to get room for a new one
                memmove(regions + i+2, regions + i+1, (regionCount-i-1)*sizeof(region_t));

                ++regionCount;

                // Setup the regions
                regions[i+1].size     = regions[i].size - size;
                regions[i+1].reserved = false;
                regions[i+1].number   = 0;

                regions[i].size       = size;
            }

            // Set the region to "reserved" and return its address
            regions[i].reserved = true;
            strncpy(regions[i].comment, comment, 20);
            regions[i].comment[20] = 0;
            regions[i].number = ++consecutiveNumber;

          #ifdef _MEMLEAK_FIND_
            counter++;
            writeInfo(2, "Malloc - free: %u", counter);
          #endif
          #ifdef _MALLOC_FREE_LOG_
            textColor(YELLOW);
            printf("\nmalloc: %Xh %s", regionAddress, comment);
            textColor(TEXT);
          #endif

            mutex_unlock(&mutex);
            return (regionAddress);

        } //region is free and big enough

        regionAddress += regions[i].size;
    }

    // There is nothing free, try to expand the heap
    uint32_t sizeToGrow = max(HEAP_MIN_GROWTH, alignUp(size*3/2,PAGESIZE));
    bool success = heap_grow(sizeToGrow, (uint8_t*)((uintptr_t)heapStart + heapSize), continuous);

    mutex_unlock(&mutex);

    if (!success)
    {
        printfe("\nmalloc (\"%s\") failed, heap could not be expanded!", comment);
        return (0);
    }
    else
    {
      #ifdef _MALLOC_FREE_LOG_
        textColor(YELLOW);
        printf("\nheap expanded: %Xh heap end: %Xh", sizeToGrow, (uintptr_t)heapStart + heapSize);
        textColor(TEXT);
      #endif
    }

    // Now there should be a region that is large enough
    return malloc(size, alignment, comment);
}


void free(void* addr)
{
  #ifdef _MALLOC_FREE_LOG_
    textColor(LIGHT_GRAY);
    printf("\nfree:   %Xh", addr);
    textColor(TEXT);
  #endif

    if (addr == 0)
    {
        return;
    }

  #ifdef _MEMLEAK_FIND_
    counter--;
    writeInfo(2, "Malloc - free: %u", counter);
  #endif

    mutex_lock(&mutex);

    // Walk the regions and find the correct one
    uint8_t* regionAddress = heapStart;
    for (uint32_t i=0; i<regionCount; i++)
    {
        if (regionAddress == addr && regions[i].reserved)
        {
          #ifdef _MALLOC_FREE_LOG_
            textColor(LIGHT_GRAY);
            printf(" %s", regions[i].comment);
            textColor(TEXT);
          #endif
            regions[i].number = 0;
            regions[i].reserved = false; // free the region

            // Check for a merge with the next region
            if ((i+1 < regionCount) && !regions[i+1].reserved)
            {
                // Adjust the size of the now free region
                regions[i].size += regions[i+1].size; // merge

                // Move all following regions back by one
                memmove(regions + i+1, regions + i+2, (regionCount-2-i)*sizeof(region_t));

                --regionCount;
            }

            // Check for a merge with the previous region
            if (i>0 && !regions[i-1].reserved)
            {
                // Adjust the size of the previous region
                regions[i-1].size += regions[i].size; // merge

                // Move all following regions back by one
                memmove(regions + i, regions + i+1, (regionCount-1-i)*sizeof(region_t));

                --regionCount;
            }

            if (i < firstFreeRegion)
            {
                firstFreeRegion = i;
                firstFreeAddr = regionAddress;
            }

            mutex_unlock(&mutex);
            return;
        }

        regionAddress += regions[i].size;
    }

    mutex_unlock(&mutex);

    printfe("\nBroken free: %Xh", addr);
    printStackTrace(0, 0); // Print a stack trace to get the function call that caused the problem
}

void heap_logRegions(void)
{
    printf("\nDebug: Heap regions sent to serial output.\n");
    serial_log(SER_LOG_HEAP,"\n\nregionMaxCount: %u, regionCount: %u, firstFreeRegion: %u\n", regionMaxCount, regionCount, firstFreeRegion);
    serial_log(SER_LOG_HEAP,"\n---------------- HEAP REGIONS ----------------\n");
    serial_log(SER_LOG_HEAP,"#\taddress\t\tsize\t\tnumber\tcomment");

    uintptr_t regionAddress = (uintptr_t)heapStart;

    for (uint32_t i=0; i<regionCount; i++)
    {
        if (regions[i].reserved)
            serial_log(SER_LOG_HEAP, "\n%u\t%Xh\t%Xh\t%u\t%s", i, regionAddress, regions[i].size, regions[i].number, regions[i].comment);
        else
            serial_log(SER_LOG_HEAP, "\n%u\t%Xh\t%Xh\t-\t-", i, regionAddress, regions[i].size);
        regionAddress += regions[i].size;
    }
    serial_log(SER_LOG_HEAP,"\n---------------- HEAP REGIONS ----------------\n\n");
}

/*
* Copyright (c) 2009-2015 The PrettyOS Project. All rights reserved.
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
