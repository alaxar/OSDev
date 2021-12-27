#ifndef VM86_H
#define VM86_H

// code derived on basic proposal at http://osdev.berlios.de/v86.html

#include "paging.h"
#include "irq.h"

// eflags
#define EFLAG_IF     BIT(9)
#define EFLAG_DF     BIT(10)
#define EFLAG_VM     BIT(17)
#define EFLAGS_MASK  0xFFFFF32A // Bits that must not be modified
#define FP_TO_LINEAR(seg, off) ((void*) ((((uint16_t) (seg)) << 4) + ((uint16_t) (off))))


bool vm86_sensitiveOpcodehandler(registers_t* ctx);
void vm86_initPageDirectory(pageDirectory_t* pd, void* address, void* code, size_t size);
void vm86_executeSync(pageDirectory_t* pd, void (*entry)(void));


#endif
