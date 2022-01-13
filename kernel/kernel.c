#include "low_level.h"
#include "../drivers/screen.h"
#include <float.h>
#include <cpuid.h>
#include "../include/conversion.h"
#include "../drivers/keyboard.h"

extern void loadIDT(void);
// extern unsigned char address_of_initrd;
int main() {
    idt_install();
    isrs_install();
    irq_install();
    clear_screen();
    keyboard_install();
    terminal_init();
    // printf(&address_of_initrd+8, -1, -1);
    // int x = string_compare("alazar", "alazar");
    // printf(itoa(x), -1, -1);
    // if(x) {
    //     printf("matched", -1, -1);
    // } else {
    //     printf("not matched", -1, -1);
    // }
   __asm__("sti");
    while(1);
}