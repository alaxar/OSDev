#ifndef USERHEAP_H
#define USERHEAP_H

#include "paging.h"
#include "tasking/synchronisation.h"
#include "util/list.h"
#include "util/types.h"

typedef struct
{
    uint32_t magic;
    mutex_t lock;
    pageDirectory_t* pageDir;
    list_t regions;
    list_t allocatedBlocks;
    list_t freeBlocks;
} userHeap_t;

userHeap_t* userHeap_create(pageDirectory_t* pageDir, size_t initialSize);
void userHeap_destroy(userHeap_t* heap);
void* userHeap_alloc(userHeap_t* heap, size_t size);
void userHeap_free(userHeap_t* heap, void* address);

void userHeap_cleanUp(task_t* task);

userHeap_t* userHeapSc_create(size_t initSize);
void userHeapSc_destroy(userHeap_t* heap);
void* userHeapSc_alloc(userHeap_t* heap, size_t size);
void userHeapSc_free(userHeap_t* heap, void* address);

#endif
