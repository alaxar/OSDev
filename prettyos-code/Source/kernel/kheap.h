#ifndef KHEAP_H
#define KHEAP_H

#include "os.h"

#define HEAP_ALIGNMENT_MASK 0x00FFFFFF
#define HEAP_WITHIN_PAGE BIT(24)
#define HEAP_WITHIN_64K BIT(25)
#define HEAP_CONTINUOUS BIT(31)

void* heap_getCurrentEnd(void);
void  heap_install(void);
void  heap_logRegions(void);
void* malloc(size_t size, uint32_t alignment, const char* comment);
void  free(void* addr);


#endif
