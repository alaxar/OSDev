/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "apic.h"
#include "cpu.h"
#include "video/console.h"
#include "util/util.h"
#include "paging.h"
#include "irq.h"
#include "timer.h"
#include "pit.h"
#include "ipc.h"


static volatile uint32_t* lapic_base  = 0;
static volatile uint8_t*  ioapic_base = 0;

#ifdef _IOAPIC_ENABLE_
static uintptr_t phys_ioapic_base;
static void ioapic_remapVector(uint8_t vector, uint32_t mapped, bool level /*f:edge t:level*/, bool low /*t:high f:low*/, bool disabled);
#endif

bool lapic_available(void)
{
    // We need MSR (to initialize Local APIC) and (obviously) APIC.
    return cpu_supports(CF_MSR) && cpu_supports(CF_APIC);
}

bool ioapic_available(void)
{
  #ifdef _IOAPIC_ENABLE_
    int64_t baseaddripc;
    if (ipc_getInt("/PrettyOS/ACPI/madt/ioapic/0/address", &baseaddripc) != IPC_SUCCESSFUL) // Ensure that I/O-APIC is available - this is the case if its address was found in the ACPI tables
        return false;
    phys_ioapic_base = (uintptr_t)baseaddripc;
    return true;
  #else
    return false;
  #endif
}

bool ioapic_installed(void)
{
    return ioapic_base != 0;
}

uint8_t lapic_getID(void)
{
    return lapic_base[APIC_APICID];
}

bool lapic_install(void)
{
    /////////////////////////////////////////////////
    // Local APIC, cf. Intel manual 3A, chapter 10

    uintptr_t phys_apic_base = (uintptr_t)(cpu_MSRread(IA32_APIC_BASE_MSR) & ~0xFFFU); // read APIC base address (ignore bit0-11)
    cpu_MSRwrite(IA32_APIC_BASE_MSR, ((phys_apic_base & ~0xFFFU) | IA32_APIC_BASE_BSP | IA32_APIC_BASE_MSR_ENABLE)); // enable APIC, Bootstrap Processor
    lapic_base = paging_allocMMIO(phys_apic_base, 1);

    lapic_base[APIC_DFR] =  0xFFFFFFFF;             // Put the APIC into flat delivery mode

    lapic_base[APIC_LDR] &= 0x00FFFFFF;             // LDR mask
    uint8_t logicalApicID = 0;                      // Logical APIC ID
    lapic_base[APIC_LDR] |= logicalApicID<<24;      // LDR

    lapic_base[APIC_TASKPRIORITY] = 0;

    lapic_base[APIC_LINT0] = APIC_EXT_INT | BIT(15); // Enable normal external Interrupts // binär 1000 0111 0000 0000
    lapic_base[APIC_LINT1] = APIC_NMI;               // Enable NMI Processing
    lapic_base[APIC_ERROR] = APIC_INTERRUPTDISABLED; // Disable Error Interrupts
    lapic_base[APIC_THERMALSENSOR] = APIC_INTERRUPTDISABLED; // Disable thermal sensor interrupts
    lapic_base[APIC_PERFORMANCECOUNTER] = APIC_NMI;  // Enable NMI Processing

    // Local APIC timer
    lapic_base[APIC_TIMER_DIVIDECONFIG] = 0x03;     // Set up divide value to 16
    lapic_base[APIC_TIMER] = TMR_PERIODIC | (32+IRQ_TIMER); // Enable timer interrupts, 32, IRQ0
    lapic_base[APIC_TIMER_INITCOUNT] = 0xFFFFFFFF;  // Set initial counter -> starts timer

    lapic_base[APIC_SPURIOUSINTERRUPT] = APIC_SW_ENABLE | (32+7); // Enable APIC. Spurious: 39, IRQ7 (bit0-7)

    // repeat
    lapic_base[APIC_LINT0] = APIC_EXT_INT | BIT(15);// Enable normal external Interrupts // binär 1000 0111 0000 0000
    lapic_base[APIC_LINT1] = APIC_NMI;              // Enable NMI Processing

    // we use PIT (counter 2) for determining correct APIC timer init counter value
    uint16_t partOfSecond = 20; // 50 ms
    outportb(COUNTER_2_CONTROLPORT, (inportb(COUNTER_2_CONTROLPORT) & 0xFD)|1);
    outportb(COMMANDREGISTER, COUNTER_2 | RW_LO_HI_MODE | MODE0 | SIXTEEN_BIT_BINARY); // Mode 0 is important!
    uint32_t val = TIMECOUNTER_i8254_FREQU/partOfSecond;
    outportb(COUNTER_2_DATAPORT, BYTE1(val));    // LSB
    outportb(COUNTER_2_DATAPORT, BYTE2(val));    // MSB

    // start PIT counting and APIC timer
    val = inportb(COUNTER_2_CONTROLPORT) & 0xFE;
    outportb(COUNTER_2_CONTROLPORT, val);
    outportb(COUNTER_2_CONTROLPORT, val|1);
    lapic_base[APIC_TIMER_INITCOUNT] = 0xFFFFFFFF;

    // PIT timer at zero?
    while (!(inportb(COUNTER_2_CONTROLPORT) & BIT(5))) {/*if no, do nothing*/};
    lapic_base[APIC_TIMER] = APIC_INTERRUPTDISABLED; // if yes, stop APIC timer

    // calculate value for APIC timer
    val = (((0xFFFFFFFF - lapic_base[APIC_TIMER_CURRENTCOUNT]) +1) / SYSTEMFREQUENCY) * partOfSecond;

    // setting divide value register again and set timer
    lapic_base[APIC_TIMER_DIVIDECONFIG] = 0x03;
    lapic_base[APIC_TIMER] = TMR_PERIODIC | (32+IRQ_TIMER);

    // set APIC timer init value
    lapic_base[APIC_TIMER_INITCOUNT] = (val < 16 ? 16 : val);

    return true;
}

