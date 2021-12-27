#ifndef UTIL_H
#define UTIL_H

#include "os.h"


#define isdigit(c) ((c) >= '0' && (c) <= '9')
#define isalpha(c) (((c) >= 'a' && (c) <= 'z') || ((c) >= 'A' && (c) <= 'Z'))
#define isalnum(c) (isdigit(c) || isalpha(c))
#define isupper(c) ((c) >= 'A' && (c) <= 'Z')
#define islower(c) ((c) >= 'a' && (c) <= 'z')

#define BIT(n) (1U<<(n))
#define IS_BIT_SET(value, pos) ((value)&BIT(pos))
#define CLEAR_BIT(val, bit) __asm__("btr %1, %0" : "+g"(val) : "r"(bit))
#define SET_BIT(val, bit) __asm__("bts %1, %0" : "+g"(val) : "r"(bit))
static inline int bsr(int val)
{
    int result;
    __asm__("bsr %1, %0" : "=r"(result) : "r"(val));
    return result;
}

#define BYTE1(a) ( (a)      & 0xFF)
#define BYTE2(a) (((a)>> 8) & 0xFF)
#define BYTE3(a) (((a)>>16) & 0xFF)
#define BYTE4(a) (((a)>>24) & 0xFF)

#define WORD1(a) ( (a)      & 0xFFFF)
#define WORD2(a) (((a)>>16) & 0xFFFF)

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <= (b) ? (a) : (b))
#define clamp(minimum, x, maximum) (max(min(x, maximum), minimum))

#define NAN (__builtin_nanf(""))
#define NULL 0

#define offsetof(st, m) ((size_t) ( (char*)&((st *)(0))->m - (char*)0 ))
#define getField(addr, byte, shift, len) ((((uint8_t*)(addr))[byte]>>(shift)) & (BIT(len)-1))

#define ASSERT(b) ((b) ? (void)0 : panic_assert(__FILE__, __LINE__, #b))
void panic_assert(const char* file, uint32_t line, const char* desc);

#define WAIT_FOR_CONDITION(condition, runs, wait, message...)\
    for (unsigned int timeout_ = 0; !(condition); timeout_++) {\
        if (timeout_ >= runs) {\
             printfe(message);\
             break;\
        }\
        sleepMilliSeconds(wait);\
    }


static inline void nop(void) { __asm__ volatile ("nop"); } // Do nothing
static inline void hlt(void) { __asm__ volatile ("hlt"); } // Wait until next interrupt
static inline void sti(void) { __asm__ volatile ("sti"); } // Enable interrupts
static inline void cli(void) { __asm__ volatile ("cli"); } // Disable interrupts
static inline uint64_t rdtsc(void)
{
    uint64_t val;
    __asm__ volatile ("rdtsc" : "=A"(val)); // "=A" for getting 64 bit value
    return val;
}

static inline uint8_t inportb(uint16_t port)
{
    uint8_t ret_val;
    __asm__ volatile ("inb %1, %0" : "=a"(ret_val) : "Nd"(port));
    return ret_val;
}

static inline uint16_t inportw(uint16_t port)
{
    uint16_t ret_val;
    __asm__ volatile ("inw %1, %0" : "=a" (ret_val) : "Nd"(port));
    return ret_val;
}

static inline uint32_t inportl(uint16_t port)
{
    uint32_t ret_val;
    __asm__ volatile ("inl %1, %0" : "=a" (ret_val) : "Nd"(port));
    return ret_val;
}

static inline void repinsb(uint16_t port, uint8_t* buf, uint32_t count)
{
    uint32_t temp = (uint32_t)buf;
    __asm__ volatile ("rep insb" : "+D"(temp), "+c"(count) : "Nd"(port) : "memory");
}

static inline void repinsw(uint16_t port, uint16_t* buf, uint32_t count)
{
    uint32_t temp = (uint32_t)buf;
    __asm__ volatile ("rep insw" : "+D"(temp), "+c"(count)  : "Nd"(port) : "memory");
}

static inline void repinsl(uint16_t port, uint32_t* buf, uint32_t count)
{
    uint32_t temp = (uint32_t)buf;
    __asm__ volatile ("rep insl" : "+D"(temp), "+c"(count)  : "Nd"(port) : "memory");
}

