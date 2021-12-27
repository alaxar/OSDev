#ifndef STDLIB_H
#define STDLIB_H

#include "stddef.h"

#define EXIT_SUCCESS      0
#define EXIT_FAILURE     -1
#define MB_CUR_MAX
#define RAND_MAX     0x7FFF  // do not change


typedef struct
{
    int quot, rem;
} div_t;

typedef struct
{
    long quot, rem;
} ldiv_t;

typedef struct
{
    long long quot, rem;
} lldiv_t;


#ifdef _cplusplus
extern "C" {
#endif

void abort(void);
void exit(int status);
void quick_exit(int status);
void _Exit(int status);
int atexit(void (*func)(void));
int at_quick_exit(void (*func)(void));

int abs(int n);
long labs(long n);
long long llabs(long long n);

div_t div(int numerator, int denominator);
ldiv_t ldiv(long numerator, long denominator);
lldiv_t lldiv(long long numerator, long long denominator);

int atoi(const char* nptr);
long int atol(const char* nptr);
long long atoll(const char* nptr);
double atof(const char *nptr);
float strtof(const char* nptr, char** endptr);
double strtod(const char* nptr, char** endptr);
long double strtold(const char* nptr, char** endptr);
long int strtol(const char* nptr, char** endptr, int base);
long long strtoll(const char* nptr, char** endptr, int base);
unsigned long int strtoul(const char* nptr, char** endptr, int base);
unsigned long long strtoull(const char* nptr, char** endptr, int base);

int mblen(const char* pmb, size_t max);
size_t mbstowcs(wchar_t* wcstr, const char* mbstr, size_t max);
int mbtowc(wchar_t* pwc, const char* pmb, size_t max);
size_t wcstombs(char* mbstr, const wchar_t* wcstr, size_t max);
int wctomb(char* pmb, wchar_t character);

void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*comparator)(const void*, const void*));
void qsort(void* base, size_t nmemb, size_t size, int (*compar)(const void*, const void*));

void srand(int val);
int rand(void);
double random(double lower, double upper);

void* malloc(size_t size);
void* calloc(size_t num, size_t size);
void* realloc(void* ptr, size_t size);
void free(void* ptr);

char* getenv(const char* name);
int system(const char* command);

#ifdef _cplusplus
}
#endif

#endif
