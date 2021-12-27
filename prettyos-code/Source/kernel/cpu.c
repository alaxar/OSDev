/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "cpu.h"
#include "video/console.h"
#include "cmos.h"
#include "ipc.h"


// http://www.lowlevel.eu/wiki/Cpuid

static bool cpuid_available = false;
static bool supports_rdtsc = false;

int64_t* cpu_frequency;

void cpu_install(void)
{
    ipc_node_t* node;
    ipc_createNode("PrettyOS/CPU/Frequency (kHz)", &node, IPC_UINT);
    cpu_frequency = &node->data.integer;

    // Test if the CPU supports the CPUID-Command
    uint32_t result = 0;
    __asm__ volatile("pushfl\n"
                     "pop %%ecx\n"
                     "mov %%ecx, %%eax\n"
                     "xor %%eax, 0x200000\n"
                     "push %%eax\n"
                     "popfl\n"
                     "pushfl\n"
                     "pop %%eax\n"
                     "sub %%ecx, %%eax\n" : "=a"(result) : : "ecx");
    cpuid_available = (result == 0);

    if (cpu_supports(CF_PGE)) // We take this to indicate availability of CR4 register
        __asm__("mov %%cr4, %%eax;"
                "or $0x00000080, %%eax;" // Activate PGE
                "mov %%eax, %%cr4" : : : "eax");
    if (cpu_supports(CF_FXSR))
        __asm__("mov %%cr4, %%eax;"
                "or $0x00000200, %%eax;" // Activate OSFXSR
                "mov %%eax, %%cr4" : : : "eax");
    if (cpu_supports(CF_SSE))
        __asm__("mov %%cr4, %%eax;"
                "or $0x00000400, %%eax;" // Activate OSXMMEXCPT
                "mov %%eax, %%cr4" : : : "eax");
}

static void printSupport(bool b)
{
    textColor(TEXT);
    if (b)
        puts(" Yes");
    else
        puts(" No");
    textColor(LIGHT_GRAY);
}

void cpu_analyze(void)
{
    textColor(LIGHT_GRAY);
    if (!cpuid_available)
    {
        printf("   => CPU: ");
        textColor(TEXT);
        printf("Does not support cpuid instruction.\n");
        return;
    }

    printf("   => CPU:\n");
    printf("     => Sysenter:");
    printSupport(cpu_supports(CF_SYSENTEREXIT));
    printf("; PGE:");
    printSupport(cpu_supports(CF_PGE));
    printf("; FXSR:");
    printSupport(cpu_supports(CF_FXSR));
    supports_rdtsc = cpu_supports(CF_RDTSC);
    printf("; RDTSC:");
    printSupport(supports_rdtsc);
    printf("; SSE:");
    printSupport(cpu_supports(CF_SSE));
    printf("; MTRR:");
    printSupport(cpu_supports(CF_MTRR));

    // Read out VendorID
    char cpu_vendor[13];
    ((uint32_t*)cpu_vendor)[0] = cpu_idGetRegister(0, CR_EBX);
    ((uint32_t*)cpu_vendor)[1] = cpu_idGetRegister(0, CR_EDX);
    ((uint32_t*)cpu_vendor)[2] = cpu_idGetRegister(0, CR_ECX);
    cpu_vendor[12] = 0;

    ipc_setString("PrettyOS/CPU/VendorID", cpu_vendor);

    printf("\n     => VendorID: ");
    textColor(TEXT);
    printf("%s\n", cpu_vendor);
}

void cpu_calculateFrequency(void)
{
    if (!supports_rdtsc)
        return;

    static uint64_t LastRdtscValue = 0; // rdtsc: read time-stamp counter

    // calculate cpu frequency
    uint64_t Rdtsc = rdtsc();
    uint64_t RdtscKCounts   = (Rdtsc - LastRdtscValue);  // Build difference
    uint32_t RdtscKCountsHi = RdtscKCounts >> 32;        // high dword
    uint32_t RdtscKCountsLo = RdtscKCounts & 0xFFFFFFFF; // low dword
    LastRdtscValue = Rdtsc;

    if (RdtscKCountsHi == 0)
        *cpu_frequency = RdtscKCountsLo/1000;
}

