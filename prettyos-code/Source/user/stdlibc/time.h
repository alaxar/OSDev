#ifndef TIME_H
#define TIME_H

#include "stddef.h"

#define CLOCKS_PER_SEC

struct tm {
    int tm_sec, tm_min, tm_hour, tm_mday, tm_mon, tm_year, tm_wday, tm_yday, tm_isdst;
};


typedef unsigned int time_t;
typedef unsigned int clock_t;


#ifdef _cplusplus
extern "C" {
#endif

char* asctime(const struct tm* timeptr);
clock_t clock(void);
char* ctime(const time_t* time);
double difftime(time_t t1, time_t t2);
struct tm* gmtime(const time_t* timer);
struct tm* localtime(const time_t* timer);
time_t mktime(struct tm* timeptr);
size_t strftime(char* ptr, size_t maxsize, const char* format, const struct tm* timeptr);
time_t time(time_t* timer);

#ifdef _cplusplus
}
#endif

#endif
