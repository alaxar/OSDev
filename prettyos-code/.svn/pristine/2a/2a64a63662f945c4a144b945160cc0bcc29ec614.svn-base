/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userheap.h"
#include "kheap.h"
#include "memory.h"
#include "serial.h"
#include "util/util.h"
#include "tasking/task.h"


typedef struct
{
    uint8_t* start;
    uint8_t* end; // Pointer to post-last byte of the region (similar to C++ iterators)
    uint32_t useCount;
    bool     keepOnZeroUse;
} userHeap_region_t;

typedef struct
{
    userHeap_region_t* reg;
    uint8_t* start;
    uint8_t* end; // Pointer to post-last byte of the region (similar to C++ iterators)
} userHeap_block_t;

static bool allocateNewReg(userHeap_t* heap, size_t size, bool keepOnZeroUse);      // Allocates a region of <= size bytes
static bool increaseReg(userHeap_region_t* reg, userHeap_t* heap, size_t increase); // Tries to increase the region
static void* tryAllocFromFreeBlock(userHeap_t* heap, size_t size, bool reverseOrder);

#ifdef _USERHEAP_DIAGNOSIS_
static void dumpHeap(userHeap_t* heap);
#endif

userHeap_t* userHeap_create(pageDirectory_t* pageDir, size_t initialSize)
{
  #ifdef _USERHEAP_DIAGNOSIS_
    serial_log(SER_LOG_USERHEAP, "userHeap_create\n");
  #endif
    if (initialSize > (size_t)(USER_HEAP_END - USER_HEAP_START))
        return (0);

    userHeap_t* heap = malloc(sizeof(userHeap_t), 0, "userHeap_t");
    heap->magic = 0xABC0FFEE;
    mutex_construct(&heap->lock);
    heap->pageDir = pageDir;

    list_construct(&heap->regions);
    list_construct(&heap->allocatedBlocks);
    list_construct(&heap->freeBlocks);

    if (initialSize && !allocateNewReg(heap, initialSize, true))
    {
        userHeap_destroy(heap);
        return (0);
    }

  #ifdef _USERHEAP_DIAGNOSIS_
    serial_log(SER_LOG_USERHEAP, "Created heap at %X (PD: %X)\n", heap, heap->pageDir);
    dumpHeap(heap);
  #endif
    return heap;
}

void userHeap_destroy(userHeap_t* heap)
{
    mutex_lock(&heap->lock);

  #ifdef _USERHEAP_DIAGNOSIS_
    serial_log(SER_LOG_USERHEAP, "userHeap_destroy\n");
    dumpHeap(heap);
  #endif

    for (dlelement_t* listEntry = heap->regions.head; listEntry; listEntry = listEntry->next)
    {
        userHeap_region_t* region = listEntry->data;
        paging_free(heap->pageDir, region->start, region->end - region->start);
        free(region);
    }

    for (dlelement_t* listEntry = heap->allocatedBlocks.head; listEntry; listEntry = listEntry->next)
        free(listEntry->data);

    for (dlelement_t* listEntry = heap->freeBlocks.head; listEntry; listEntry = listEntry->next)
        free(listEntry->data);

    list_destruct(&heap->regions);
    list_destruct(&heap->allocatedBlocks);
    list_destruct(&heap->freeBlocks);

    mutex_destruct(&heap->lock);

  #ifdef _USERHEAP_DIAGNOSIS_
    serial_log(SER_LOG_USERHEAP, "Destroyed heap at %X\n", heap);
  #endif
    free(heap);
}

