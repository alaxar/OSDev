/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

// code derived on basic proposal at http://osdev.berlios.de/v86.html
// cf. "The Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volumes 3A", 17.2 VIRTUAL-8086 MODE
/*
17.2.7 Sensitive Instructions
When an IA-32 processor is running in virtual-8086 mode, the CLI, STI, PUSHF, POPF, INT n,
and IRET instructions are sensitive to IOPL. The IN, INS, OUT, and OUTS instructions,
which are sensitive to IOPL in protected mode, are not sensitive in virtual-8086 mode.
The CPL is always 3 while running in virtual-8086 mode; if the IOPL is less than 3, an
attempt to use the IOPL-sensitive instructions listed above triggers a general-protection
exception (#GP). These instructions are sensitive to IOPL to give the virtual-8086 monitor
a chance to emulate the facilities they affect.
*/

#include "vm86.h"
#include "util/util.h"
#include "task.h"
#include "kheap.h"
#ifdef _VM_DIAGNOSIS_
#include "serial.h"
#endif


static volatile bool v86_if = false;


bool vm86_sensitiveOpcodehandler(registers_t* ctx)
{
    uint8_t*  ip           = FP_TO_LINEAR(ctx->cs, ctx->eip);
  #ifdef _VM_DIAGNOSIS_
    serial_log(SER_LOG_VM86, "%X ", ip);
  #endif

    const volatile uint16_t* ivt = 0; // Has to be volatile to work with current clang
    uint16_t* stack        = (uint16_t*)FP_TO_LINEAR(ctx->ss, ctx->useresp);
    uint32_t* stack32      = (uint32_t*)stack;
    bool isOperand32       = false;
    bool isAddress32       = false;
    bool repeat            = false;
    uint32_t* segmentreg   = 0;

    // regarding opcodes, cf. "The Intel® 64 and IA-32 Architectures Software Developer’s Manual, Volumes 2A & 2B"

    // Detect prefixes
    switch (ip[0]) // Instruction prefix
    {
        case 0xF3:
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "rep %u ", ctx->ecx);
          #endif
            repeat = true;
            ip++;
            ctx->eip++;
            break;
        case 0xF0: case 0xF2:
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "unsupported prefix %x ", ip[0]);
          #endif
            ip++;
            ctx->eip++;
            break;
        default:
            break;
    }
    if (ip[0] == 0x66) // Operand size
    {
      #ifdef _VM_DIAGNOSIS_
        serial_log(SER_LOG_VM86, "o32 ");
      #endif
        isOperand32 = true;
        ip++;
        ctx->eip++;
    }
    if (ip[0] == 0x67) // Address size
    {
      #ifdef _VM_DIAGNOSIS_
        serial_log(SER_LOG_VM86, "a32 ");
      #endif
        isAddress32 = true;
        ip++;
        ctx->eip++;
    }
    switch (ip[0]) // Segment override
    {
        case 0x2E: // CS
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "CS ");
          #endif
            segmentreg = &ctx->cs;
            ip++;
            ctx->eip++;
            break;
        case 0x36: // SS
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "SS ");
          #endif
            segmentreg = &ctx->ss;
            ip++;
            ctx->eip++;
            break;
        case 0x3E: // DS
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "DS ");
          #endif
            segmentreg = &ctx->ds;
            ip++;
            ctx->eip++;
            break;
        case 0x26: // ES
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "ES ");
          #endif
            segmentreg = &ctx->es;
            ip++;
            ctx->eip++;
            break;
        case 0x64: // FS
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "FS ");
          #endif
            segmentreg = &ctx->fs;
            ip++;
            ctx->eip++;
            break;
        case 0x65: // GS
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "GS ");
          #endif
            segmentreg = &ctx->gs;
            ip++;
            ctx->eip++;
            break;
        default:
            break;
    }

    switch (ip[0]) // Analyze opcode
    {
        case 0x9C: // PUSHF
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "pushf\n");
          #endif
            if (isOperand32)
            {
                ctx->useresp = (ctx->useresp - 4) & 0xFFFF;
                stack32--;
                stack32[0] = ctx->eflags;

                if (v86_if)
                {
                    stack32[0] |= EFLAG_IF;
                }
                else
                {
                    stack32[0] &= ~EFLAG_IF;
                }
            }
            else
            {
                ctx->useresp = (ctx->useresp - 2) & 0xFFFF;
                stack--;
                stack[0] = (uint16_t)ctx->eflags;

                if (v86_if)
                {
                    stack[0] |= EFLAG_IF;
                }
                else
                {
                    stack[0] &= ~EFLAG_IF;
                }
            }
            ctx->eip++;
            return true;

        case 0x9D: // POPF
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "popf\n");
          #endif

            if (isOperand32)
            {
                ctx->eflags = (ctx->eflags & EFLAGS_MASK) | (stack32[0] & ~EFLAGS_MASK);
                v86_if = (stack32[0] & EFLAG_IF) != 0;
                ctx->useresp = (ctx->useresp + 4) & 0xFFFF;
            }
            else
            {
                ctx->eflags = (ctx->eflags & EFLAGS_MASK) | (stack[0] & ~EFLAGS_MASK);
                v86_if = (stack[0] & EFLAG_IF) != 0;
                ctx->useresp = (ctx->useresp + 2) & 0xFFFF;
            }
            ctx->eip++;
            return true;

        case 0xE6: // OUT imm8, AX and OUT imm8, EAX
            if (!isOperand32)
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "outw8\n");
              #endif
                outportw(ip[1], ctx->eax);
            }
            else
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "outl8\n");
              #endif
                outportl(ip[1], ctx->eax);
            }
            ctx->eip += 2;
            return true;

        case 0xE7: // OUT imm8, AL
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "outb8\n");
          #endif
            outportb(ip[1], ctx->eax);
            ctx->eip += 2;
            return true;

        case 0xEF: // OUT DX, AX and OUT DX, EAX
            if (!isOperand32)
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "outw\n");
              #endif
                outportw(ctx->edx, ctx->eax);
            }
            else
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "outl\n");
              #endif
                outportl(ctx->edx, ctx->eax);
            }
            ctx->eip++;
            return true;

        case 0xEE: // OUT DX, AL
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "outb\n");
          #endif
            outportb(ctx->edx, ctx->eax);
            ctx->eip++;
            return true;

        case 0xE4: // IN AL, imm8
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "inb8\n");
          #endif
            ctx->eax = (ctx->eax & 0xFFFFFF00) | inportb(ip[1]);
            ctx->eip += 2;
            break;

        case 0xE5: // IN AX,imm8 and IN EAX,imm8
            if (!isOperand32)
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "inw8\n");
              #endif
                ctx->eax = (ctx->eax & 0xFFFF0000) | inportw(ip[1]);
            }
            else
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "inl8\n");
              #endif
                ctx->eax = inportl(ip[1]);
            }
            ctx->eip += 2;
            return true;

        case 0xED: // IN AX,DX and IN EAX,DX
            if (!isOperand32)
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "inw\n");
              #endif
                ctx->eax = (ctx->eax & 0xFFFF0000) | inportw(ctx->edx);
            }
            else
            {
              #ifdef _VM_DIAGNOSIS_
                serial_log(SER_LOG_VM86, "inl\n");
              #endif
                ctx->eax = inportl(ctx->edx);
            }
            ctx->eip++;
            return true;

        case 0xEC: // IN AL,DX
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "inb\n");
          #endif
            ctx->eax = (ctx->eax & 0xFFFFFF00) | inportb(ctx->edx);
            ctx->eip++;
            return true;

        case 0x6C: // INSB
        {
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "insb\n");
          #endif
            uint16_t repetitions = 1;
            if (repeat) {
                repetitions = ctx->ecx;
                ctx->ecx = 0;
            }
            for (uint16_t i = 0; i < repetitions; i++) {
                *(uint8_t*)FP_TO_LINEAR(ctx->es, ctx->edi) = inportb(ctx->edx);
                if (ctx->eflags & EFLAG_DF)
                    ctx->edi--;
                else
                    ctx->edi++;
            }
            ctx->eip++;
            return true;
        }

        case 0x6D: // INSW/D
        {
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "ins\n");
          #endif
            uint16_t repetitions = 1;
            if (repeat) {
                repetitions = ctx->ecx;
                ctx->ecx = 0;
            }
            for (uint16_t i = 0; i < repetitions; i++) {
                if (isAddress32) {
                    *(uint32_t*)FP_TO_LINEAR(ctx->es, ctx->edi) = inportl(ctx->edx);
                    if (ctx->eflags & EFLAG_DF)
                        ctx->edi -= 4;
                    else
                        ctx->edi += 4;
                }
                else {
                    *(uint16_t*)FP_TO_LINEAR(ctx->es, ctx->edi) = inportw(ctx->edx);
                    if (ctx->eflags & EFLAG_DF)
                        ctx->edi -= 2;
                    else
                        ctx->edi += 2;
                }
            }
            ctx->eip++;
            return true;
        }

        case 0x6E: // OUTSB
        {
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "outsb\n");
          #endif
            uint16_t repetitions = 1;
            if (repeat) {
                repetitions = ctx->ecx;
                ctx->ecx = 0;
            }
            if (segmentreg == 0)
                segmentreg = &ctx->ds;
            for (uint16_t i = 0; i < repetitions; i++) {
                outportb(ctx->edx, *(uint8_t*)FP_TO_LINEAR(*segmentreg, ctx->esi));
                if (ctx->eflags & EFLAG_DF)
                    ctx->edi--;
                else
                    ctx->edi++;
            }
            ctx->eip++;
            return true;
        }

        case 0x6F: // OUTSW/D
        {
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "outs");
          #endif
            uint16_t repetitions = 1;
            if (repeat) {
                repetitions = ctx->ecx;
                ctx->ecx = 0;
            }
            if (segmentreg == 0)
                segmentreg = &ctx->ds;
            for (uint16_t i = 0; i < repetitions; i++) {
                if (isAddress32) {
                    outportl(ctx->edx, *(uint32_t*)FP_TO_LINEAR(*segmentreg, ctx->esi));
                    if (ctx->eflags & EFLAG_DF)
                        ctx->edi -= 4;
                    else
                        ctx->edi += 4;
                }
                else {
                    outportw(ctx->edx, *(uint16_t*)FP_TO_LINEAR(*segmentreg, ctx->esi));
                    if (ctx->eflags & EFLAG_DF)
                        ctx->edi -= 2;
                    else
                        ctx->edi += 2;
                }
            }
            ctx->eip++;
            return true;
        }
        case 0xCD: // INT imm8
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "interrupt %X\n", ip[1]);
          #endif
            stack -= 3;
            ctx->useresp = (ctx->useresp - 6) & 0xFFFF;
            stack[0] = ctx->eip + 2;
            stack[1] = ctx->cs;
            stack[2] = ctx->eflags;

            if (v86_if)
            {
                stack[2] |= EFLAG_IF;
            }
            else
            {
                stack[2] &= ~EFLAG_IF;
            }
            ctx->eip = ivt[2 * ip[1]    ];
            ctx->cs =  ivt[2 * ip[1] + 1];
            return true;

        case 0xCF: // IRET
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "iret\n");
          #endif
            ctx->eip     = stack[0];
            ctx->cs      = stack[1];
            ctx->eflags  = (ctx->eflags & EFLAGS_MASK) | (stack[2] & ~EFLAGS_MASK);
            ctx->useresp = (ctx->useresp + 6) & 0xFFFF;

            v86_if = (stack[2] & EFLAG_IF) != 0;
            return true;

        case 0xFA: // CLI
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "cli\n");
          #endif
            v86_if = false;
            ctx->eip++;
            return true;

        case 0xFB: // STI
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "sti\n");
          #endif
            v86_if = true;
            ctx->eip++;
            return true;

        case 0xF4: // HLT
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "hlt\n");
          #endif
            exit();
            return true;

        default: // should not happen!
          #ifdef _VM_DIAGNOSIS_
            serial_log(SER_LOG_VM86, "error: unhandled opcode %y\n", ip[0]);
          #endif
            ctx->eip++;
            return false;
    }
    return false;
}

