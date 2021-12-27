/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "util.h"
#include "audio/sys_speaker.h"
#include "video/console.h"
#include "timer.h"
#include "power_management.h"
#include "hid/keyboard.h"
#include "tasking/task.h"
#include "kheap.h"


/**********************************************************************/

void panic_assert(const char* file, uint32_t line, const char* desc)
{
    cli();
    console_current = &kernelConsole;
    console_display(11); // We need to switch to the kernel console before the system is halted. Otherwise the output is invisible.
    printStackTrace(0, 0);
    printfe("\n\nASSERTION FAILED (%s) in file '%s', line %u\nOPERATING SYSTEM HALTED!", desc, file, line);
    hlt();
}

/**********************************************************************/

void memshow(const void* start, size_t count, bool alpha)
{
    for (size_t i = 0; i < count; i++)
    {
        if (alpha)
        {
            putch(((const char*)start)[i]);
        }
        else
        {
            if (i%16 == 0)
                putch('\n');
            printf("%y ", ((const uint8_t*)start)[i]);
        }
    }
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

void* memmove(void* destination, const void* source, size_t size)
{
    if (source == destination || size == 0) // Copying is not necessary. Calling memmove with source==destination or size==0 is not a bug.
    {
        return (destination);
    }

    // Check if either one of the memory regions is beyond the end of the
    // address space. We think that trying to copy from or to beyond the end of
    // the address space is a bug of the caller.
    // In future versions of this function, an exception will be thrown in this
    // case. For now, just return and do nothing.
    // The subtraction used to calculate the value of "max" cannot produce an
    // underflow because size can neither be greater than the maximum value of a
    // variable of type uintp or 0.
    const uintptr_t memMax = ~((uintptr_t)0) - (size - 1); // ~0 is the highest possible value of the variables type
    if ((uintptr_t)source > memMax || (uintptr_t)destination > memMax)
    {
        return (destination);
    }

    // The source overlaps with the destination and the destination is after the
    // source in memory. Coping from start to the end of source will overwrite
    // the last (size - (destination - source)) bytes of source with the first
    // ones. Therefore it is necessary to copy from the end to the start of
    // source and destination. Let us look at an example. Each letter or space
    // is one byte in memory.
    // |source     |
    // |      destination|
    // source starts at 0. destination at 6. Coping from start to end will
    // overwrite the last 5 bytes of the source.
    if (source < destination)
    {
        memcpyr(destination, source, size);
    }
    else // In all other cases, it is ok to copy from the start to the end of source.
    {
        memcpy(destination, source, size); // We assume, that memcpy does forward copy
    }
    return (destination);
}

void* memset(void* dest, uint8_t val, size_t bytes)
{
    void* retval = dest;
    size_t dwords = bytes/4; // Number of dwords (4 Byte blocks) to be written
    bytes %= 4;              // Remaining bytes
    uint32_t dval = (val<<24)|(val<<16)|(val<<8)|val; // Create dword from byte value
    __asm__ volatile("cld\n"
                     "rep stosl\n"
                     "mov %%edx, %%ecx\n"
                     "rep stosb" : "+D"(dest), "+c"(dwords) : "a"(dval), "d"(bytes) : "memory");
    return retval;
}

uint16_t* memsetw(uint16_t* dest, uint16_t val, size_t words)
{
    uint16_t* retval = dest;
    size_t dwords = words/2; // Number of dwords (4 Byte blocks) to be written
    words %= 2;              // Remaining words
    uint32_t dval = (val<<16)|val; // Create dword from byte value
    __asm__ volatile("cld\n"
                     "rep stosl\n"
                     "mov %%edx, %%ecx\n"
                     "rep stosw" : "+D"(dest), "+c"(dwords) : "a"(dval), "d"(words) : "memory");
    return retval;
}

uint32_t* memsetl(uint32_t* dest, uint32_t val, size_t dwords)
{
    uint32_t* retval = dest;
    __asm__ volatile("cld\n"
                     "rep stosl" : "+D"(dest), "+c"(dwords) : "a"(val) : "memory");
    return retval;
}

int32_t memcmp(const void* s1, const void* s2, size_t n)
{
    if (n == 0) return (0);

    const uint8_t* v1 = s1;
    const uint8_t* v2 = s2;
    for (; n > 1 && *v1 == *v2; n--)
    {
        ++v1;
        ++v2;
    }
    return (*v1 - *v2);
}

/**********************************************************************/

char* gets_s(char* s, size_t l)
{
    int32_t i = 0;
    char c;
    do
    {
        c = getch();
        if (c=='\b')  // Backspace
        {
            if (i>0)
            {
                putch(c);
                if (i < l-2)
                    s[i-1]='\0';
                --i;
            }
        }
        else
        {
            if (c != '\n')
            {
                if(i < l-1)
                    s[i] = c;
                i++;
            }
            putch(c);
        }
    }
    while (c != '\n'); // Linefeed

    s[min(l-1, i)] = '\0';

    return (s);
}

void waitForKeyStroke(void)
{
    textColor(LIGHT_GRAY);
    printf("\n             - - - - - - - - - - - press key - - - - - - - - - - -");
    textColor(TEXT);
    getch();
}

/**********************************************************************/

size_t vsnprintf(char* buffer, size_t length, const char* args, va_list ap)
{
    char m_buffer[32]; // Larger is not needed at the moment

    size_t pos;
    size_t count;
    for (pos = 0; *args && pos < length-1; args++)
    {
        switch (*args)
        {
            case '%':
                switch (*(++args))
                {
                    case 'u':
                        utoa(va_arg(ap, uint32_t), m_buffer);
                        count = min(strlen(m_buffer), length - pos - 1);
                        memcpy(buffer+pos, m_buffer, count);
                        pos += count;
                        break;
                    case 'f':
                        ftoa(va_arg(ap, double), m_buffer);
                        count = min(strlen(m_buffer), length - pos - 1);
                        memcpy(buffer+pos, m_buffer, count);
                        pos += count;
                        break;
                    case 'i': case 'd':
                        itoa(va_arg(ap, int32_t), m_buffer);
                        count = min(strlen(m_buffer), length - pos - 1);
                        memcpy(buffer+pos, m_buffer, count);
                        pos += count;
                        break;
                    case 'X':
                        i2hex(va_arg(ap, uint32_t), m_buffer, 8);
                        count = min(8, length - pos - 1);
                        memcpy(buffer+pos, m_buffer, count);
                        pos += count;
                        break;
                    case 'x':
                        i2hex(va_arg(ap, uint32_t), m_buffer, 4);
                        count = min(4, length - pos - 1);
                        memcpy(buffer+pos, m_buffer, count);
                        pos += count;
                        break;
                    case 'y':
                        i2hex(va_arg(ap, uint32_t), m_buffer, 2);
                        count = min(2, length - pos - 1);
                        memcpy(buffer + pos, m_buffer, count);
                        pos += count;
                        break;
                    case 's':
                    {
                        const char* string = va_arg(ap, const char*);
                        count = min(strlen(string), length - pos - 1);
                        memcpy(buffer+pos, string, count);
                        pos += count;
                        break;
                    }
                    case 'c':
                        buffer[pos] = (char)va_arg(ap, int32_t);
                        pos++;
                        break;
                    case '%':
                        buffer[pos] = '%';
                        pos++;
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
    buffer[pos] = 0;
    return (pos);
}

size_t snprintf(char* buffer, size_t length, const char* args, ...)
{
    va_list ap;
    va_start(ap, args);
    size_t retval = vsnprintf(buffer, length, args, ap);
    va_end(ap);
    return (retval);
}

size_t strlen(const char* str)
{
    size_t retval = 0;
    for (; *str != '\0'; ++str)
        ++retval;
    return retval;
}

// Compare two strings. Returns -1 if str1 < str2, 0 if they are equal or 1 otherwise.
int32_t strcmp(const char* s1, const char* s2)
{
    while ((*s1) && (*s1 == *s2))
    {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

int32_t strncmp(const char* s1, const char* s2, size_t n)
{
    if (n == 0) return (0);

    for (; *s1 && n > 1 && *s1 == *s2; n--)
    {
        ++s1;
        ++s2;
    }
    return (*s1 - *s2);
}

int32_t strncicmp(const char* s1, const char* s2, size_t n)
{
    if (n == 0) return (0);

    for (; *s1 && n > 1 && toLower(*s1) == toLower(*s2); n--)
    {
        ++s1;
        ++s2;
    }
    return (toLower(*s1) - toLower(*s2));
}

/// http://en.wikipedia.org/wiki/Strcpy
// Copy the NUL-terminated string src into dest, and return dest.
char* strcpy(char* dest, const char* src)
{
    char* save = dest;
    while ((*dest++ = *src++));
    return save;
}

char* strncpyandfill(char* dest, const char* src, size_t n, char val)
{
    size_t i = 0;
    for (; i < n && src[i] != 0; i++)
    {
        dest[i] = src[i];
    }
    memset(dest+i, (uint8_t)val, n-i);
    return (dest);
}

/// http://en.wikipedia.org/wiki/Strcat
char* strcat(char* dest, const char* src)
{
    strcpy(dest + strlen(dest), src);
    return dest;
}

char* strncat(char* dest, const char* src, size_t n)
{
    strncpy(dest + strlen(dest), src, n);
    return dest;
}

char* strchr(const char* str, int character)
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

char* strpbrk(const char* str, const char* delim)
{
    for (; *str != 0; str++)
        for (size_t i = 0; delim[i] != 0; i++)
            if (*str == delim[i])
                return ((char*)str);

    return (0);
}

char* strdup(const char* src, const char* description)
{
    size_t length = strlen(src)+1;
    char* newStr = malloc(length, 0, description);
    memcpy(newStr, src, length);
    return newStr;
}

/**********************************************************************/

char* toupper(char* s)
{
    for (size_t i = 0; s[i] != 0; i++)
    {
        s[i] = toUpper(s[i]);
    }
    return s;
}

char* tolower(char* s)
{
    for (size_t i = 0; s[i] != 0; i++)
    {
        s[i] = toLower(s[i]);
    }
    return s;
}

/**********************************************************************/

/// http://en.wikipedia.org/wiki/Itoa
void reverse(char* s)
{
    for (size_t i=0, j=strlen(s)-1; i<j; i++, j--)
    {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}

int8_t ctoi(char c)
{
    if (c < 48 || c > 57)
        return (-1);
    return (c-48);
}

char* itoa(int32_t n, char* s)
{
    if (n < 0)
    {
        s[0] = '-';
        utoa(-n, s + 1);
        return s;
    }
    return utoa(n, s);
}

char* utoa(uint32_t n, char* s)
{
    uint32_t i = 0;
    do // generate digits in reverse order
    {
        s[i++] = n % 10 + '0'; // get next digit
    }
    while ((n /= 10) > 0);     // delete it
    s[i] = '\0';
    reverse(s);
    return (s);
}

void i2hex(uint32_t val, char* dest, int32_t len)
{
    char* cp = &dest[len];
    while (cp > dest)
    {
        char x = val & 0xF;
        val >>= 4;
        *--cp = x + ((x > 9) ? 'A' - 10 : '0');
    }
    dest[len]='\0';
}

int atoi(const char* s)
{
    int num = 0;
    bool sign = false;
    for (size_t i=0; s[i] != 0; i++)
    {
        if (s[i] >= '0' && s[i] <= '9')
        {
            num = num * 10 + s[i] -'0';
        }
        else if (s[0] == '-' && i==0)
        {
            sign = true;
        }
        else
        {
            break;
        }
    }
    if (sign)
    {
        num *= -1;
    }
    return num;
}

float atof(const char* s)
{
    int32_t i = 0;
    int8_t sign = 1;
    while (s[i] == ' ' || s[i] == '+' || s[i] == '-')
    {
        if (s[i] == '-')
        {
            sign *= -1;
        }
        i++;
    }

    float val;
    for (val = 0.0f; isdigit(s[i]); i++)
    {
        val = 10.0f * val + s[i] - '0';
    }
    if (s[i] == '.')
    {
        i++;
    }
    float pow;
    for (pow = 1.0f; isdigit(s[i]); i++)
    {
        val = 10.0f * val + s[i] - '0';
        pow *= 10.0f;
    }
    return (sign * val / pow);
}

void ftoa(float f, char* buffer)
{
    if (f < 0)
        *(buffer++) = '-';
    int32_t i = (int32_t)f;
    itoa(i < 0 ? -i : i, buffer);

    if (f < 0.0f)
        f = -f;

    buffer += strlen(buffer);
    *buffer = '.';
    ++buffer;

    *buffer++ = ((uint32_t)(f * 10.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 100.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 1000.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 10000.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 100000.0f) % 10) + '0';
    *buffer++ = ((uint32_t)(f * 1000000.0f) % 10) + '0';
    *buffer   = '\0';
}


uint8_t BCDtoDecimal(uint8_t packedBCDVal)
{
    return ((packedBCDVal >> 4) * 10 + (packedBCDVal & 0xF));
}

/**********************************************************************/

void systemControl(SYSTEM_CONTROL todo) // TODO: Improve it.
{
    switch (todo)
    {
        case STANDBY:
            if (!powmgmt_action(PM_STANDBY))
                puts("Standby failed");
            break;
        case REBOOT:
            if (!powmgmt_action(PM_REBOOT))
                puts("Rebooting failed");
            break;
        case SHUTDOWN:
            if (!powmgmt_action(PM_SOFTOFF))
                puts("Shutdown failed");
            break;
    }
}

void printStackTrace(void* eip, void* ebp)
{
    textColor(HEADLINE);
    printf("\nStack backtrace:");
    textColor(TEXT);

    struct stackFrame
    {
        struct stackFrame* ebp;
        uintptr_t eip;
    } *frame;

    if (ebp != 0)
    {
        printf("\nebp: %X   eip: %X", ebp, eip);
        frame = ebp;
    }
    else
    {
        __asm__("mov %%ebp, %0" : "=r"(frame));
    }

    for (; frame != 0 && frame->ebp != 0; frame = frame->ebp)
    {
        printf("\nebp: %X   eip: %X", frame->ebp, frame->eip);
        if (!paging_getPhysAddr(frame->ebp))
        {
            printf("   Not mapped. Backtrace finished.");
            break;
        }
    }
}

// BOOTSCREEN
#ifdef _BOOTSCREEN_
static void bootsound(void)
{
    // Melody
    // C Es F G F Es
    // C E F G F E C
    // http://www.flutepage.de/deutsch/goodies/frequenz.shtml (German)
    // http://www.flutepage.de/englisch/goodies/frequenz.shtml (English)

    beep(523, 200); // C
    beep(622, 200); // Es
    beep(689, 200); // F
    beep(784, 200); // G
    beep(689, 200); // F
    beep(622, 200); // Es
    beep(523, 200); // C
    beep(659, 200); // E
    beep(689, 200); // F
    beep(784, 200); // G
    beep(689, 200); // F
    beep(659, 200); // E

    beep(523, 1000); // C
}

void bootscreen(void)
{
    task_t* soundtask = create_thread(&bootsound, "bootsound");
    scheduler_insertTask(soundtask);
    textColor(GRAY);
    puts("\n\n\n\n\n");
    puts("      #######                    #    #               ####       #####\n");
    textColor(TEXT);
    puts("     #######");
    textColor(GRAY);
    puts("##                  ");
    textColor(TEXT);
    putch('#');
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    putch('#');
    textColor(GRAY);
    puts("#             #");
    textColor(TEXT);
    puts("####");
    textColor(GRAY);
    puts("###    ");
    textColor(TEXT);
    puts("#####");
    textColor(GRAY);
    puts("##\n");
    textColor(TEXT);
    puts("     ########");
    textColor(GRAY);
    puts("##                ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#            ");
    textColor(TEXT);
    puts("########");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("#######");
    textColor(GRAY);
    puts("##\n");
    textColor(TEXT);
    puts("     ##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("###");
    textColor(GRAY);
    puts("#  ## ##   ###   ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("###");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#####     ## ");
    textColor(TEXT);
    puts("##    ##");
    textColor(GRAY);
    puts("## ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("###");
    textColor(GRAY);
    puts("#\n");
    textColor(TEXT);
    puts("     ##");
    textColor(GRAY);
    puts("#    ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("# ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    putch('#');
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("###");
    textColor(GRAY);
    puts("## ");
    textColor(TEXT);
    puts("############");
    textColor(GRAY);
    puts("#    ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    putch('#');
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#     ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("# ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("###  ");
    textColor(TEXT);
    puts("##\n");
    puts("     ##");
    textColor(GRAY);
    puts("####");
    textColor(TEXT);
    puts("###  #####  #####");
    textColor(GRAY);
    puts("##");
    textColor(TEXT);
    puts("############");
    textColor(GRAY);
    puts("##   ");
    textColor(TEXT);
    puts("## ##");
    textColor(GRAY);
    puts("#     ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("# ");
    textColor(TEXT);
    puts("####");
    textColor(GRAY);
    puts("####\n");
    textColor(TEXT);
    puts("     ########   ###   ##");
    textColor(GRAY);
    puts("###");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("# ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("# ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#     ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("######");
    textColor(GRAY);
    puts("##\n");
    textColor(TEXT);
    puts("     #######    ##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("#######");
    textColor(GRAY);
    puts("# ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("## ");
    textColor(TEXT);
    puts("##  ##");
    textColor(GRAY);
    puts("#     ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#    ");
    textColor(TEXT);
    puts("#####");
    textColor(GRAY);
    puts("##\n");
    textColor(TEXT);
    puts("     ##");
    textColor(GRAY);
    puts("#        ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("#######  ##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    putch('#');
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#     ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ##   ");
    textColor(TEXT);
    puts("###");
    textColor(GRAY);
    puts("#\n");
    textColor(TEXT);
    puts("     ##");
    textColor(GRAY);
    puts("#        ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("##  ## ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    putch('#');
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("##    ");
    textColor(TEXT);
    puts("##  ##");
    textColor(GRAY);
    puts("##   ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#\n");
    textColor(TEXT);
    puts("     ##");
    textColor(GRAY);
    puts("#        ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#   ");
    textColor(TEXT);
    puts("###");
    textColor(GRAY);
    puts("##");
    textColor(TEXT);
    puts("##  ##");
    textColor(GRAY);
    puts("###");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("### ");
    textColor(TEXT);
    puts("#####    ##");
    textColor(GRAY);
    puts("####");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("###");
    textColor(GRAY);
    puts("###");
    textColor(TEXT);
    puts("###\n");
    puts("     ##");
    textColor(GRAY);
    puts("#        ");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#    ");
    textColor(TEXT);
    puts("#####   ####");
    textColor(GRAY);
    putch('#');
    textColor(TEXT);
    puts("####");
    textColor(GRAY);
    puts("#  ");
    textColor(TEXT);
    puts("###");
    textColor(GRAY);
    puts("#    ");
    textColor(TEXT);
    puts("########    #######\n");
    puts("     ##         ##      ###     ###  ###   ###       ####       #####\n");
    textColor(GRAY);
    puts("                                          #");
    textColor(TEXT);
    puts("##");
    textColor(GRAY);
    puts("#\n");
    textColor(TEXT);
    puts("                                         ####\n");
    puts("                                         ###\n");
    puts("\n\n\n\n\n");
    puts("     ####################################################################\n");
    puts("     #                                                                  #\n");
    puts("     #                                                                  #\n");
    puts("     #                                                                  #\n");
    puts("     #                                                                  #\n");
    puts("     ####################################################################\n");
    puts("\n\n\n\n\n\n");

    textColor(YELLOW);
    puts("                  Copyright (c) 2009-2017  The PrettyOS Team");
    textColor(TEXT);

    for (uint8_t x = 6; x < 72; x++)
    {
        sleepMilliSeconds(30);
        console_setPixel(x, 27, 0x0200|'#');
        console_setPixel(x, 28, 0x0200|'#');
        console_setPixel(x, 29, 0x0200|'#');
        console_setPixel(x, 30, 0x0200|'#');
    }

    waitForTask(soundtask, 2000); // Wait for sound

  #ifdef _DIAGNOSIS_
    scheduler_log();
    sleepSeconds(2);
  #endif
}
#endif

uint32_t abs(int32_t arg)
{
    if (arg < 0)
        arg = -arg;
    return (uint32_t)(arg);
}

double fabs(double x)
{
    double result;
    __asm__("fabs" : "=t"(result) : "0"(x));
    return result;
}

double sqrt(double x)
{
    if (x <  0.0)
        return NAN;

    double result;
    __asm__("fsqrt" : "=t"(result) : "0"(x));
    return result;
}

static uint32_t seed = 0;

void srand(uint32_t val)
{
    seed = val;
}

uint32_t rand(void)
{
    return (((seed = seed * 214013L + 2531011L) >> 16) & 0x7FFF);
}


/*
* Copyright (c) 2009-2017 The PrettyOS Project. All rights reserved.
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
