#ifndef STRING_H
#define STRING_H

#include "stddef.h"


#ifdef _cplusplus
extern "C" {
#endif

void* memchr(void* ptr, char value, size_t num);
int memcmp(const void* ptr1, const void* ptr2, size_t num);
void* memcpy(void* dest, const void* src, size_t bytes);
void* memmove(void* dest, const void* src, size_t bytes);
void* memset(void* dest, char val, size_t bytes);

size_t strlen(const char* str);
int strcmp(const char* s1, const char* s2);
int strncmp(const char* s1, const char* s2, size_t n);
char* strcpy(char* dest, const char* src);
char* strncpy(char* dest, const char* src, size_t n);
char* strcat(char* dest, const char* src);
char* strncat(char* dest, const char* src, size_t n);
char* strchr(const char* str, char character);
int strcoll(const char* str1, const char* str2);
size_t strcspn(const char* str, const char* key);
char* strerror(int errornum);
char* strpbrk(const char* str, const char* delim);
char* strrchr(const char* s, int c);
size_t strspn(const char* str, const char* key);
char* strstr(const char* str1, const char* str2);
char* strtok(char* str, const char* delimiters);
size_t strxfrm(char* destination, const char* source, size_t num);

#ifdef _cplusplus
}
#endif

#endif
