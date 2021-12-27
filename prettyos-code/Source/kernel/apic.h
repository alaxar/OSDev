#ifndef APIC_H
#define APIC_H

#include "os.h"

#define IA32_APIC_BASE_MSR          0x1B
#define IA32_APIC_BASE_BSP          0x100
#define IA32_APIC_BASE_MSR_ENABLE   0x800

#define APIC_SW_ENABLE              BIT(8)
#define APIC_CPUFOCUS               BIT(9)
#define APIC_NMI                    BIT(10)
#define APIC_LEVEL                  BIT(13)
#define APIC_LOW                    BIT(15)
#define APIC_INTERRUPTDISABLED      BIT(16)

#define TMR_PERIODIC                BIT(17)

#define APIC_EXT_INT                (BIT(8) | BIT(9) | BIT(10))


// APIC registers (as indices of uint32_t*)
enum {
    APIC_APICID             = 0x20  / 4,
    APIC_APICVERSION        = 0x30  / 4,
    APIC_TASKPRIORITY       = 0x80  / 4,
    APIC_EOI                = 0xB0  / 4, // End of Interrupt
    APIC_LDR                = 0xD0  / 4, // Logical Destination
    APIC_DFR                = 0xE0  / 4, // Destination Format
    APIC_SPURIOUSINTERRUPT  = 0xF0  / 4,
    APIC_ESR                = 0x280 / 4, // Error Status
    APIC_ICRL               = 0x300 / 4, // Interrupt Command Lo
    APIC_ICRH               = 0x310 / 4, // Interrupt Command Hi

    APIC_TIMER              = 0x320 / 4, // LVT Timer Register, cf. 10.5.1 Local Vector Table
    APIC_THERMALSENSOR      = 0x330 / 4, // LVT Thermal Monitor Register, cf. 10.5.1 Local Vector Table
    APIC_PERFORMANCECOUNTER = 0x340 / 4, // LVT Performance Counter Register, cf. 10.5.1 Local Vector Table
    APIC_LINT0              = 0x350 / 4, // LVT LINT0 Register, cf. 10.5.1 Local Vector Table
    APIC_LINT1              = 0x360 / 4, // LVT LINT1 Register, cf. 10.5.1 Local Vector Table
    APIC_ERROR              = 0x370 / 4, // LVT Error Register, cf. 10.5.1 Local Vector Table

    APIC_TIMER_INITCOUNT    = 0x380 / 4,
    APIC_TIMER_CURRENTCOUNT = 0x390 / 4,
    APIC_TIMER_DIVIDECONFIG = 0x3E0 / 4,
};

// IO APIC

/* Offset der memory-mapped Register */
#define IOAPIC_IOREGSEL      0x00
#define IOAPIC_IOWIN         0x10

#define IOAPIC_ID            0x00
#define IOAPIC_VERSION       0x01
#define IOAPIC_ARBITRATIONID 0x02


bool ioapic_available(void);
bool ioapic_install(void);
bool ioapic_installed(void);
#ifdef _IOAPIC_ENABLE_
uint8_t ioapic_getID(void);
#endif

bool lapic_available(void);
bool lapic_install(void);
bool lapic_eoi(uint32_t no);
uint8_t lapic_getID(void);


#endif