void* userHeap_alloc(userHeap_t* heap, size_t size)
{
  #ifdef _USERHEAP_DIAGNOSIS_
    serial_log(SER_LOG_USERHEAP, "userHeap_alloc\n");
  #endif
    mutex_lock(&heap->lock);

    void* ret = tryAllocFromFreeBlock(heap, size, false);
    if (ret)
    {
      #ifdef _USERHEAP_DIAGNOSIS_
        dumpHeap(heap);
      #endif
        mutex_unlock(&heap->lock);
        return ret;
    }

    for (dlelement_t* listEntry = heap->regions.tail; listEntry; listEntry = listEntry->prev)
    {
        if (increaseReg(listEntry->data, heap, size))
        {
            ret = tryAllocFromFreeBlock(heap, size, true);
            if (ret)
            {
              #ifdef _USERHEAP_DIAGNOSIS_
                dumpHeap(heap);
              #endif
                mutex_unlock(&heap->lock);
                return ret;
            }
        }
    }

    if (allocateNewReg(heap, size, false))
    {
        ret = tryAllocFromFreeBlock(heap, size, true);
        if (ret)
        {
          #ifdef _USERHEAP_DIAGNOSIS_
            dumpHeap(heap);
          #endif
            mutex_unlock(&heap->lock);
            return ret;
        }
    }

  #ifdef _USERHEAP_DIAGNOSIS_
    dumpHeap(heap);
  #endif
    mutex_unlock(&heap->lock);
    return (0);
}

void userHeap_free(userHeap_t* heap, void* address)
{
  #ifdef _USERHEAP_DIAGNOSIS_
    serial_log(SER_LOG_USERHEAP, "userHeap_free\n");
  #endif
    if (!address)
    {
        return;
    }

    mutex_lock(&heap->lock);

    for (dlelement_t* listEntry = heap->allocatedBlocks.tail; listEntry; listEntry = listEntry->prev)
    {
        userHeap_block_t* block = listEntry->data;

        if (block->start == address)
        {
            --block->reg->useCount;
            list_delete(&heap->allocatedBlocks, listEntry);

            //  Region no longer in use; delete it
            if (!block->reg->useCount && !block->reg->keepOnZeroUse)
            {
                list_delete(&heap->regions, list_find(&heap->regions, block->reg));
                paging_free(heap->pageDir, block->reg->start, block->reg->end - block->reg->start);

                // Delete any free blocks pointing to the unused region
                for (dlelement_t* freeBlockEntry = heap->freeBlocks.head; freeBlockEntry; freeBlockEntry = freeBlockEntry->next)
                {
                    userHeap_block_t* freeBlock = freeBlockEntry->data;
                    if (freeBlock->reg == block->reg)
                    {
                        free(freeBlock);
                        list_delete(&heap->freeBlocks, freeBlockEntry);
                    }
                }

              #ifdef _USERHEAP_DIAGNOSIS_
                for (dlelement_t* allocatedBlockEntry = heap->allocatedBlocks.head; allocatedBlockEntry;
                    allocatedBlockEntry = allocatedBlockEntry->next)
                {
                    if (((userHeap_block_t*)allocatedBlockEntry->data)->reg == block->reg)
                    {
                        dumpHeap(heap);
                        mutex_unlock(&heap->lock);
                        panic_assert(__FILE__, __LINE__, "Allocated entry pointing to region with useCount 0");
                    }
                }
              #endif

                free(block->reg);
                free(block);

              #ifdef _USERHEAP_DIAGNOSIS_
                dumpHeap(heap);
              #endif
                mutex_unlock(&heap->lock);
                return;
            }

            list_append(&heap->freeBlocks, block);

            // Prevent fragmenting of free blocks next to each other
            for (bool runAgain = true; runAgain;)
            {
                runAgain = false;
                for (dlelement_t* freeBlockEntry = heap->freeBlocks.head; freeBlockEntry;
                    freeBlockEntry = freeBlockEntry->next)
                {
                    userHeap_block_t* freeBlock = freeBlockEntry->data;
                    if (freeBlock->end == block->start)
                    {
                        freeBlock->end = block->end;

                        list_delete(&heap->freeBlocks, list_find(&heap->freeBlocks, block));
                        free(block);

                        block = freeBlock; // Re-run the merge loop
                        runAgain = true;
                        break;
                    }
                    else if (freeBlock->start == block->end)
                    {
                        freeBlock->start = block->start;

                        list_delete(&heap->freeBlocks, list_find(&heap->freeBlocks, block));
                        free(block);

                        block = freeBlock; // Re-run the merge loop
                        runAgain = true;
                        break;
                    }
                }
            }

          #ifdef _USERHEAP_DIAGNOSIS_
            dumpHeap(heap);
          #endif
            mutex_unlock(&heap->lock);
            return;
        }
    }
}

