/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "stdio.h"
#include "stdint.h"
#include "stdlib.h"
#include "string.h"
#include "stdbool.h"


void  i2hex(uint32_t val, char* dest, uint32_t len); // -> Userlib
char* itoa(int32_t n, char* s); // -> Userlib
char* utoa(unsigned int n, char* s); // -> Userlib
void  ftoa(float f, char* buffer); // -> Userlib
int fmove(const char* oldpath, const char* newpath, bool copy); // -> Userlib


FILE* stderr;
FILE* stdin;
FILE* stdout;


FILE* fopen(const char* path, const char* mode); // -> Syscall
FILE* tmpfile(void); /// TODO
FILE* freopen(const char* filename, const char* mode, FILE* file); /// TODO
int fclose(FILE* file); // -> Syscall
int remove(const char* path)
{
    return fmove(path, 0, false);
}
int rename(const char* oldpath, const char* newpath)
{
    return fmove(oldpath, newpath, false);
}
int fputc(char c, FILE* file); // -> Syscall
int putc(char c, FILE* file)
{
    return (fputc(c, file));
}
char fgetc(FILE* file); // -> Syscall
char getc(FILE* file)
{
    return (fgetc(file));
}
int ungetc(char c, FILE* file); /// TODO
char* fgets(char* dest, size_t num, FILE* file)
{
    for (size_t i = 0; i < num-1; i++)
    {
        dest[i] = fgetc(file);
        if (dest[i] == EOF || dest[i] == '\n')
        {
            dest[i] = 0;
            return (dest);
        }
    }
    dest[num - 1] = 0;
    return (dest);
}
int fputs(const char* src, FILE* file)
{
    for (; *src; src++)
        fputc(*src, file);
    fputc('\n', file);
    return (0);
}
size_t fread(void* dest, size_t size, size_t count, FILE* file)
{
    for (size_t i = 0; i < count*size; i++)
        ((char*)dest)[i] = fgetc(file);
    return (count*size);
}
size_t fwrite(const void* src, size_t size, size_t count, FILE* file)
{
    for (size_t i = 0; i < count*size; i++)
        fputc(((const char*)src)[i], file);
    return (count*size);
}
int fflush(FILE* file); // -> Syscall
size_t ftell(FILE* file); /// TODO
int fseek(FILE* file, int offset, SEEK_ORIGIN origin); // -> Syscall
int rewind(FILE* file); /// TODO
int feof(FILE* file); /// TODO
int ferror(FILE* file); /// TODO
void clearerr(FILE* file); /// TODO
int fgetpos(FILE* file, fpos_t* position); /// TODO
int fsetpos(FILE* file, const fpos_t* position); /// TODO
int vfprintf(FILE* file, const char* format, va_list arg); /// TODO
int fprintf(FILE* file, const char* format, ...); /// TODO
int vfscanf(FILE* file, const char* format, va_list arg); /// TODO
int fscanf(FILE* file, const char* format, ...); /// TODO
void setbuf(FILE* file, char* buffer); /// TODO
int setvbuf(FILE* file, char* buffer, int mode, size_t size); /// TODO
char* tmpnam(char* str); /// TODO



void perror(const char* string); /// TODO



char getchar(void); // -> Syscall
char* gets_s(char* str, size_t length)
{
    int32_t i = 0;
    char c;
    do
    {
        c = getchar();
        if (c=='\b')  // Backspace
        {
            if (i>0)
            {
                putchar(c);
                if (i < length-2)
                    str[i-1]='\0';
                --i;
            }
        }
        else
        {
            if (c != '\n')
            {
                if (i < length-1)
                    str[i] = c;
                i++;
            }
            putchar(c);
        }
    }
    while (c != '\n'); // Linefeed

    if (i >= length)
        i = length - 1;
    str[i] = '\0';

    return str;
}

int vscanf(const char* format, va_list arg); /// TODO
int scanf(const char* format, ...); /// TODO
int putchar(char c); // -> Syscall
int puts(const char* str)
{
    size_t i = 0;
    for (; str[i] != 0; i++)
    {
        putchar(str[i]);
    }
    return (int)i; // Return number of characters written. The standard only requires a non-negative return value in case of success.
}

int vprintf(const char* format, va_list arg)
{
    char buffer[32]; // Larger is not needed at the moment

    int pos = 0;
    for (; *format; format++)
    {
        switch (*format)
        {
        case '%':
            switch (*(++format))
            {
            case 'u':
                utoa(va_arg(arg, uint32_t), buffer);
                pos += puts(buffer);
                break;
            case 'f':
                ftoa(va_arg(arg, double), buffer);
                pos += puts(buffer);
                break;
            case 'i': case 'd':
                itoa(va_arg(arg, int32_t), buffer);
                pos += puts(buffer);
                break;
            case 'X':
                i2hex(va_arg(arg, uint32_t), buffer, 8);
                puts(buffer);
                pos += 8;
                break;
            case 'x':
                i2hex(va_arg(arg, uint32_t), buffer, 4);
                puts(buffer);
                pos += 4;
                break;
            case 'y':
                i2hex(va_arg(arg, uint32_t), buffer, 2);
                puts(buffer);
                pos += 2;
                break;
            case 's':
            {
                char* temp = va_arg(arg, char*);
                pos += puts(temp);
                break;
            }
            case 'c':
                putchar((int8_t)va_arg(arg, int32_t));
                pos++;
                break;
            case '%':
                putchar('%');
                pos++;
                break;
            default:
                --format;
                --pos;
                break;
            }
            break;
        default:
            putchar(*format);
            pos++;
            break;
        }
    }
    return (pos);
}

