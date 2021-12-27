#ifndef SERIAL_H
#define SERIAL_H

#include "os.h"


// Most emulators implement serial interface with output to a file, e.g. called "serial1.txt" (qemu: -serial file:serial1.txt)

// BDA
#define BDA_COM1_BASE     0x400
#define BDA_COM2_BASE     0x402
#define BDA_COM3_BASE     0x404
#define BDA_COM4_BASE     0x406
#define BDA_EQUIPMENTWORD 0x410

// COM
#define TRANSMITTINGBUFFER   0
#define INTERRUPTENABLE      1
#define FIFOCONTROL          2
#define LINECONTROL          3
#define MODEMCONTROL         4
#define LINESTATUS           5
#define MODEMSTATUS          6
#define SCRATCH              7

#define DLAB_BIT             BIT(7)

void serial_init(void);
void serial_write(uint8_t com, char a);
bool serial_received(uint8_t com);
char serial_read(uint8_t com);
bool serial_isTransmitEmpty(uint8_t com);
void serial_vprintf(uint8_t com, const char* msg, va_list ap);
void serial_printf(uint8_t com, const char* msg, ...);

#ifdef _SERIAL_LOG_
  #define serial_log serial_printf
#else
  static inline void serial_log(uint8_t com, const char* msg, ...) {}
#endif


#endif
