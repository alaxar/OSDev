#ifndef CPU_H
#define CPU_H

#include "util/util.h"
#include "tasking/task.h"


typedef enum
{ // Uses bits larger than 5 because the bitnumber inside the registers can be 31 at maximum which needs the first 5 bits
    CR_EAX = BIT(5),
    CR_EBX = BIT(6),
    CR_ECX = BIT(7),
    CR_EDX = BIT(8)
} CPU_REGISTER;

typedef enum
{ // In this enum the last five bits contain the bit in the register, the other bits are the register (see above)
    CF_CPUID        = 0,

    CF_FPU          = CR_EDX|0,
    CF_VME86        = CR_EDX|1,
    CF_IOBREAKPOINT = CR_EDX|2,
    CF_PAGES4MB     = CR_EDX|3,
    CF_RDTSC        = CR_EDX|4,
    CF_MSR          = CR_EDX|5,
    CF_PAE          = CR_EDX|6,
    CF_MCE          = CR_EDX|7,
    CF_X8           = CR_EDX|8,
    CF_APIC         = CR_EDX|9,
    CF_SYSENTEREXIT = CR_EDX|11,
    CF_MTRR         = CR_EDX|12,
    CF_PGE          = CR_EDX|13,
    CF_MCA          = CR_EDX|14,
    CF_CMOV         = CR_EDX|15,
    CF_PAT          = CR_EDX|16,
    CF_PSE36        = CR_EDX|17,
    CF_CLFLUSH      = CR_EDX|19,
    CF_MMX          = CR_EDX|23,
    CF_FXSR         = CR_EDX|24,
    CF_SSE          = CR_EDX|25,
    CF_SSE2         = CR_EDX|26,
    CF_HTT          = CR_EDX|28,

    CF_SSE3         = CR_ECX|0,
    CF_PCLMULQDQ    = CR_ECX|1,
    CF_MONITOR      = CR_ECX|3,
    CF_SSSE3        = CR_ECX|9,
    CF_FMA          = CR_ECX|12,
    CF_CMPXCHG16B   = CR_ECX|13,
    CF_SSE41        = CR_ECX|19,
    CF_SSE42        = CR_ECX|20,
    CF_MOVBE        = CR_ECX|22,
    CF_POPCNT       = CR_ECX|23,
    CF_AESNI        = CR_ECX|25,
    CF_XSAVE        = CR_ECX|26,
    CF_OSXSAVE      = CR_ECX|27,
    CF_AVX          = CR_ECX|28,
    CF_F16C         = CR_ECX|29,
    CF_RDRAND       = CR_ECX|30,
} CPU_FEATURE;


extern int64_t* cpu_frequency;
extern task_t* volatile FPUTask;


void     cpu_install(void);
void     cpu_analyze(void);
void     cpu_calculateFrequency(void);
bool     cpu_supports(CPU_FEATURE feature);
uint32_t cpu_idGetRegister(uint32_t function, CPU_REGISTER reg);
uint64_t cpu_MSRread(uint32_t msr);
void     cpu_MSRwrite(uint32_t msr, uint64_t value);

bool fpu_install(void);
void fpu_test(void);


#endif
