/*
 *  license and disclaimer for the use of this source code as per statement below
 *  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
 */

#include "stdlib.h"
#include "ctype.h"
#include "limits.h"
#include "stdint.h"

// TODO: errno.h -> ERANGE, strtod() overflow -> HUGE_VAL?

double atof(const char* nptr)
{
    return strtod(nptr, 0);
}

int atoi(const char* nptr)
{
    return (int)strtol(nptr, 0, 10);
}

long int atol(const char* nptr)
{
    return strtol(nptr, 0, 10);
}

long long atoll(const char* nptr)
{
    return strtoll(nptr, 0, 10);
}

float strtof(const char* nptr, char** endptr)
{
    return strtod(nptr, endptr); // HACK?
}

double strtod(const char* nptr, char** endptr)
{
    double num = 0.0;
    int point = 0, exp = 0;

    while (isspace(*nptr))
        ++nptr; // skip spaces

    int sign = *nptr == '-' ? -1 : 1;
    if (*nptr == '+' || *nptr == '-')
        ++nptr;

    while (*nptr == '0')
        ++nptr;

    for (; *nptr != '\0'; ++nptr)
    {
        if (*nptr >= '0' && *nptr <= '9')
        {
            if (exp)
            {
                exp = exp * 10 + *nptr - '0';
            }
            else if (point)
            {
                num += (double)(*nptr - '0') / point;
                point *= 10;
            }
            else
            {
                num = num * 10 + *nptr - '0';
            }
        }
        else if (*nptr == '.')
        {
            if (!point)
            {
                point = 10;
            }
            else
            {
                break;
            }
        }
        else if (*nptr == 'E' || *nptr == 'e')
        {
            ++nptr;
            exp = *nptr == '-' ? -1 : 1;
            if (*nptr == '+' || *nptr == '-')
                ++nptr;
            if (*nptr >= '0' && *nptr <= '9')
                exp *= *nptr - '0';
        }
    }

    if (exp > 0)
    {
        while (exp--)
            num *= 10;
    }
    else if (exp < 0)
    {
        while (exp++)
            num /= 10;
    }

    if (endptr)
        *endptr = (char*)nptr;

    return num * sign;
}

long double strtold(const char* nptr, char** endptr); /// TODO

long int strtol(const char* nptr, char** endptr, int base)
{
    long num = 0;

    if (base && (base < 2 || base > 36))
        return num;

    while (isspace(*nptr))
        ++nptr; // skip spaces

    int sign = *nptr == '-' ? -1 : 1;
    if (*nptr == '-' || *nptr == '+')
        ++nptr;

    if (base == 0)
    {
        base = 10;
        if (*nptr == '0' && (*(++nptr) == 'X' || *nptr == 'x'))
        {
            ++nptr;
            base = 16;
        }
    }
    else if (base == 16)
    {
        if (*nptr == '0' && (*(++nptr) == 'X' || *nptr == 'x'))
            ++nptr;
    }

    while (*nptr == '0')
        ++nptr;

    for (; *nptr != '\0'; ++nptr)
    {
        if (*nptr - '0' < base && *nptr - '0' >= 0)
        {
            num = num * base + *nptr - '0';
            if (num <= 0)
            {
                num = LONG_MAX;
                break;
            }
        }
        else if ((*nptr | 0x20) - 'a' + 10 < base &&
            (*nptr | 0x20) - 'a' + 10 >= 0)
        { // 'x' | 0x20 =~= tolower('X')
            num = num * base + (*nptr | 0x20) - 'a' + 10;
            if (num <= 0)
            {
                num = LONG_MAX;
                break;
            }
        }
        else
        {
            break;
        }
    }

    for (; *nptr != '\0'; ++nptr)
    {
        // skip rest of integer constant
        if (!(*nptr - '0' < base && *nptr - '0' >= 0) &&
                !((*nptr | 0x20) - 'a' + 10 < base) &&
                !((*nptr | 0x20) - 'a' + 10 >= 0))
        {
            break;
        }
    }

    if (endptr)
        *endptr = (char*)nptr;

    return num * sign;
}

long long strtoll(const char* nptr, char** endptr, int base); /// TODO

unsigned long int strtoul(const char* nptr, char** endptr, int base)
{
    unsigned long num = 0;

    while (isspace(*nptr))
        ++nptr; // skip spaces

    if (base && (base < 2 || base > 36))
        return num;

    if (base == 0)
    {
        base = 10;
        if (*nptr == '0' && (*(++nptr) == 'X' || *nptr == 'x'))
        {
            ++nptr;
            base = 16;
        }
    }
    else if (base == 16)
    {
        if (*nptr == '0' && (*(++nptr) == 'X' || *nptr == 'x'))
            ++nptr;
    }

    while (*nptr == '0')
        ++nptr;

    for (; *nptr != '\0'; ++nptr)
    {
        unsigned long buf = num;
        if (*nptr - '0' < base && *nptr - '0' >= 0)
        {
            num = num * base + *nptr - '0';
        }
        else if ((*nptr | 0x20) - 'a' + 10 < base &&
                 (*nptr | 0x20) - 'a' + 10 >= 0)
        { // 'x' | 0x20 =~= tolower('X')
            num = num * base + (*nptr | 0x20) - 'a' + 10;
        }
        else
        {
            break;
        }
        if (num <= buf)
        {
            num = ULONG_MAX;
            break;
        }
    }

    for (; *nptr != '\0'; ++nptr)
    {
        // skip rest of integer constant
        if (!(*nptr - '0' < base && *nptr - '0' >= 0) &&
                !((*nptr | 0x20) - 'a' + 10 < base) &&
                !((*nptr | 0x20) - 'a' + 10 >= 0))
        {
            break;
        }
    }

    if (endptr)
        *endptr = (char*)nptr;

    return num;
}

unsigned long long strtoull(const char* nptr, char** endptr, int base); /// TODO

/*
* Copyright (c) 2011-2013 The PrettyOS Project. All rights reserved.
*
* http://www.c-plusplus.de/forum/viewforum-var-f-is-62.html
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