static inline void outportb(uint16_t port, uint8_t val)
{
    __asm__ volatile ("outb %0, %1" :: "a"(val), "Nd"(port));
}

static inline void outportw(uint16_t port, uint16_t val)
{
    __asm__ volatile ("outw %0, %1" :: "a"(val), "Nd"(port));
}

static inline void outportl(uint16_t port, uint32_t val)
{
    __asm__ volatile ("outl %0, %1" :: "a"(val), "Nd"(port));
}

static inline void repoutsb(uint16_t port, uint8_t* buf, uint32_t count)
{
    uint32_t temp = (uint32_t)buf;
    __asm__ volatile ("rep outsb" : "+S"(temp), "+c"(count) : "Nd"(port));
}

static inline void repoutsw(uint16_t port, uint16_t* buf, uint32_t count)
{
    uint32_t temp = (uint32_t)buf;
    __asm__ volatile ("rep outsw" : "+S"(temp), "+c"(count) : "Nd"(port));
}

static inline void repoutsl(uint16_t port, uint32_t* buf, uint32_t count)
{
    uint32_t temp = (uint32_t)buf;
    __asm__ volatile ("rep outsl" : "+S"(temp), "+c"(count) : "Nd"(port));
}

static inline uint32_t alignUp(uint32_t val, uint32_t alignment)
{
    if (!alignment)
        return val;
    --alignment;
    return (val+alignment) & ~alignment;
}

static inline uint32_t alignDown(uint32_t val, uint32_t alignment)
{
    if (!alignment)
        return val;
    return val & ~(alignment-1);
}

void      memshow(const void* start, size_t count, bool alpha);
void*     memset(void* dest, uint8_t val, size_t bytes);
uint16_t* memsetw(uint16_t* dest, uint16_t val, size_t words);
uint32_t* memsetl(uint32_t* dest, uint32_t val, size_t dwords);
void*     memcpy(void* dest, const void* src, size_t bytes);
void*     memmove(void* destination, const void* source, size_t size);
int32_t   memcmp(const void* s1, const void* s2, size_t n);

size_t  vsnprintf(char *buffer, size_t length, const char *args, va_list ap);
size_t  snprintf (char *buffer, size_t length, const char *args, ...);
size_t  strlen(const char* str);
int32_t strcmp(const char* s1, const char* s2);
int32_t strncmp(const char* s1, const char* s2, size_t n);
int32_t strncicmp(const char* s1, const char* s2, size_t n);
char*   strcpy(char* dest, const char* src);
char*   strncpyandfill(char* dest, const char* src, size_t n, char val);
static inline char* strncpy(char* dest, const char* src, size_t n) { return (strncpyandfill(dest, src, n, 0)); }
char*   strcat(char* dest, const char* src);
char*   strncat(char* dest, const char* src, size_t n);
char*   strchr(const char* str, int character);
char*   strpbrk(const char* str, const char* delim);
char*   strdup(const char* src, const char* description);

static inline char toLower(char c) { return isupper(c) ? ('a' - 'A') + c : c; }
static inline char toUpper(char c) { return islower(c) ? ('A' - 'a') + c : c; }
char* toupper(char* s);
char* tolower(char* s);

char* gets_s(char* s, size_t l);
void  waitForKeyStroke(void);


typedef enum
{
    STANDBY, SHUTDOWN, REBOOT
} SYSTEM_CONTROL;

void systemControl(SYSTEM_CONTROL todo); // Reboot, Shutdown, ...

void printStackTrace(void* eip, void* ebp);

void bootscreen(void);

void   reverse(char* s);
int8_t ctoi(char c);
int    atoi(const char* s);
float  atof(const char* s);
char*  itoa(int32_t n,  char* s);
char*  utoa(uint32_t n, char* s);
void   ftoa(float f, char* buffer);
void   i2hex(uint32_t val, char* dest, int32_t len);

uint8_t BCDtoDecimal(uint8_t packedBCDVal);

uint32_t abs(int32_t arg);
double   fabs(double x);
double   sqrt(double x);

void srand(uint32_t val);
uint32_t rand(void);


#endif
