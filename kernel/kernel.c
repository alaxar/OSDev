#include "low_level.h"
#include "../drivers/screen.h"
#include <float.h>
#include <cpuid.h>
#include "idt.h"

extern void interrupt_handler();
int main() {
    int image[5][5] = {
        {0, 1, 1, 1, 0},
        {1, 0, 0, 0, 1},
        {1, 1, 1, 1, 1},
        {1, 0, 0, 0, 1},
        {1, 0, 0, 0, 1},
    };
    // clear_screen();
    // printf("Alazar demessie\n", -1, -1, 0xff);
    unsigned char* VGA = (unsigned char*)0xa0000;
    for(int rows = 0; rows < 5; rows++) {
        for(int cols = 0; cols < 5; cols++) {
            if(image[rows][cols] == 1) {
                plot_pixel(rows, cols, 0x44, VGA);
            } else {
                plot_pixel(rows, cols, 0xff, VGA);

            }
        }
    }
    // InitializationIDT(1, 0, interrupt_handler);
    // printf("Kernel", -1, -1);

    while(1);
}