#ifdef _IOAPIC_ENABLE_
// read IO APIC register
static uint32_t ioapic_read(uint8_t index)
{
    *(ioapic_base + IOAPIC_IOREGSEL) = index;
    return *(volatile uint32_t *)(ioapic_base + IOAPIC_IOWIN);
}

// write IO APIC register
static void ioapic_write(uint8_t index, uint32_t val)
{
    *(ioapic_base + IOAPIC_IOREGSEL) = index;
    *(volatile uint32_t *)(ioapic_base + IOAPIC_IOWIN) = val;
}
#endif

bool ioapic_install(void)
{
    // Disable PIC to use APIC
#ifdef _IOAPIC_ENABLE_
    pic_disable();

    /////////////////////////////////////////////////
    // IO APIC
    ioapic_base = (uint8_t*)paging_allocMMIO(phys_ioapic_base, 1);

    uint8_t ioapicMaxIndexRedirTab = BYTE3(ioapic_read(IOAPIC_VERSION)); // bit16-23 // Maximum Redirection Entry — ReadOnly

  #ifdef _IRQ_DIAGNOSIS_
    uint8_t ioapicID = (ioapic_read(IOAPIC_ID) >> 24) & 0xF; // bit24-27 // IOAPIC Identification — R/W. This 4 bit field contains the IOAPIC identification.
    uint8_t ioapicVersion = ioapic_read(IOAPIC_VERSION) & 0xF; // bit0-7
    printf("\n     -> IO APIC ID: %u", ioapicID);
    printf("\n     -> IO APIC Version: %u", ioapicVersion);
    printf("\n     -> IO APIC Max. Index RedirTab: %u", ioapicMaxIndexRedirTab);
    printf("\n     -> APIC ID: %u", apic_base[APIC_APICID]);
  #endif

    for (uint8_t i=0; i<16; i++)
    {
        // Remap ISA IRQs (edge/hi)
        ioapic_remapVector(i, i+32, false /*edge*/, false /*high*/, i == 2 /*enabled, except #2*/);
    }

    ipc_setTaskWorkingNode("PrettyOS/PCI/ISABridge", true);
    if (ioapicMaxIndexRedirTab >= 19)
    {
        int64_t irqA_D[4] = { 0 };

        ipc_getInt(">/irqA", irqA_D + 0);
        ipc_getInt(">/irqB", irqA_D + 1);
        ipc_getInt(">/irqC", irqA_D + 2);
        ipc_getInt(">/irqD", irqA_D + 3);

        for (uint8_t i = 16; i < 20 && i < ioapicMaxIndexRedirTab; i++) // let's have a try until we know better from acpi or pci device
        {
            uint8_t mapped = irqA_D[i - 16];
            // 0x80 is default value in P2I bridge device; 0, 1, 2, 13 are not allowed.
            bool disabled = (mapped == 0x80 || mapped == 0 || mapped == 1 || mapped == 2 || mapped == 13);

            /// TODO: what is correct? level/low or edge/high
            /*
            // Remap PCI lines A#, B#, C#, D# (level/low)
            ioapic_remapVector(i, mapped + 32, true, true, disabled); // bochs: PIRQA# set to 0x0b, PIRQC# set to 0x0b
            */

            // Remap PCI lines A#, B#, C#, D# (edge/high)
            ioapic_remapVector(i, mapped + 32, false, false, disabled); // bochs: PIRQA# set to 0x0b, PIRQC# set to 0x0b
            // important for vmware!
            /// TODO
        }

      #ifdef _IRQ_DIAGNOSIS_
        printf("\nPCI #A: %u  #B: %u  #C: %u  #D: %u", (uint32_t)irqA_D[0], (uint32_t)irqA_D[1], (uint32_t)irqA_D[2], (uint32_t)irqA_D[3]);
      #endif
    }

    if (ioapicMaxIndexRedirTab >= 23)
    {
        int64_t irqE_H[4] = { 0 };
        ipc_getInt(">/irqE", irqE_H + 0);
        ipc_getInt(">/irqF", irqE_H + 1);
        ipc_getInt(">/irqG", irqE_H + 2);
        ipc_getInt(">/irqH", irqE_H + 3);

      #ifdef _IRQ_DIAGNOSIS_
        printf("\nPCI #E: %u  #F: %u  #G: %u  #H: %u", (uint32_t)irqE_H[0], (uint32_t)irqE_H[1], (uint32_t)irqE_H[2], (uint32_t)irqE_H[3]);
      #endif

        for (uint8_t i=20; i<24; i++)
        {
            // Remap motherboard interrupts 20/21, general purpose interrupt 22, INTIN23 or SMI# (dependant on mask bit)
            // Could also be E#, F#, G#, H# from PCI
            uint8_t mapped = irqE_H[i - 20];
            // 0x80 is default value in P2I bridge device; 0, 1, 2, 13 are not allowed.
            bool disabled = (mapped == 0x80 || mapped == 0 || mapped == 1 || mapped == 2 || mapped == 13);

            /// TODO: what is correct? level/low or edge/high
            // Remap PCI lines E#, F#, G#, H# (level/low)
            ioapic_remapVector(i, mapped + 32, true, true, disabled);
            /// TODO
        }
    }
#endif
    return true;
}