bool cpu_supports(CPU_FEATURE feature)
{
    if (feature == CF_CPUID)
        return (cpuid_available);

    if (!cpuid_available)
        return (false);

    CPU_REGISTER r = feature&~31;

    return (cpu_idGetRegister(0x00000001, r) & (BIT(feature-r)));
}

uint32_t cpu_idGetRegister(uint32_t function, CPU_REGISTER reg)
{
    if (!cpuid_available) return (0);

    switch (reg)
    {
        case CR_EAX:
        {
            uint32_t temp;
            __asm__("cpuid" : "=a"(temp) : "a"(function) : "ebx", "ecx", "edx");
            return (temp);
        }
        case CR_EBX:
        {
            uint32_t temp, temp2;
            __asm__("cpuid" : "=b"(temp), "=a"(temp2) : "a"(function) : "ecx", "edx");
            return (temp);
        }
        case CR_ECX:
        {
            uint32_t temp, temp2;
            __asm__("cpuid" : "=c"(temp), "=a"(temp2) : "a"(function) : "ebx", "edx");
            return (temp);
        }
        case CR_EDX:
        {
            uint32_t temp, temp2;
            __asm__("cpuid" : "=d"(temp), "=a"(temp2) : "a"(function) : "ebx", "ecx");
            return (temp);
        }
        default:
            return (0);
    }
}

uint64_t cpu_MSRread(uint32_t msr)
{
    uint32_t low, high;

    __asm__("rdmsr" : "=a" (low), "=d" (high) : "c" (msr));

    return ((uint64_t)high << 32) | low;
}

void cpu_MSRwrite(uint32_t msr, uint64_t value)
{
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;

    __asm__("wrmsr" :: "a"(low), "c"(msr), "d"(high));
}


// FPU

task_t* volatile FPUTask = 0;

bool fpu_install(void)
{
    if (!(cmos_read(CMOS_DEVICES) & BIT(1)) || (cpu_supports(CF_CPUID) && !cpu_supports(CF_FPU)))
        return (false);

    __asm__ volatile ("finit");

    uint16_t ctrlword = 0x37F;
    __asm__ volatile("fldcw %0"::"m"(ctrlword)); // Set the FPU Control Word. FLDCW = Load FPU Control Word

    // Set the TS bit (no. 3) in CR0 to enable #NM (exception no. 7)
    // Set NS bit (no. 5) to signal FPU exceptions internally instead using PIC
    uint32_t cr0;
    __asm__("mov %%cr0, %0": "=r"(cr0)); // Read cr0
    cr0 |= BIT(3) | BIT(5);
    __asm__("mov %0, %%cr0":: "r"(cr0)); // Write cr0

    return (true);
}

void fpu_test(void)
{
    textColor(LIGHT_GRAY);
    printf("   => FPU test: ");

    if (!(cmos_read(CMOS_DEVICES) & BIT(1)) || (cpu_supports(CF_CPUID) && !cpu_supports(CF_FPU)))
    {
        printfe("FPU not available\n");
        return;
    }

    double squareroot = sqrt(2.0);
    squareroot = fabs(squareroot);
    squareroot /= sqrt(2.0);

    putch('[');

    if (fabs(squareroot - 1.0) < 0.000001)
    {
        textColor(SUCCESS);
        printf("PASSED");
    }
    else
    {
        printfe("FAILED");
    }

    textColor(LIGHT_GRAY);
    printf("]\n");
    textColor(TEXT);
}


/*
* Copyright (c) 2010-2016 The PrettyOS Project. All rights reserved.
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
