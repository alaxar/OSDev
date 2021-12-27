/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "irq.h"
#include "util/util.h"
#include "tasking/task.h"
#include "tasking/vm86.h"
#include "cpu.h"
#include "kheap.h"
#include "hid/keyboard.h"
#include "apic.h"
#include "descriptor_tables.h"
#include "pci.h"


typedef enum
{
    IHT_DEFAULT, IHT_PCI, IHT_CDI
} irq_handlerType_t;

typedef struct
{
    union
    {
        pciDev_t* pciDev;
        struct cdi_device* cdiDev;
    } data;
    union
    {
        void (*def)(registers_t*);            // Interrupt handler used per default
        void (*pci)(registers_t*, pciDev_t*); // Interrupt handler used for PCI devices
        void (*cdi)(struct cdi_device*);      // Interrupt handler used for CDI devices
    } func;
    irq_handlerType_t type;
} irq_handler_t;

static struct irq
{
    size_t calls;             // Counts all interrupts of this number
    size_t handlerCount;      // Counts the number of handlers assigned to this IRQ. Used to determine, which of the members of following union is valid
    union
    {
        irq_handler_t handler; // Single IRQ handler
        list_t handlers;       // List of IRQ handlers. Used if we have more than 1 handler for this IRQ number
    } handler;
} interrupts[256] = {}; // Array of function pointers handling custom ir handlers for a given ir

#ifdef _IRQ_DIAGNOSIS_
static uint32_t numberOfInterrupts[4] = {0};

void* irq_getNumberOfInterrupts(void)
{
    static uint32_t tmpNumberOfInterrupts[4];
    cli();
    memcpy(tmpNumberOfInterrupts, numberOfInterrupts, 4 * sizeof(uint32_t));
    memset(numberOfInterrupts, 0, 4 * sizeof(uint32_t));
    sti();
    return tmpNumberOfInterrupts;
}
#endif

// Add an IRQ handler
static void installHandler(struct irq* irq, void* handler, irq_handlerType_t type, void* data)
{
    if (irq->handlerCount == 0) // No handler installed. Install first handler
    {
        irq->handler.handler.func.def    = handler;
        irq->handler.handler.type        = type;
        irq->handler.handler.data.pciDev = data;
    }
    else // Multiple handlers installed.
    {
        if (irq->handlerCount == 1) // One handler installed. We have to create the handlers-List)
        {
            // Save old handler
            dlelement_t* elem = list_alloc_elem(sizeof(irq_handler_t), "irq handler");
            irq_handler_t* tempHandler = elem->data;
            *tempHandler = irq->handler.handler;

            // Create list, insert old handler
            cli(); // There must not occur an irq while the handler is in an invalid state
            list_construct(&irq->handler.handlers);
            list_append_elem(&irq->handler.handlers, elem);
            sti();
        }

        // Create new handler. Insert to list.
        dlelement_t* elem = list_alloc_elem(sizeof(irq_handler_t), "irq handler");
        irq_handler_t* tempHandler = elem->data;
        tempHandler->func.def = handler;
        tempHandler->type = type;
        tempHandler->data.pciDev = data;
        list_append_elem(&irq->handler.handlers, elem);
    }
    irq->handlerCount++;
}

// Add a default IRQ handler
void irq_installHandler(IRQ_NUM_t irq, void (*handler)(registers_t*))
{
    installHandler(interrupts + 32 + irq, handler, IHT_DEFAULT, 0);
}

// Add a cdi IRQ handler
void irq_installCDIHandler(IRQ_NUM_t irq, void (*handler)(struct cdi_device*), struct cdi_device* device)
{
    installHandler(interrupts + 32 + irq, handler, IHT_CDI, device);
}

// Add a pci IRQ handler
void irq_installPCIHandler(IRQ_NUM_t irq, void (*handler)(registers_t*, pciDev_t*), pciDev_t* device)
{
    installHandler(interrupts + 32 + irq, handler, IHT_PCI, device);
}