int printf(const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    int retval = vprintf(format, arg);
    va_end(arg);
    return (retval);
}




int vsprintf(char* dest, const char* format, va_list arg)
{
    int pos = 0;
    char m_buffer[32]; // Larger is not needed at the moment
    size_t length;

    for (; *format; format++)
    {
        switch (*format)
        {
            case '%':
                switch (*(++format))
                {
                    case 'u':
                        utoa(va_arg(arg, uint32_t), m_buffer);
                        length = strlen(m_buffer);
                        memcpy(dest+pos, m_buffer, length);
                        pos += length;
                        break;
                    case 'f':
                        ftoa(va_arg(arg, double), m_buffer);
                        length = strlen(m_buffer);
                        memcpy(dest+pos, m_buffer, length);
                        pos += length;
                        break;
                    case 'i': case 'd':
                        itoa(va_arg(arg, int32_t), m_buffer);
                        length = strlen(m_buffer);
                        memcpy(dest+pos, m_buffer, length);
                        pos += length;
                        break;
                    case 'X':
                        i2hex(va_arg(arg, uint32_t), m_buffer, 8);
                        length = strlen(m_buffer);
                        memcpy(dest+pos, m_buffer, length);
                        pos += length;
                        break;
                    case 'x':
                        i2hex(va_arg(arg, uint32_t), m_buffer, 4);
                        length = strlen(m_buffer);
                        memcpy(dest+pos, m_buffer, length);
                        pos += length;
                        break;
                    case 'y':
                        i2hex(va_arg(arg, uint32_t), m_buffer, 2);
                        length = strlen(m_buffer);
                        memcpy(dest+pos, m_buffer, length);
                        pos += length;
                        break;
                    case 's':
                    {
                        char* buf = va_arg(arg, char*);
                        length = strlen(buf);
                        memcpy(dest+pos, buf, length);
                        pos += length;
                        break;
                    }
                    case 'c':
                        dest[pos] = (int8_t)va_arg(arg, int32_t);
                        pos++;
                        break;
                    case '%':
                        dest[pos] = '%';
                        pos++;
                        break;
                    default:
                        --format;
                        break;
                }
                break;
            default:
                dest[pos] = (*format);
                pos++;
                break;
        }
        dest[pos] = '\0';
    }
    return (pos);
}

int sprintf(char* dest, const char* format, ...)
{
    va_list arg;
    va_start(arg, format);
    int retval = vsprintf(dest, format, arg);
    va_end(arg);
    return (retval);
}

void vsnprintf(char* buffer, size_t length, const char* args, va_list ap)
{
    char m_buffer[32]; // Larger is not needed at the moment

    size_t pos;
    for (pos = 0; *args && pos < length; args++)
    {
        switch (*args)
        {
            case '%':
                switch (*(++args))
                {
                    case 'u':
                        utoa(va_arg(ap, uint32_t), m_buffer);
                        strncpy(buffer+pos, m_buffer, length - pos);
                        pos += strlen(m_buffer);
                        break;
                    case 'f':
                        ftoa(va_arg(ap, double), m_buffer);
                        strncpy(buffer+pos, m_buffer, length - pos);
                        pos += strlen(m_buffer);
                        break;
                    case 'i': case 'd':
                        itoa(va_arg(ap, int32_t), m_buffer);
                        strncpy(buffer+pos, m_buffer, length - pos);
                        pos += strlen(m_buffer);
                        break;
                    case 'X':
                        i2hex(va_arg(ap, uint32_t), m_buffer, 8);
                        strncpy(buffer+pos, m_buffer, length - pos);
                        pos += 8;
                        break;
                    case 'x':
                        i2hex(va_arg(ap, uint32_t), m_buffer, 4);
                        strncpy(buffer+pos, m_buffer, length - pos);
                        pos += 4;
                        break;
                    case 'y':
                        i2hex(va_arg(ap, uint32_t), m_buffer, 2);
                        strncpy(buffer+pos, m_buffer, length - pos);
                        pos += 2;
                        break;
                    case 's':
                    {
                        const char* string = va_arg(ap, const char*);
                        strncpy(buffer+pos, string, length - pos);
                        pos += strlen(string);
                        break;
                    }
                    case 'c':
                        buffer[pos] = (char)va_arg(ap, int32_t);
                        pos++;
                        buffer[pos] = 0;
                        break;
                    case '%':
                        buffer[pos] = '%';
                        pos++;
                        buffer[pos] = 0;
                        break;
                    default:
                        --args;
                        break;
                }
                break;
            default:
                buffer[pos] = (*args);
                pos++;
                break;
        }
    }
    if (pos < length)
        buffer[pos] = 0;
}

void snprintf(char *buffer, size_t length, const char *args, ...)
{
    va_list ap;
    va_start(ap, args);
    vsnprintf(buffer, length, args, ap);
    va_end(ap);
}

int vsscanf(const char* src, const char* format, va_list arg); /// TODO
int sscanf(const char* src, const char* format, ...); /// TODO


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
