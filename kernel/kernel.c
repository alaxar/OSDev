#include "low_level.h"
#include "../drivers/screen.h"
#include <float.h>
#include <cpuid.h>
#include "../include/conversion.h"
#include "../drivers/keyboard.h"
#include "../fonts/fonts_handler.h"
#include "../drivers/vesa_vbe/vbe.h"

extern void loadIDT(void);
unsigned char *vbe_addr = 0x3000;

int main() {
    idt_install();
    isrs_install();
    irq_install();
    clear_screen();
    // keyboard_install();
    // terminal_init();
    load_vbe_data_structures();
    vbe_software_support();
    get_vbe_mode_info();

    unsigned int *lfb = (unsigned int*)&vbe_mode->framebuffer;
	
    int offset = 0;				// draw the first pixel
    lfb[offset] = 0xFF0000;			// red

    while(1);
}
