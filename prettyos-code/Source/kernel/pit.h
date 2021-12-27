#ifndef PIT_H
#define PIT_H


// Counter 0, 1, 2
#define TIMECOUNTER_i8254_FREQU  1193182
#define COUNTER_0_DATAPORT       0x40              // Channel 0 data port (read/write)
#define COUNTER_1_DATAPORT       0x41              // Channel 1 data port (read/write)
#define COUNTER_2_DATAPORT       0x42              // Channel 2 data port (read/write), OUT connected to speaker
#define COMMANDREGISTER          0x43              // Command register (write only, read ignored)
#define COUNTER_2_CONTROLPORT    0x61              // Channel 2 gate: the input can be controlled by IO port 0x61, bit 0.

// command register
// bit0
#define SIXTEEN_BIT_BINARY       0x00
#define FOUR_DIGIT_BCD           0x01
//bit1-3
/*In MODES 0, 1, 4, and 5 the Counter wraps around to the highest count (FFFFh or 9999bcd) and continues counting.
In MODES 2 and 3 (which are periodic) the Counter reloads itself with the initial count and continues counting from there. */
#define MODE0                    0x00              // interrupt on terminal count
#define MODE1                    0x02              // hardware re-triggerable one-shot
#define RATEGENERATOR            0x04              // Mode 2: divide by N counter
#define SQUAREWAVE               0x06              // Mode 3: square-wave mode
#define MODE4                    0x08              // software triggered strobe
#define MODE5                    0x0A              // hardware triggered strobe
//bit4-5
#define RW_LO_MODE               0x10              // Read/2xWrite bits 0..7 of counter value
#define RW_HI_MODE               0x20              // Read/2xWrite bits 8..15 of counter value
#define RW_LO_HI_MODE            0x30              // 2xRead/2xWrite bits 0..7 then 8..15 of counter value
//bit6-7
#define COUNTER_0                0x00              // select counter 0
#define COUNTER_1                0x40              // select counter 1
#define COUNTER_2                0x80              // select counter 2

// counter 2 control port
#define AUX_GATE_2               0x01              // aux port, PIT gate 2 input
#define AUX_OUT_2                0x02              // aux port, PIT clock out 2 enable


#endif
