#include "timer.h"

int timer_tick = 0;
void timer_handler() {
    timer_tick++;

    if(timer_tick % 60 == 0) {
        printf(itoa(timer_tick), -1, -1);
        printf("\n", -1, -1);
    }
}

void timer_install() {
    irq_install_handler(0, timer_handler);
}