// Remove an IRQ handler
void irq_uninstallHandler(IRQ_NUM_t irq, void* handler)
{
    irq += 32;
    if (interrupts[irq].handlerCount == 1) // We delete the last IRQ handler assigned to this IRQ number
    {
        if (handler == interrupts[irq].handler.handler.func.def)
        {
            interrupts[irq].handler.handler.func.def = 0;
            interrupts[irq].handlerCount = 0;
        }
    }
    else
    {
        for (dlelement_t* e = interrupts[irq].handler.handlers.head; e != 0; e = e->next)
        {
            irq_handler_t* ihandler = e->data;
            if (ihandler->func.def == handler) // Found handler -> delete it
            {
                list_delete(&interrupts[irq].handler.handlers, e);
                break;
            }
        }
        interrupts[irq].handlerCount--;
        if (interrupts[irq].handlerCount == 1) // There remains just one IRQ handler
        {
            irq_handler_t temp = *(irq_handler_t*)interrupts[irq].handler.handlers.head->data;
            cli(); // There must not occur an irq while the handler is in an invalid state
            list_destruct(&interrupts[irq].handler.handlers);
            interrupts[irq].handler.handler = temp;
            sti();
        }
    }
}


void irq_resetCounter(IRQ_NUM_t number)
{
    interrupts[number+32].calls = 0;
}

bool waitForIRQ(IRQ_NUM_t number, uint32_t timeout)
{
    if (timeout > 0)
        return (scheduler_blockCurrentTask(BL_INTERRUPT, (void*)(number+32), max(1, timeout)));
    else
    {
        scheduler_blockCurrentTask(BL_INTERRUPT, (void*)(number+32), 0);
        return (true);
    }
}

bool irq_unlockTask(void* data)
{
    return (interrupts[(size_t)data].calls > 0);
}


// Message string corresponding to the exception number 0-31: exceptionMessages[interrupt_number]
static const char* const exceptionMessages[] =
{
    "Division By Zero",        "Debug",                         "Non Maskable Interrupt",    "Breakpoint",
    "Into Detected Overflow",  "Out of Bounds",                 "Invalid Opcode",            "No Coprocessor",
    "Double Fault",            "Coprocessor Segment Overrun",   "Bad TSS",                   "Segment Not Present",
    "Stack Fault",             "General Protection Fault",      "Page Fault",                "Unknown Interrupt",
    "Coprocessor Fault",       "Alignment Check",               "Machine Check",             "SIMD Exception",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved",
    "Reserved",                "Reserved",                      "Reserved",                  "Reserved"
};

static void quitTask(void)
{
    if (currentTask == &kernelTask)
    {
        cli();
        console_current = &kernelConsole;
        console_display(11); // We need to switch to the kernel console before the system is halted. Otherwise the output is invisible.
        printfe("\n| <Severe Failure - Kernel Halted> |");
        hlt();
    }
    else
    {
        if (currentTask->type != VM86) {
            printfe("\n| <Severe Failure - Task %u Halted> |"
                "\n         Press key to exit.", currentTask->pid);
            sti();
            getch();
        }
        else
            printfe("\n| <Severe Failure - VM86-task %u Aborted> |", currentTask->pid);
        exit();
    }
    for (;;);
}

static void defaultError(registers_t* r)
{
    printfe("\n\n%s!", exceptionMessages[r->int_no]);
    printf("\nerr_code: %u  eip: %Xh\n", r->err_code, r->eip);
    printf("edi: %Xh esi: %Xh ebp: %Xh eax: %Xh ebx: %Xh ecx: %Xh edx: %Xh\n", r->edi, r->esi, r->ebp, r->eax, r->ebx, r->ecx, r->edx);
    printf("cs: %xh  ds: %xh  es: %xh  fs: %xh  gs: %xh  ss: %xh\n", r->cs, r->ds, r->es, r->fs, r->gs, r->ss);
    printf("eflags: %Xh  useresp: %Xh\n", r->eflags, r->useresp);

    printStackTrace((void*)r->eip, (void*)r->ebp);

    quitTask();
}

static void invalidOpcode(registers_t* r)
{
    printfe("\n\n%s!", exceptionMessages[r->int_no]);
    printf("\nerr_code: %u  eip: %Xh  instruction: %yh\n", r->err_code, r->eip, *(uint8_t*)FP_TO_LINEAR(r->cs, r->eip));
    printf("edi: %Xh esi: %Xh ebp: %Xh eax: %Xh ebx: %Xh ecx: %Xh edx: %Xh\n", r->edi, r->esi, r->ebp, r->eax, r->ebx, r->ecx, r->edx);
    printf("cs: %xh  ds: %xh  es: %xh  fs: %xh  gs: %xh  ss: %xh\n", r->cs, r->ds, r->es, r->fs, r->gs, r->ss);
    printf("eflags: %Xh  useresp: %Xh\n", r->eflags, r->useresp);

    printStackTrace((void*)r->eip, (void*)r->ebp);

    quitTask();
}

