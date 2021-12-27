#include "idt.h"
#include "../drivers/screen.h"
extern uint32_t isr1;
struct InterruptDescriptorTable _idt[256];
struct IDTR idtr;

void loadIDT() {
    idtr.limit = 256*(sizeof(struct InterruptDescriptorTable)) - 1;
    idtr.base = _idt;

    struct IDTR *IDTRPtr = &idtr;

    __asm__("LIDT (%0) ": : "p" (IDTRPtr));
    __asm__("sti");
}

void InitializationIDT(int number, int dpl, uint32_t handler) {
    uint16_t selector;
    uint16_t mode_setting;

    // get the Code Segment Selector
    __asm__("mov %%cs, %0" : "=g"(selector));

    switch (dpl)
    {
    case 0:
        mode_setting = KERNEL_MODE;
        break;
    case 3:
        mode_setting = USER_MODE;
        break;
    default:
        break;
    }
    _idt[number].offset_1 = (handler & 0xffff);
    _idt[number].selector = selector;
    _idt[number].type_attributes = mode_setting;
    _idt[number].offset_2 = (handler >> 16);

    // reprogram the PIC
    port_byte_out(0x21, 0xfd);
    port_byte_out(0xa1, 0xff);
    loadIDT();
}

void interrupt_handler() {
    print_char('F', -1, -1, 0);
    port_byte_out(0x20, 0x20);
    port_byte_out(0xa0, 0x20);
    while(1);
}