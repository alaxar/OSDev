#include "screen.h"
#include "../kernel/low_level.h"
#include "../include/types.h"

void enable_cursor(uint8_t cursor_start, uint8_t cursor_end)
{
    port_byte_out(0x3d4, 0x0a);
    port_byte_out(0x3d5, (port_byte_in(0x3d5) & 0xC0) | cursor_start);

    port_byte_out(0x3d4, 0x0b);
    port_byte_out(0x3d5, (port_byte_in(0x3d5) & 0xE0) | cursor_end);
}

void disable_cursor() {
    port_byte_out(0x3d4, 0x0A);
    port_byte_out(0x3d5, 0x20);
}

void printf(char *string, int col, int row) {
    if(col >= 0 && row >= 0)
        set_cursor(get_screen_offset(col, row));

    for(int i = 0; string[i] != 0; i++) {
        print_char(string[i], col, row, 0);
    }
}

void print_hex(int decimal) {
    int remainder = 0, i = count_digit(decimal); 
    char *hexa_decimal;
    while(decimal != 0) {
        remainder = decimal % 16;
        if(remainder >= 10) {
            hexa_decimal[i] = remainder + 55;
            i--;
        } else {
            hexa_decimal[i] = remainder + 48;
            i--;
        }
        if(decimal == 0) {
            hexa_decimal[i-1] = '\0';
        }
        decimal /= 16;
    }

    printf(hexa_decimal, -1, -1);
}
void print_char(char character, int col, int row, char attribute_byte) {
    unsigned char *vidmem = (unsigned char*)VIDEO_MEMORY;
    if(!attribute_byte)
        attribute_byte = WHITE_ON_BLACK;

    // Get the vidoe memory offset for the screen location
    int offset;
    // if col and row are non-negative, use them for offset
    if(col >= 0 && row >= 0) {
        offset = get_screen_offset(col, row);
    } else {
        offset = get_cursor();
    }

    // if we see a newline character, set offset to the end of current row,
    // so it will be advanced to the first col
    // of the next row

    if(character == '\n') {
        int rows  = offset / (2*MAX_COLS);
        offset = get_screen_offset(79, rows);
        // otherwise write the character
    } else if(character == '\b') {
        // handling backspace
        // 320 * 200 (x, y)
        set_cursor(get_cursor() - 1);
    } else {
        vidmem[offset] = character;
        vidmem[offset+1] = attribute_byte;
    }

    offset+=2;

   offset = handling_scrolling(offset);

   set_cursor(offset);
}


int get_screen_offset(int col, int row) {
    int offset = (row * MAX_COLS + col) * 2;
    return offset;
}

int get_cursor() {
    uint16_t offset = 0;
    port_byte_out(REG_SCREEN_CTRL, 0x0f);
    offset = offset |  port_byte_in(REG_SCREEN_DATA);
    port_byte_out(REG_SCREEN_CTRL, 0x0e);
    offset = offset | ((uint16_t)port_byte_in(REG_SCREEN_DATA)) << 8;
    return offset * 2;
}

void set_cursor(int offset) {
    offset /= 2;        // convert from cell offset to char offset 
                        // this is similar to get_cursor, only we write bytes to those internal device registers
    port_byte_out(REG_SCREEN_CTRL, 15);
    port_byte_out(REG_SCREEN_DATA, (unsigned char)(offset & 0xff));
    port_byte_out(REG_SCREEN_CTRL, 14);    
    port_byte_out(REG_SCREEN_DATA, (unsigned char)((offset >> 8) & 0xff));
}

void clear_screen() {
    int rows = 0;
    int cols = 0;


    for(rows = 0; rows < 80; rows++) {
        for(cols = 0; cols < 25; cols++) {
            print_char(' ', rows, cols, WHITE_ON_BLACK);
        }
    }

    set_cursor(get_screen_offset(0, 0));
}

void plot_pixel(int y, int x, byte color, unsigned char* VGA) {
    // VGA[(y<<8) + (y<<6) + x] = color;
    VGA[y * 800 + x] = color;
}