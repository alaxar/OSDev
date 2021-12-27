#ifndef DESCRIPTOR_TABLES_H
#define DESCRIPTOR_TABLES_H

#include "os.h"


/************************/
/*  Parameter "access"  */
/************************/

// "Segment Present" bit 7
#define VALID                0x80

// "Deskriptor Privilege Level" bit 6:5
#define RING_0               0x00
#define RING_1               0x20
#define RING_2               0x40
#define RING_3               0x60

// "Segment" bit 4
#define SYSTEM               0x00
#define CODE_DATA_STACK      0x10

// "Descriptor Type" bit 3:1
#define DATA_READ_ONLY       0x0  // ....000.
#define DATA_READ_WRITE      0x2  // ....001.
#define STACK_READ_ONLY      0x4  // ....010.
#define STACK_READ_WRITE     0x6  // ....011.
#define CODE_EXEC_ONLY       0x8  // ....100.
#define CODE_EXEC_READ       0xA  // ....101.
#define CODE_EXEC_ONLY_CONF  0xC  // ....110.
#define CODE_EXEC_READ_CONF  0xE  // ....111.

// "Segment Accessed" bit 0
#define SEGM_ACCESSED        0x1

/**********************/
/*  Parameter "gran"  */
/**********************/

// "granularity"            bit 7
#define _BYTE_              0x00
#define _4KB_               0x80

// "Default Operation Size" bit 6
#define USE16               0x00
#define USE32               0x40


// Defines a GDT entry
typedef struct
{
    uint16_t limit_low;
    uint16_t base_low;
    uint8_t  base_middle;
    uint8_t  access;
    uint8_t  granularity;
    uint8_t  base_high;
} __attribute__((packed)) GDTentry_t;

typedef struct
{
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) GDTptr_t;


// Task State Segment (TSS)
typedef struct
{
    uint32_t prev_tss;   // The previous TSS - if we used hardware task switching this would form a linked list.
    uint32_t esp0;       // The stack pointer to load when we change to kernel mode.
    uint32_t ss0;        // The stack segment to load when we change to kernel mode.
    uint32_t esp1;       // Unused...
    uint32_t ss1;
    uint32_t esp2;
    uint32_t ss2;
    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;
    uint32_t es;         // The value to load into ES when we change to kernel mode.
    uint32_t cs;         // The value to load into CS when we change to kernel mode.
    uint32_t ss;         // The value to load into SS when we change to kernel mode.
    uint32_t ds;         // The value to load into DS when we change to kernel mode.
    uint32_t fs;         // The value to load into FS when we change to kernel mode.
    uint32_t gs;         // The value to load into GS when we change to kernel mode.
    uint32_t ldt;        // Unused...
    uint16_t trap;
    uint16_t iomap_base;
} __attribute__((packed)) TSSentry_t;


void idt_install(void); // c.f. interrupts.asm
void gdt_install(void);
void gdt_setGate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran);
void gdt_flush(GDTptr_t*); // c.f. flush.asm
void tss_write(int32_t num, uint16_t ss0, uint32_t esp0);
void tss_flush(void); // c.f. flush.asm
void tss_switch(uint32_t esp0, uint32_t esp, uint32_t ss); // Used by task_switch


#endif
