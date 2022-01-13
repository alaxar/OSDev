#ifndef _SCREEN_HEADER_
#define _SCREEN_HEADER_
#define VIDEO_MEMORY 0xb8000
#define MAX_ROWS 25
#define MAX_COLS 80

#include "../include/types.h"
#define WHITE_ON_BLACK 0x0f
#define REG_SCREEN_CTRL 0x3d4
#define REG_SCREEN_DATA 0x3d5


int get_screen_offset(int, int);
int get_cursor();
void print_char(char character, int col, int row, char attribute_byte);
void clear_screen();
void print_hex(int decimal);
void enable_cursor(uint8_t cursor_start, uint8_t cursor_end);
void disable_cursor();
void plot_pixel(int y, int x, byte color, unsigned char* VGA);

#endif