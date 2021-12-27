#ifndef IRQ_H
#define IRQ_H

#include "os.h"


#define PIC_MASTER_CMD  0x20
#define PIC_MASTER_DATA 0x21
#define PIC_SLAVE_CMD   0xA0
#define PIC_SLAVE_DATA  0xA1

#define PIC_EOI         0x20  // End-of-interrupt (command code)


struct cdi_device;
struct pciDev;

typedef enum
{
    ISR_invalidOpcode = 6,
    ISR_NM            = 7,
    ISR_GPF           = 13,
    ISR_PF            = 14,
} ISR_NUM_t;

typedef enum
{
    IRQ_TIMER         = 0,
    IRQ_KEYBOARD      = 1,
    IRQ_FLOPPY        = 6,
    IRQ_MOUSE         = 12,
    IRQ_ATA_PRIMARY   = 14,
    IRQ_ATA_SECONDARY = 15,
    IRQ_SYSCALL       = 95 // PrettyOS SYSCALL_NUMBER 127 minus 32 // cf. interrupts.asm
} IRQ_NUM_t;

// This defines what the stack looks like after an ISR was running
typedef struct
{
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
} __attribute__((packed)) registers_t;


void isr_install(void);
uint32_t irq_handler(uintptr_t esp);
void irq_installHandler(IRQ_NUM_t irq, void (*handler)(registers_t*));
void irq_installCDIHandler(IRQ_NUM_t irq, void (*handler)(struct cdi_device*), struct cdi_device* device);
void irq_installPCIHandler(IRQ_NUM_t irq, void (*handler)(registers_t*, struct pciDev*), struct pciDev* device);
void irq_uninstallHandler(IRQ_NUM_t irq, void* handler);

void irq_resetCounter(IRQ_NUM_t number);
bool waitForIRQ(IRQ_NUM_t number, uint32_t timeout); // Call irq_resetCounter before to reset counter. Returns false in case of timeout.
bool irq_unlockTask(void* data);


void pic_remap(void);
void pic_disable(void);

#ifdef _IRQ_DIAGNOSIS_
void* irq_getNumberOfInterrupts(void);
#endif

#endif
