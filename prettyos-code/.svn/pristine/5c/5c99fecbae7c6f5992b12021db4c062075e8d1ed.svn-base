#ifndef CDI_CMOS_H
#define CDI_CMOS_H

#include "../cmos.h"

/* Performs CMOS reads
   index:  Index within CMOS RAM to read from.
   return: Result of reading CMOS RAM at specified index. */
static inline uint8_t cdi_cmos_read(uint8_t index)
{
    return cmos_read(index); // Read byte from CMOS
}

/* Performs CMOS writes
   index: Index within CMOS RAM to write to.
   value: Value to write to the CMOS */
static inline void cdi_cmos_write(uint8_t index, uint8_t value)
{
    cmos_write(index, value); // Write byte to CMOS
}

#endif