static bool allocateNewReg(userHeap_t* heap, size_t size, bool keepOnZeroUse) // Brute-force; only use, when increase does not work
{
    for (uint8_t* start = USER_HEAP_START; (start + alignUp(size, PAGESIZE)) <= USER_HEAP_END; start += PAGESIZE)
    {
        if (paging_allocNew(heap->pageDir, start, alignUp(size, PAGESIZE), MEM_USER | MEM_WRITE))
        {
            userHeap_region_t* newReg = malloc(sizeof(userHeap_region_t), 0, "userHeap_region_t");
            newReg->start = start;
            newReg->end = start + alignUp(size, PAGESIZE);
            newReg->useCount = 0;
            newReg->keepOnZeroUse = keepOnZeroUse;
            list_append(&heap->regions, newReg);

            userHeap_block_t* freeBlock = malloc(sizeof(userHeap_block_t), 0, "userHeap_block_t");
            freeBlock->start = newReg->start;
            freeBlock->end = newReg->end;
            freeBlock->reg = newReg;
            list_append(&heap->freeBlocks, freeBlock);

            return true;
        }
    }

    return false; // Could not find any fitting memory area
}

static bool increaseReg(userHeap_region_t* reg, userHeap_t* heap, size_t increase)
{
    if (paging_allocNew(heap->pageDir, reg->end, alignUp(increase, PAGESIZE), MEM_USER | MEM_WRITE))
    {
        for (dlelement_t* listEntry = heap->freeBlocks.head; listEntry; listEntry = listEntry->next)
        {
            userHeap_block_t* block = listEntry->data;
            if (block->reg == reg && block->end == reg->end)
            {
                reg->end += alignUp(increase, PAGESIZE);
                block->end = reg->end;

                return true;
            }
        }

        // No free block at the end (block was completely used)
        userHeap_block_t* block = malloc(sizeof(userHeap_block_t), 0, "userHeap_block_t");
        block->reg = reg;
        block->start = reg->end; // Old end
        reg->end += alignUp(increase, PAGESIZE); // Set new end
        block->end = reg->end; // New end
        list_append(&heap->freeBlocks, block);

        return true;
    }

    return false;
}

static void* tryAllocFromFreeBlock(userHeap_t* heap, size_t size, bool reverseOrder)
{
    for (dlelement_t* listEntry = reverseOrder ? heap->freeBlocks.tail : heap->freeBlocks.head; listEntry;
        listEntry = reverseOrder ? listEntry->prev : listEntry->next)
    {
        userHeap_block_t* block = listEntry->data;
        if ((size_t)(block->end - block->start) > size)
        {
            userHeap_block_t* newBlock = malloc(sizeof(userHeap_block_t), 0, "userHeap_block_t");
            newBlock->start = block->start;
            newBlock->end = block->start + size;
            newBlock->reg = block->reg;
            list_append(&heap->allocatedBlocks, newBlock);

            block->start = newBlock->end;
          #ifdef _USERHEAP_DIAGNOSIS_
            ASSERT(block->start != block->end);
          #endif
            ++newBlock->reg->useCount;
            return newBlock->start;
        }
        else if ((size_t)(block->end - block->start) == size)
        {
            list_delete(&heap->freeBlocks, listEntry);
            list_append(&heap->allocatedBlocks, block);

            ++block->reg->useCount;
            return block->start;
        }
    }

    return (0);
}