static void NM_fxsr(registers_t* r) // -> FPU (with support for SSE)
{
  #ifdef _DIAGNOSIS_
    printf("#NM (fxsr): FPU is used. currentTask: %Xh\n", currentTask);
  #endif

    __asm__ volatile("clts"); // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM

    // save FPU data ...
    if (FPUTask) // fxsave to FPUTask->FPUptr
        __asm__("fxsave (%0)" : : "r"(FPUTask->FPUptr));

    FPUTask = currentTask; // store the last task using FPU

    // restore FPU data ...
    if (currentTask->FPUptr) // fxrstor from currentTask->FPUptr
        __asm__ volatile("fxrstor (%0)" : : "r"(currentTask->FPUptr));
    else // allocate memory to save the content of the FPU registers
        currentTask->FPUptr = malloc(512, 4, "FPUptr"); // C.f. Intel Manual vol. 2A
}

static void NM(registers_t* r) // -> FPU
{
  #ifdef _DIAGNOSIS_
    printf("#NM: FPU is used. currentTask: %Xh\n", currentTask);
  #endif

    __asm__ volatile("clts"); // CLearTS: reset the TS bit (no. 3) in CR0 to disable #NM

    // save FPU data ...
    if (FPUTask) // fsave or fnsave to FPUTask->FPUptr
        __asm__("fsave (%0)" : : "r"(FPUTask->FPUptr));

    FPUTask = currentTask; // store the last task using FPU

    // restore FPU data ...
    if (currentTask->FPUptr) // frstor from currentTask->FPUptr
        __asm__ volatile("frstor (%0)" : : "r"(currentTask->FPUptr));
    else // allocate memory to save the content of the FPU registers
        currentTask->FPUptr = malloc(108, 4, "FPUptr"); // 80 Bytes (r0..r7) + 28 Bytes (environment image). C.f. Intel Manual vol. 2A
}

static void GPF(registers_t* r) // VM86
{
    // http://en.wikipedia.org/wiki/FLAGS_register_%28computing%29
    if (r->eflags & EFLAG_VM)    // it is a VM86-task
    {
        if (!vm86_sensitiveOpcodehandler(r))
        {
            uint8_t* ip = FP_TO_LINEAR(r->cs, r->eip-1);
            printfe("\nvm86: sensitive opcode error: %y. (prefix: %y)\n", ip[0], ip[-1]);
            quitTask();
        }
    }
    else
        defaultError(r);
}

static void PF(registers_t* r)
{
    uint32_t faulting_address;
    __asm__("mov %%cr2, %0" : "=r"(faulting_address)); // faulting address <== CR2 register

    // The error code gives us details of what happened.
    bool pres  = !(r->err_code & BIT(0)); // Page not present
    bool rw    =   r->err_code & BIT(1);  // Write operation?
    bool us    =   r->err_code & BIT(2);  // Processor was in user-mode?
    bool res   =   r->err_code & BIT(3);  // Overwritten CPU-reserved bits of page entry?
    bool id    =   r->err_code & BIT(4);  // Caused by an instruction fetch?

    // Output an error message.
    printfe("\n\n%s!", exceptionMessages[r->int_no]);
    textColor(IMPORTANT);
    printf("\nAddress: %Xh   EIP: %Xh", faulting_address, r->eip);
    textColor(TEXT);
    if (pres) printf("\npage not present");
    if (rw)   printf("\nread-only - write operation");
    if (us)   printf("\nuser-mode");
    if (res)  printf("\noverwritten CPU-reserved bits of page entry");
    if (id)   printf("\ncaused by an instruction fetch");

    printStackTrace((void*)r->eip, (void*)r->ebp);

    quitTask();
}


void isr_install(void)
{
    // Installing ISR-Routines
    for (uint8_t i = 0; i < 32; i++)
    {
        interrupts[i].handlerCount = 1;
        interrupts[i].handler.handler.type = IHT_DEFAULT;
        interrupts[i].handler.handler.func.def = &defaultError; // If nothing else is specified, the default handler is called
    }

    interrupts[ISR_invalidOpcode].handler.handler.func.def = &invalidOpcode;
    if (cpu_supports(CF_FXSR))
        interrupts[ISR_NM].handler.handler.func.def = &NM_fxsr;
    else
        interrupts[ISR_NM].handler.handler.func.def = &NM;
    interrupts[ISR_GPF].handler.handler.func.def = &GPF;
    interrupts[ISR_PF].handler.handler.func.def = &PF;

    // Load IDT
    idt_install();
}

