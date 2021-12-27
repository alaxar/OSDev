#ifndef AUDIO_H
#define AUDIO_H

#include "pci.h"


typedef struct {
    void (*install)(pciDev_t*);
} audio_driver_t;


void audio_installDevice(pciDev_t* device);
void audio_test(void);


#endif
