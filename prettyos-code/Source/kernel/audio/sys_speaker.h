#ifndef SYS_SPEAKER_H
#define SYS_SPEAKER_H

#include "os.h"


void sound(uint32_t frequency);
void noSound(void);
void beep(uint32_t freq, uint32_t duration);
void msgbeep(void);


#endif
