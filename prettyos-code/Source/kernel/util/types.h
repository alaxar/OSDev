#ifndef TYPES_H
#define TYPES_H


typedef unsigned int        size_t;
typedef unsigned long long  uint64_t;
typedef unsigned long       uint32_t;
typedef unsigned short      uint16_t;
typedef unsigned char       uint8_t;
typedef signed long long    int64_t;
typedef signed long         int32_t;
typedef signed short        int16_t;
typedef signed char         int8_t;
typedef uint32_t            uintptr_t;
#ifndef __bool_true_false_are_defined
  typedef _Bool             bool;
  #define true  1
  #define false 0
  #define __bool_true_false_are_defined 1
#endif

typedef __builtin_va_list va_list;
#define va_start(ap, X)    __builtin_va_start(ap, X)
#define va_arg(ap, type)   __builtin_va_arg(ap, type)
#define va_end(ap)         __builtin_va_end(ap)
#define va_copy(dest, src) __builtin_va_copy(dest, src)


#endif
