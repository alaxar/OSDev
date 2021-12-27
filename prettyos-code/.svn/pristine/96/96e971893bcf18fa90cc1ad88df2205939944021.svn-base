/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "syscall.h"
#include "cpu.h"
#include "util/util.h"
#include "tasking/task.h"
#include "filesystem/fat12.h"
#include "audio/sys_speaker.h"
#include "timer.h"
#include "irq.h"
#include "storage/devicemanager.h"
#include "executable.h"
#include "events.h"
#include "hid/keyboard.h"
#include "hid/mouse.h"
#include "netprotocol/tcp.h"
#include "netprotocol/udp.h"
#include "video/video.h"
#include "network/network.h"
#include "ipc.h"
#include "userheap.h"


// Overwiew to all syscalls in documentation/Syscalls.odt

static void* syscalls[] =
{
/*  0  */    &executeFile,
/*  1  */    &nop, // createThread
/*  2  */    &exit,
/*  3  */    &scheduler_blockCurrentTask,
/*  4  */    &nop, // createConsoleThread
/*  5  */    &nop,
/*  6  */    &getpid,
/*  7  */    &nop,
/*  8  */    &nop,
/*  9  */    &nop,

/*  10 */    &userHeapSc_create,
/*  11 */    &userHeapSc_destroy,
/*  12 */    &userHeapSc_alloc,
/*  13 */    &userHeapSc_free,
/*  14 */    &nop,

/*  15 */    &fopen,
/*  16 */    &fgetc,
/*  17 */    &fputc,
/*  18 */    &fseek,
/*  19 */    &fflush,
/*  20 */    &fmove,
/*  21 */    &fclose,
/*  22 */    &formatPartition,
/*  23 */    &folderAccess,
/*  24 */    &folderClose,

/*  25 */    &ipc_fopen,
/*  26 */    &ipc_getFolder,
/*  27 */    &ipc_getString,
/*  28 */    &ipc_setString,
/*  29 */    &ipc_getInt,
/*  30 */    &ipc_setInt,
/*  31 */    &ipc_getDouble,
/*  32 */    &ipc_setDouble,
/*  33 */    &ipc_deleteKey,
/*  34 */    &ipc_setAccess,
/*  35 */    &ipc_setTaskWorkingNode,

/*  36 */    &nop,
/*  37 */    &waitForEvent,
/*  38 */    &event_enable,
/*  39 */    &event_poll,

/*  40 */    &timer_getMilliseconds,
/*  41 */    &nop,
/*  42 */    &nop,
/*  43 */    &nop,
/*  44 */    &nop,
/*  45 */    &nop,
/*  46 */    &nop,
/*  47 */    &nop,
/*  48 */    &nop,
/*  49 */    &nop,

/*  50 */    &systemControl,
/*  51 */    &nop, // systemRefresh
/*  52 */    &nop,
/*  53 */    &nop,
/*  54 */    &nop,

/*  55 */    &putch,
/*  56 */    &textColor,
/*  57 */    &setScrollField,
/*  58 */    &setCursor,
/*  59 */    &getCursor,
/*  60 */    &nop, // readChar
/*  61 */    &console_clear,
/*  62 */    &console_setProperties,
/*  63 */    &refreshUserScreen,
/*  64 */    &nop,
/*  65 */    &nop,
/*  66 */    &nop,
/*  67 */    &nop,
/*  68 */    &nop,
/*  69 */    &nop,

/*  70 */    &nop,
/*  71 */    &keyPressed,
/*  72 */    &mouse_buttonPressed,
/*  73 */    &mouse_getPosition,
/*  74 */    &mouse_setPosition,
/*  75 */    &nop,
/*  76 */    &nop,
/*  77 */    &nop,
/*  78 */    &nop,
/*  79 */    &nop,

/*  80 */    &beep,
/*  81 */    &nop,
/*  82 */    &nop,

/*  83 */    &dns_getServer,
/*  84 */    &dns_setServer,
/*  85 */    &tcp_uconnect,
/*  86 */    &tcp_usend,
/*  87 */    &tcp_uclose,
/*  88 */    &udp_usend,
/*  89 */    &udp_bind,
/*  90 */    &udp_unbind,
/*  91 */    &getMyIP,
/*  92 */    &nop,
/*  93 */    &nop,
/*  94 */    &nop,
/*  95 */    &nop,
/*  96 */    &nop,
/*  97 */    &nop,
/*  98 */    &nop,
/*  99 */    &nop,


// COMPATIBILITY (100-101); should be removed
    &flpydsk_read_directory,
    &cprintf
};

static void syscall_handler(registers_t* r);
void syscall_sysenterHandler(void);

void syscall_install(void)
{
    irq_installHandler(IRQ_SYSCALL, syscall_handler);
    if (cpu_supports(CF_SYSENTEREXIT))
    {
        cpu_MSRwrite(0x174, 8); // CS
        cpu_MSRwrite(0x175, 0x480000); // ESP
        cpu_MSRwrite(0x176, (uintptr_t)&syscall_sysenterHandler); // EIP
    }
}

static void syscall_handler(registers_t* r)
{
    // Firstly, check if the requested syscall number is valid. The syscall number is found in EAX.
    if (r->eax >= sizeof(syscalls)/sizeof(*syscalls))
        return;

    console_current = currentTask->console; // Syscall output should appear in the console of the task that caused the syscall

    // We don't know how many parameters the function wants.
    // Therefore, we push them all onto the stack in the correct order.
    // The function will use the number of parameters it wants.
    __asm__ volatile (
     "push %1; \
      push %2; \
      push %3; \
      push %4; \
      push %5; \
      call *%6; \
      add $20, %%esp;"
       : "=a" (r->eax) : "D" (r->edi), "S" (r->esi), "d" (r->edx), "c" (r->ecx), "b" (r->ebx), "a" (syscalls[r->eax]) : "esp", "memory");

    console_current = kernelTask.console;
}

__asm__("syscall_sysenterHandler:"
        "    shl $2, %eax;"        // Calculate address of function: Multiply syscall number by 4
        "    add $syscalls, %eax;"
        "    push %ebp;"           // Return address is in ebp - save it on stack.
        "    push %edi;"           // Push parameters on stack
        "    push %esi;"
        "    push %edx;"
        "    push %ecx;"
        "    push %ebx;"
        "    call *(%eax);"        // Call syscall
        "    add $20, %esp;"       // Restore stack
        "    pop %edx;"            // Move return address to edx
        "    sti;"                 // IF flag is disabled by sysenter
        "    sysexit;");           // Leave kernel

/*
* Copyright (c) 2009-2016 The PrettyOS Project. All rights reserved.
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