#ifdef _USERHEAP_DIAGNOSIS_
static void dumpHeap(userHeap_t* heap)
{
    serial_log(SER_LOG_USERHEAP, "@@ BEGINNING USER HEAP DUMP @@@@@@@@@@@\n");

    serial_log(SER_LOG_USERHEAP, "@%X\nMutex:\t%X\nPage-Dir:\t%X\n", heap, &heap->lock, heap->pageDir);

    serial_log(SER_LOG_USERHEAP, "Regions:\n");
    for (dlelement_t* listEntry = heap->regions.head; listEntry; listEntry = listEntry->next)
    {
        userHeap_region_t* reg = listEntry->data;
        serial_log(SER_LOG_USERHEAP, "\tRegion @%X:\n", reg);
        serial_log(SER_LOG_USERHEAP, "\t\tStart:\t\t%X\n", reg->start);
        serial_log(SER_LOG_USERHEAP, "\t\tEnd:\t\t%X\n", reg->end);
        serial_log(SER_LOG_USERHEAP, "\t\tuseCount:\t%u\n", reg->useCount);
    }

    serial_log(SER_LOG_USERHEAP, "Allocated blocks:\n");
    for (dlelement_t* listEntry = heap->allocatedBlocks.head; listEntry; listEntry = listEntry->next)
    {
        userHeap_block_t* block = listEntry->data;
        serial_log(SER_LOG_USERHEAP, "\tBlock @%X:\n", block);
        serial_log(SER_LOG_USERHEAP, "\t\tStart:\t%X\n", block->start);
        serial_log(SER_LOG_USERHEAP, "\t\tEnd:\t%X\n", block->end);
        serial_log(SER_LOG_USERHEAP, "\t\tRegion:\t%X\n", block->reg);
    }

    serial_log(SER_LOG_USERHEAP, "Free blocks:\n");
    for (dlelement_t* listEntry = heap->freeBlocks.head; listEntry; listEntry = listEntry->next)
    {
        userHeap_block_t* block = listEntry->data;
        serial_log(SER_LOG_USERHEAP, "\tBlock @%X:\n", block);
        serial_log(SER_LOG_USERHEAP, "\t\tStart:\t%X\n", block->start);
        serial_log(SER_LOG_USERHEAP, "\t\tEnd:\t%X\n", block->end);
        serial_log(SER_LOG_USERHEAP, "\t\tRegion:\t%X\n", block->reg);
    }

    serial_log(SER_LOG_USERHEAP, "@@ ENDING USER HEAP DUMP    @@@@@@@@@@@\n");
}
#endif

userHeap_t* userHeapSc_create(size_t initSize)
{
    userHeap_t* ret = userHeap_create(currentTask->pageDirectory, initSize);
    if (ret)
        list_append(currentTask->heaps, ret);
    return ret;
}

void userHeapSc_destroy(userHeap_t* heap)
{
    // Check wether the handle was manipulated
    if ((uint32_t)heap < 0xBF000000 || heap->magic != 0xABC0FFEE)
        return; // TODO: Maybe kill a task trying to pass an invalid pointer

    list_delete(currentTask->heaps, list_find(currentTask->heaps, heap));

    userHeap_destroy(heap);
}

void* userHeapSc_alloc(userHeap_t* heap, size_t size)
{
    // Check wether the handle was manipulated
    if ((uint32_t)heap < 0xBF000000 || heap->magic != 0xABC0FFEE)
        return (0); // TODO: Maybe kill a task trying to pass an invalid pointer

    return userHeap_alloc(heap, size);
}

void userHeapSc_free(userHeap_t* heap, void* address)
{
    // Check wether the handle was manipulated
    if ((uint32_t)heap < 0xBF000000 || heap->magic != 0xABC0FFEE)
        return; // TODO: Maybe kill a task trying to pass an invalid pointer

    userHeap_free(heap, address);
}

void userHeap_cleanUp(task_t* task)
{
    for (dlelement_t* listEntry = task->heaps->head; listEntry; listEntry = listEntry->next)
        userHeap_destroy(listEntry->data);

    list_free(task->heaps);
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
