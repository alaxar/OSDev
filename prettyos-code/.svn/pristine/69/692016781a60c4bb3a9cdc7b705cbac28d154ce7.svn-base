/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "descriptor_tables.h"
#ifdef _DIAGNOSIS_
#include "video/console.h"
#endif


// GDT
#define NUMBER_GDT_GATES 6 // 0-5: Null, Kernel Code, Kernel Data, User Code, User Data, TSS

// Our GDT
static GDTentry_t gdt[NUMBER_GDT_GATES];

// Setup a descriptor in the Global Descriptor Table
void gdt_setGate(int32_t num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    // Setup the descriptor base address
    gdt[num].base_low    =  base & 0xFFFF;
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    // Setup the descriptor limits
    gdt[num].limit_low   = limit & 0xFFFF;
    gdt[num].granularity = (limit >> 16) & 0x0F;

    // Finally, set up the granularity and access flags
    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

void gdt_install(void)
{
    GDTptr_t gdt_register;

    // Setup the GDT pointer and limit
    gdt_register.limit = (sizeof(GDTentry_t) * NUMBER_GDT_GATES)-1;
    gdt_register.base  = (uint32_t)&gdt;

    // GDT GATES -  desriptors with pointers to the linear memory address
    gdt_setGate(0,0,0,0,0); // NULL descriptor

    //          num base limit    access                                              gran
    gdt_setGate(1,  0,   0xFFFFF, VALID | RING_0 | CODE_DATA_STACK | CODE_EXEC_READ,  _4KB_ | USE32);
    gdt_setGate(2,  0,   0xFFFFF, VALID | RING_0 | CODE_DATA_STACK | DATA_READ_WRITE, _4KB_ | USE32);
    gdt_setGate(3,  0,   0xFFFFF, VALID | RING_3 | CODE_DATA_STACK | CODE_EXEC_READ,  _4KB_ | USE32);
    gdt_setGate(4,  0,   0xFFFFF, VALID | RING_3 | CODE_DATA_STACK | DATA_READ_WRITE, _4KB_ | USE32);

    tss_write(5, 0x10, 0x0); // num, ss0, esp0
    gdt_flush(&gdt_register);
    tss_flush();
}


/// TSS

static TSSentry_t tss = {};

#ifdef _DIAGNOSIS_
static void tss_log(TSSentry_t* tssEntry)
{
    textColor(DATA);
    printf("esp0: %Xh ", tssEntry->esp0);
    printf("ss0: %Xh ", tssEntry->ss0);
    printf("cr3: %Xh ", tssEntry->cr3);
    printf("eip: %Xh ", tssEntry->eip);
    printf("eflags: %Xh ", tssEntry->eflags);
    printf("eax: %Xh ", tssEntry->eax);
    printf("ecx: %Xh ", tssEntry->ecx);
    printf("edx: %Xh ", tssEntry->edx);
    printf("ebx: %Xh ", tssEntry->ebx);
    printf("esp: %Xh ", tssEntry->esp);
    printf("esi: %Xh ", tssEntry->esi);
    printf("edi: %Xh ", tssEntry->edi);
    printf("es: %Xh ", tssEntry->es);
    printf("cs: %Xh ", tssEntry->cs);
    printf("ss: %Xh ", tssEntry->ss);
    printf("ds: %Xh ", tssEntry->ds);
    printf("fs: %Xh ", tssEntry->fs);
    printf("gs: %Xh\n", tssEntry->gs);
    textColor(TEXT);
}
#endif

// Initialise our task state segment structure.
void tss_write(int32_t num, uint16_t ss0, uint32_t esp0)
{
    // Firstly, let's compute the base and limit of our entry into the GDT.
    uint32_t base = (uint32_t)&tss;
    uint32_t limit = sizeof(tss); //http://forum.osdev.org/viewtopic.php?f=1&t=19819&p=155587&hilit=tss_entry#p155587

    // Now, add our TSS descriptor's address to the GDT.
    gdt_setGate(num, base, limit, 0xE9, 0x00);

    tss.ss0  = ss0;  // Set the kernel stack segment.
    tss.esp0 = esp0; // Set the kernel stack pointer.

    tss.cs   = 0x08;
    tss.ss = tss.ds = tss.es = tss.fs = tss.gs = 0x10;
  #ifdef _DIAGNOSIS_
    tss_log(&tss);
  #endif
}

void tss_switch(uint32_t esp0, uint32_t esp, uint32_t ss)
{
    tss.esp0 = esp0;
    tss.esp = esp;
    tss.ss = ss;
}

/*
* Copyright (c) 2009-2015 The PrettyOS Project. All rights reserved.
*
* http://www.c-plusplus.de/forum/viewforum-var-f-is-62.html
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
