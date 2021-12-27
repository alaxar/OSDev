#ifndef TIME_H
#define TIME_H

#include "os.h"


typedef struct
{
    uint8_t second;      // seconds 0-60 (60, because there can be a leap second
    uint8_t minute;      // minutes 0-59
    uint8_t hour;        // hours 0-23
    uint8_t weekday;     // weekday 1-7 (1 = sunday)
    uint8_t dayofmonth;  // day 1-31
    uint8_t month;       // month 1-12
    uint8_t year;        // year (2 digits)
    uint8_t century;     // century (2 digits)
} tm_t;


void cmosTime(tm_t* ptm);
void getCurrentDateAndTime(char* pStr, size_t strsize);


#endif