#ifdef _IOAPIC_ENABLE_
uint8_t ioapic_getID(void)
{
    return ((ioapic_read(IOAPIC_ID) >> 24) & 0xF);
}
#endif

bool lapic_eoi(uint32_t no)
{
    if (lapic_base)
    {
        lapic_base[APIC_EOI] = 0;
    }
    return lapic_base != 0;
}

#ifdef _IOAPIC_ENABLE_
static void ioapic_remapVector(uint8_t vector, uint32_t mapped, bool level /*f:edge t:level*/, bool low /*f:high t:low*/, bool disabled)
{
    // INTEL 82093AA I/O ADVANCED PROGRAMMABLE INTERRUPT CONTROLLER (IOAPIC)
    // 3.2.4. IOREDTBL[23:0]—I/O REDIRECTION TABLE REGISTERS

    // 63:56 Destination Field—R/W:
    // If the Destination Mode of this entry is Physical Mode (bit 11=0), bits [59:56] contain an APIC ID
    // If Logical Mode is selected (bit 11=1), the Destination Field potentially defines a set of processors.
    // Bits [63:56] of the Destination Field specify the logical destination address.
    // 55:17 Reserved.
    // 16 Interrupt Mask—R/W: 1: masked
    // 15 Trigger Mode—R/W: 1: Level sensitive, 0: Edge sensitive
    // 14 Remote IRR—RO.
    // 13 Interrupt Input Pin Polarity (INTPOL)—R/W: 0: High active, 1: Low active
    // 12 Delivery Status (DELIVS)—RO.
    // 11 Destination Mode (DESTMOD)—R/W: 0: Physical Mode, 1: Logical Mode
    // 10:8 Delivery Mode (DELMOD)—R/W: 000: Fixed, 010: SMI, 100: NMI, 101: INIT, 111: ExtINT
    // 7:0 Interrupt Vector (INTVEC)—R/W: Vector values range from 10h to FEh.

    if (disabled)
    {
        mapped |= APIC_INTERRUPTDISABLED;
    }

    if (level)
    {
        mapped |= APIC_LEVEL;
    }

    if (low)
    {
        mapped |= APIC_LOW;
    }

    //printf("\nvector: %u, mapped: %u", vector, (uint32_t)mapped);
    ioapic_write(0x10 + vector * 2, mapped);
    ioapic_write(0x11 + vector * 2, lapic_base[APIC_APICID] << (56 - 32));
}
#endif

/*
* Copyright (c) 2012-2017 The PrettyOS Project. All rights reserved.
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

