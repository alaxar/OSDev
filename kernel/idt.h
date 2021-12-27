#ifndef _IDT_HEADER
#define _IDT_HEADER
#include "../include/types.h"
#include "../kernel/low_level.h"

#define KERNEL_MODE 0x8e00 
#define USER_MODE   0xee00

typedef struct InterruptDescriptorTable{
    uint16_t offset_1;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attributes;
    uint16_t offset_2;
}__attribute__((packed)) InterruptDescriptorTable;


struct IDTR {
    uint16_t limit;
    struct InterruptDescriptorTable *base;
}__attribute__((packed));

extern struct InterruptDescriptorTable _idt[256];
extern struct IDTR idtr;

void loadIDT();     // set up IDT by loading IDTR

#endif