/*
*  license and disclaimer for the use of this source code as per statement below
*  Lizenz und Haftungsausschluss für die Verwendung dieses Sourcecodes siehe unten
*/

#include "time.h"
#include "util/util.h"
#include "cmos.h"


void cmosTime(tm_t* ptm)
{
    ptm->second     = BCDtoDecimal(cmos_read(CMOS_SECOND));
    ptm->minute     = BCDtoDecimal(cmos_read(CMOS_MINUTE));
    ptm->hour       = BCDtoDecimal(cmos_read(CMOS_HOUR));
    ptm->dayofmonth = BCDtoDecimal(cmos_read(CMOS_DAYOFMONTH));
    ptm->month      = BCDtoDecimal(cmos_read(CMOS_MONTH));
    ptm->year       = BCDtoDecimal(cmos_read(CMOS_YEAR));
    ptm->century    = BCDtoDecimal(cmos_read(CMOS_CENTURY));
}

static const uint16_t days[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

static bool isLeapyear(uint16_t year)
{
    return (year % 4 == 0) && ((year % 100 != 0) || (year % 400 == 0));
}

// Gregorian calender started 15th October 1582
static uint8_t calculateWeekday(uint16_t year, uint8_t month, int32_t day)
{
    day += 6; // 1.1.1600 was a saturday
    day += (year/*-1600*/ * 146097)/400 + days[month-1];

    if (isLeapyear(year) && (month < 2 || (month == 2 && day <= 28)))
    {
        day--;
    }

    return (day % 7 + 1);
}

static void writeInt(uint16_t val, char* dest, size_t strsize)
{
    if (val<10)
        snprintf(dest, strsize, "0%u", val);
    else
        snprintf(dest, strsize, "%u",  val);
}

static const char* const weekdays[] =
{
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
};
static const char* const months[] =
{
    "January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"
};

void getCurrentDateAndTime(char* pStr, size_t strsize)
{
    static tm_t pct = {.dayofmonth = 0xFF};
    static char dayofmonth[3];

    uint8_t temp = pct.dayofmonth;
    cmosTime(&pct);

    if (temp != pct.dayofmonth)
    {
        pct.weekday = calculateWeekday(100*pct.century+pct.year, pct.month, pct.dayofmonth);

        writeInt(pct.dayofmonth, dayofmonth, 3);
    }
    char hour[3], minute[3], second[3];
    writeInt(pct.hour, hour, 3);
    writeInt(pct.minute, minute, 3);
    writeInt(pct.second, second, 3);

    snprintf(pStr, strsize, "%s, %s %s, %u, %s:%s:%s", weekdays[pct.weekday-1], months[pct.month-1], dayofmonth,
                                                       pct.century*100+pct.year, hour, minute, second);
}


/*
* Copyright (c) 2009-2015 The PrettyOS Project. All rights reserved.
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
