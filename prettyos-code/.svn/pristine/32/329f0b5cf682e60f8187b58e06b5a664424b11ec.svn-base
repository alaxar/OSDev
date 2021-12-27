/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "setjmp.h"
#include "stdio.h"


// TODO: Fix it.

int setjmp(jmp_buf env)
{
    register int retVal;
    __asm__ volatile("mov %%esp, %0\n"
                     "mov %%ebp, %1\n"
                     "mov -4(%%ebp), %2\n"
                     "mov $label1, %3\n"
                     "mov -8(%%ebp), %4\n"
                     "mov $0, %5\n"
                     "jmp label2\n"
                     "label1:\n"
                     "mov %%eax, %5\n"
                     "label2:\n" : "=r"(env[0].esp), "=r"(env[0].ebp), "=r"(env[0].oldebp), "=r"(env[0].jmpAdress), "=r"(env[0].returnAdress), "=r"(retVal) : : "esp", "ebp");
    return (retVal);
}

void longjmp(jmp_buf env, int retVal)
{
    __asm__ volatile("mov %0, %%esp\n"
                     "mov %1, %%ebp\n"
                     "mov %2, -4(%%ebp)\n"
                     "mov %4, -8(%%ebp)\n"
                     "mov %5, %%eax\n"
                     "jmp *%3" : : "r"(env[0].esp), "r"(env[0].ebp), "r"(env[0].oldebp), "r"(env[0].jmpAdress), "r"(env[0].returnAdress), "r"(retVal) : "esp", "ebp");
}


/*
* Copyright (c) 2010-2015 The PrettyOS Project. All rights reserved.
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
