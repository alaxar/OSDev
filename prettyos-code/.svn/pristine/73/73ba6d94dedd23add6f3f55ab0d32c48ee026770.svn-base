/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "string.h"
#include "stdint.h"
#include "errno.h"


void* memchr(void* ptr, char value, size_t num)
{
    char* str = ptr;
    for (; num > 0; str++, num--)
    {
        if (*str == value)
        {
            return str;
        }
    }
    return (0);
}

int memcmp(const void* ptr1, const void* ptr2, size_t num)
{
    if (num == 0) return (0);

    const uint8_t* s1 = ptr1;
    const uint8_t* s2 = ptr2;
    for (; num > 1 && *s1 == *s2; num--)
    {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

void* memcpy(void* dest, const void* src, size_t bytes)
{
    void* retval = dest;
    size_t dwords = bytes/4;
    bytes %= 4;
    __asm__ volatile("cld\n"
                     "rep movsl\n"
                     "mov %%edx, %%ecx\n"
                     "rep movsb" : "+S"(src), "+D"(dest), "+c"(dwords) : "d"(bytes) : "memory");
    return (retval);
}

static void memcpyr(void* dest, const void* src, size_t bytes)
{
    // Calculate starting addresses
    void* temp = dest+bytes-1;
    src += bytes-1;

    size_t dwords = bytes/4;
    bytes %= 4;

    __asm__ volatile("std\n"
                     "rep movsb\n"
                     "sub $3, %%edi\n"
                     "sub $3, %%esi\n"
                     "mov %%edx, %%ecx\n"
                     "rep movsl\n"
                     "cld" : "+S"(src), "+D"(temp), "+c"(bytes) : "d"(dwords) : "memory");
}

void* memmove(void* dest, const void* src, size_t bytes)
{
    if (src == dest || bytes == 0) // Copying is not necessary. Calling memmove with source==destination or size==0 is not a bug.
    {
        return (dest);
    }

    // Check for out of memory
    const uintptr_t memMax = ~((uintptr_t)0) - (bytes - 1); // ~0 is the highest possible value of the variables type. (No underflow possible on substraction, because size < adress space)
    if ((uintptr_t)src > memMax || (uintptr_t)dest > memMax)
    {
        return (dest);
    }

    // Arrangement of the destination and source decides about the direction of copying
    if (src < dest)
    {
        memcpyr(dest, src, bytes);
    }
    else // In all other cases, it is ok to copy from the start to the end of source.
    {
        memcpy(dest, src, bytes); // We assume, that memcpy does forward copy
    }
    return (dest);
}

void* memset(void* dest, char val, size_t bytes)
{
    void* retval = dest;
    uint8_t uval = val;
    size_t dwords = bytes/4; // Number of dwords (4 Byte blocks) to be written
    bytes %= 4;              // Remaining bytes
    uint32_t dval = (uval<<24)|(uval<<16)|(uval<<8)|uval; // Create dword from byte value
    __asm__ volatile("cld\n"
                     "rep stosl\n"
                     "mov %%edx, %%ecx\n"
                     "rep stosb" : "+D"(dest), "+c"(dwords) : "a"(dval), "d"(bytes) : "memory");
    return retval;
}



size_t strlen(const char* str)
{
    size_t retval = 0;
    for (; *str != '\0'; ++str)
        ++retval;
    return retval;
}

int strcmp(const char* s1, const char* s2)
{
    while ((*s1) && (*s1 == *s2))
    {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

int strncmp(const char* s1, const char* s2, size_t n)
{
    if (n == 0) return (0);

    for (; *s1 && n > 1 && *s1 == *s2; n--)
    {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

char* strcpy(char* dest, const char* src)
{
    char* save = dest;
    while ((*dest++ = *src++));
    return save;
}

char* strncpy(char* dest, const char* src, size_t n)
{
    size_t i = 0;
    for (; i < n && src[i] != 0; i++)
    {
        dest[i] = src[i];
    }
    memset(dest+i, 0, n-i);
    return (dest);
}

char* strcat(char* dest, const char* src)
{
    strcpy(dest + strlen(dest), src);
    return dest;
}

char* strncat(char* dest, const char* src, size_t n)
{
    char* temp = dest + strlen(dest);
    size_t i = 0;
    for (; i < n && src[i] != 0; i++)
    {
        temp[i] = src[i];
    }
    if (i < n)
        temp[i] = 0;
    return dest;
}

char* strchr(const char* str, char character)
{
    for (;; str++)
    {
        // the order here is important
        if (*str == character)
        {
            return (char*)str;
        }
        if (*str == 0) // end of string
        {
            return (0);
        }
    }
}

char* strrchr(const char* s, int c)
{
    const char* p = s + strlen(s);
    if (c == 0)
        return (char*)p;
    while (p != s)
    {
        if (*(--p) == c)
            return (char*)p;
    }
    return (0);
}

int strcoll(const char* str1, const char* str2); /// TODO

size_t strcspn(const char* str, const char* key)
{
    const char* ostr = str;
    for (; *str != 0; str++)
        if (strchr(key, *str) != 0)
            return (str-ostr);
    return (str-ostr);
}

char* strerror(int errornum)
{
    switch (errornum)
    {
        case EDOM:
            return "Domain error";
        case ERANGE:
            return "Range error";
        default:
            return "";
    }
}

char* strpbrk(const char* str, const char* delim)
{
    for (; *str != 0; str++)
        for (size_t i = 0; delim[i] != 0; i++)
            if (*str == delim[i])
                return ((char*)str);

    return (0);
}

size_t strspn(const char* str, const char* key)
{
    const char* ostr = str;
    for (; *str != 0; str++)
        if (strchr(key, *str) == 0)
            return (str-ostr);
    return (str-ostr);
}

char* strstr(const char* str1, const char* str2)
{
    const char* p1 = str1;
    while (*str1)
    {
        const char* p2 = str2;
        while (*p2 && (*p1 == *p2))
        {
            ++p1;
            ++p2;
        }
        if (*p2 == 0)
        {
            return (char*)str1;
        }
        ++str1;
        p1 = str1;
    }
    return (0);
}

char* strtok(char* str, const char* delimiters)
{
    static char* last = 0;
    if (!str)
        str = last;
    if (!str || *str == 0)
        return 0;
    while (*str != 0 && strchr(delimiters, *str) != 0)
        str++;
    char* end = str + strcspn(str, delimiters);
    last = end+1;
    *end = 0;
    return (str);
}

size_t strxfrm(char* destination, const char* source, size_t num); /// TODO


/*
* Copyright (c) 2010-2017 The PrettyOS Project. All rights reserved.
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