uint32_t irq_handler(uintptr_t esp)
{
    task_t* oldTask = currentTask; // Save old task to be able to restore attr in case of task_switch
    uint8_t attr    = currentTask->attrib;  // Save the attrib so that we do not get color changes after the interrupt, if it has changed the attrib
    console_current = kernelTask.console;   // The output is expected to appear in the kernel's console. Exception: Syscalls (cf. syscall.c)

    registers_t* r = (registers_t*)esp;

  #ifdef _IRQ_DIAGNOSIS_
    numberOfInterrupts[0]++;        // all interrupts
    if (r->int_no == 0x20) numberOfInterrupts[1]++;    // timer
    if (r->int_no == 0x7E) numberOfInterrupts[2]++;    // switch_context
    if (r->int_no == 0x7F) numberOfInterrupts[3]++;    // syscall
  #endif

    if ((r->int_no == 0x20 && task_switching) || r->int_no == 0x7E) // timer interrupt or function switch_context
        esp = scheduler_taskSwitch(esp); // get new task's esp from scheduler

    interrupts[r->int_no].calls++;

    if (interrupts[r->int_no].handlerCount == 1) // One handler registered for this interrupt
    {
        switch (interrupts[r->int_no].handler.handler.type)
        {
            case IHT_DEFAULT:
                interrupts[r->int_no].handler.handler.func.def(r); // Execute handler
                break;
            case IHT_PCI:
                //if (pci_deviceSentInterrupt(interrupts[r->int_no].handler.handler.data.pciDev)) // TODO: Why does it not work? Bit 3 of the PCI status register is not set at interrupt on VBox and real hardware
                    interrupts[r->int_no].handler.handler.func.pci(r, interrupts[r->int_no].handler.handler.data.pciDev); // Execute PCI handler
                break;
            case IHT_CDI:
                interrupts[r->int_no].handler.handler.func.cdi(interrupts[r->int_no].handler.handler.data.cdiDev); // Execute CDI handler
                break;
        }
    }
    else if (interrupts[r->int_no].handlerCount > 1) // More than one handler registered
    {
        for (dlelement_t* e = interrupts[r->int_no].handler.handlers.head; e != 0; e = e->next) // First loop: Try to find a PCI handler to call
        {
            irq_handler_t* handler = e->data;
            if (handler->type == IHT_PCI/* && pci_deviceSentInterrupt(handler->data.pciDev)*/) // TODO: Why does it not work? Bit 3 of the PCI status register is not set at interrupt on VBox and real hardware
            {
                handler->func.pci(r, handler->data.pciDev); // Execute PCI handler
                //goto HANDLED; // Disabled, because pci_deviceSentInterrupt is disabled, too.
            }
        }
        for (dlelement_t* e = interrupts[r->int_no].handler.handlers.head; e != 0; e = e->next) // Second loop: Also accept default and CDI handlers. TODO: Move CDI handlers to first loop (check pci device for interrupt)
        {
            irq_handler_t* handler = e->data;
            if (handler->type == IHT_DEFAULT)
            {
                handler->func.def(r); // Execute handler
                goto HANDLED;
            }
            else if (handler->type == IHT_CDI)
            {
                handler->func.cdi(handler->data.cdiDev);
                goto HANDLED;
            }
        }
    }

HANDLED:
    if (r->int_no >= 32)
    {
        lapic_eoi(r->int_no);
        if (r->int_no <= 32 + 16 && !ioapic_installed())
        {
            if (r->int_no >= (32 + 8))            // IRQs from slave PIC have to be quit by EOI to both PICs
                outportb(PIC_SLAVE_CMD, PIC_EOI); // Issue EOI on slave PIC
            outportb(PIC_MASTER_CMD, PIC_EOI);    // Issue EOI on master PIC
        }
    }

    console_current = currentTask->console;
    if (r->int_no != 0x7F) // Syscalls (especially textColor) should be able to change color. HACK: Can this be solved nicer?
        oldTask->attrib = attr;
    return (esp);
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
