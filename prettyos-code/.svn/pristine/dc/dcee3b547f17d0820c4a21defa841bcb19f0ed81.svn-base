#ifndef TIMER_H
#define TIMER_H

#include "os.h"


#define SYSTEMFREQUENCY 250


void     timer_install(uint16_t sysfreq, bool i8254);
void     timer_setFrequency(uint16_t freq, bool i8254);
uint16_t timer_getFrequency(void);
uint32_t timer_getSeconds(void);
uint32_t timer_getMilliseconds(void);
uint64_t timer_getTicks(void);
uint32_t timer_millisecondsToTicks(uint32_t milliseconds);

void sleepSeconds(uint32_t seconds);
void sleepMilliSeconds(uint32_t ms);
void sleepMilliSecondsOld (uint32_t ms);
void delay(uint32_t microsec);


#endif
