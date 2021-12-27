/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "stdlib.h"
#include "stdint.h"
#include "string.h"
#include "signal.h"



void(**_atexit_funcs)(void) = 0; // Stores a zero terminated array of function pointers, called at exit
void(**_at_quick_exit_funcs)(void) = 0; // Stores a zero terminated array of function pointers, called at quick_exit


void exitProcess(void); // -> Syscall, Userlib
void* userHeap_create(size_t initSize); // -> Syscall, Userlib
void userHeap_destroy(void* handle); // -> Syscall, Userlib
void* userHeap_alloc(void* handle, size_t size); // -> Syscall, Userlib
void userHeap_free(void* handle, void* address); // -> Syscall, Userlib

void abort(void)
{
    raise(SIGABRT);
}

void exit(int status)
{
    if (_atexit_funcs)
        for (size_t i = 0; _atexit_funcs[i]; i++)
            _atexit_funcs[i]();
    exitProcess();
}

void quick_exit(int status)
{
    if (_at_quick_exit_funcs)
        for (size_t i = 0; _at_quick_exit_funcs[i]; i++)
            _at_quick_exit_funcs[i]();
    exitProcess();
}

void _Exit(int status)
{
    exitProcess();
}

int atexit(void (*func)(void))
{
    static size_t num = 0;
    static size_t size = 0;
    if (num+1 >= size)
    {
        size += 5;
        void* temp = malloc(size*sizeof(*_atexit_funcs)); // TODO: realloc
        memcpy(temp, _atexit_funcs, (size - 5)*sizeof(*_atexit_funcs));
        free(_atexit_funcs);
        _atexit_funcs = temp;
    }
    _atexit_funcs[num] = func;
    num++;
    _atexit_funcs[num] = 0;
    return (0);
}

int at_quick_exit(void (*func)(void))
{
    static size_t num = 0;
    static size_t size = 0;
    if (num+1 >= size)
    {
        size += 5;
        void* temp = malloc(size*sizeof(*_at_quick_exit_funcs)); // TODO: realloc
        memcpy(temp, _at_quick_exit_funcs, (size - 5)*sizeof(*_at_quick_exit_funcs));
        free(_at_quick_exit_funcs);
        _at_quick_exit_funcs = temp;
    }
    _at_quick_exit_funcs[num] = func;
    num++;
    _at_quick_exit_funcs[num] = 0;
    return (0);
}

int abs(int n); // -> math.c
long labs(long n)
{
    return (n<0?-n:n);
}
long long llabs(long long n)
{
    return (n<0?-n:n);
}

div_t div(int numerator, int denominator)
{
    div_t d = {.quot = numerator/denominator, .rem = numerator%denominator};
    return (d);
}

ldiv_t ldiv(long numerator, long denominator)
{
    ldiv_t d = {.quot = numerator/denominator, .rem = numerator%denominator};
    return (d);
}

lldiv_t lldiv(long long numerator, long long denominator)
{
    lldiv_t d = {.quot = numerator/denominator, .rem = numerator%denominator};
    return (d);
}

int mblen(const char* pmb, size_t max); /// TODO
size_t mbstowcs(wchar_t* wcstr, const char* mbstr, size_t max); /// TODO
int mbtowc(wchar_t* pwc, const char* pmb, size_t max); /// TODO
size_t wcstombs(char* mbstr, const wchar_t* wcstr, size_t max); /// TODO
int wctomb(char* pmb, wchar_t character); /// TODO

static int seed = 0;
void srand(int val)
{
    seed = val;
}
int rand(void)
{
    return (((seed = seed * 214013L + 2531011L) >> 16) & RAND_MAX);
}
double random(double lower, double upper)
{
    return (((double)rand() / ((double)RAND_MAX / (upper - lower))) + lower);
}

static void* defaultProcessHeap = 0;
void* malloc(size_t size)
{
    if (!defaultProcessHeap)
        defaultProcessHeap = userHeap_create(size); // Initialize only once
    return userHeap_alloc(defaultProcessHeap, size);
}

void* calloc(size_t num, size_t size)
{
    void* ptr = malloc(num*size);
    memset(ptr, 0, num*size);
    return (ptr);
}

void* realloc(void* ptr, size_t size); /// TODO (Add syscall that returns the size of allocated block)

void free(void* ptr)
{
    userHeap_free(defaultProcessHeap, ptr);
}

char* getenv(const char* name)
{
    return (0); // We do not support any environment variables at the moment
}

int system(const char* command); /// TODO


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