void vm86_initPageDirectory(pageDirectory_t* pd, void* address, void* code, size_t size)
{
    pd->tables[0] = malloc(sizeof(pageTable_t), PAGESIZE, "VM86 page table");
    memcpy(pd->tables[0], kernelPageDirectory->tables[0], sizeof(pageTable_t));
    for (uint16_t i=0; i<256; i++) // Make first 1 MiB accessible
        pd->tables[0]->pages[i] |= MEM_WRITE | MEM_USER; // address = i*0x1000
    pd->codes[0] = paging_getPhysAddr(pd->tables[0]) | MEM_WRITE | MEM_USER | MEM_PRESENT;

    // Allocate memory to store vm86 code and stack
    void* paddress = (void*)alignDown((uintptr_t)address, PAGESIZE);
    size_t psize = alignUp(size, PAGESIZE);
    ASSERT(psize == PAGESIZE); // TODO: Support other sizes; Requires setting setting stack pointer dynamically
    paging_freeVirtMem(pd, paddress, psize / PAGESIZE + 1);
    paging_alloc(pd, paddress, psize + PAGESIZE, (MEMFLAGS_t)(MEM_USER | MEM_WRITE));

    // Copy vm86 code
    cli();
    paging_switch(pd);
    memcpy(address, code, size);
    paging_switch(currentTask->pageDirectory);
    sti();

    // Make VM86 code read-only
    paging_setFlags(pd, paddress, psize, (MEMFLAGS_t)(MEM_USER));
}

void vm86_executeSync(pageDirectory_t* pd, void (*entry)(void))
{
    task_t* vm86task = create_vm86_task(pd, entry, "VM86");
    cli(); // To avoid a race condition, we try to avoid task switches while creating task and initializing the blocker
    scheduler_insertTask(vm86task);
    waitForTask(vm86task, 0);
    sti();
}


/*
* Copyright (c) 2010-2017 The PrettyOS Project. All rights reserved.
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
