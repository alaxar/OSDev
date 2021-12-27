/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "ctype.h"


int isalnum(int c)
{
    return (isalpha(c) || isdigit(c));
}
int isalpha(int c)
{
    return (islower(c) || isupper(c));
}
int isblank(int c)
{
    return (c == ' ' || c == '\t');
}
int iscntrl(int c)
{
    return ((c >= 0 && c <= 0x1F) || c == 0x7F);
}
int isdigit(int c)
{
    return (c >= 0x30 && c <= 0x39);
}
int isgraph(int c)
{
    return (c >= 0x21 && c <= 0x7E);
}
int islower(int c)
{
    return (c >= 'a' && c <= 'z');
}
int isprint(int c)
{
    return (isgraph(c) || c == ' ');
}
int ispunct(int c)
{
    return (isgraph(c) && !isalnum(c));
}
int isspace(int c)
{
    return ((c >= 0x09 && c <= 0x0D) || c == ' ');
}
int isupper(int c)
{
    return (c >= 'A' && c <= 'Z');
}
int isxdigit(int c)
{
    return (isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f'));
}
int tolower(int c)
{
    return isupper(c) ? ('a' - 'A') + c : c;
}
int toupper(int c)
{
    return islower(c) ? ('A' - 'a') + c : c;
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
