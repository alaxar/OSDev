/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "userlib.hpp"
#include "stdlib.h"

#if (__GNUC__ > 4 || (__GNUC__ == 4 && __GNUC_MINOR__ >= 7))
void* operator new(long unsigned int size)
{
    return malloc(size);
}
void* operator new[](long unsigned int size)
{
    return malloc(size);
}
void operator delete(void* ptr)
{
    free(ptr);
}
void operator delete[](void* ptr)
{
    free(ptr);
}
#elif (__GNUC__ == 4 && __GNUC_MINOR__ >= 5) || defined(__clang__)
void* operator new(unsigned int size)
{
    return malloc(size);
}
void* operator new[](unsigned int size)
{
    return malloc(size);
}
void operator delete(void* ptr)
{
    free(ptr);
}
void operator delete[](void* ptr)
{
    free(ptr);
}
#else
void* operator new(unsigned long size)
{
    return malloc(size);
}
void* operator new[](unsigned long size)
{
    return malloc(size);
}
void operator delete(void* ptr)
{
    free(ptr);
}
void operator delete[](void* ptr)
{
    free(ptr);
}
#endif


/*
* Copyright (c) 2009-2014 The PrettyOS Project. All rights reserved.
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
