#include "terminal.h"
#include "../include/types.h"

unsigned char *command;
int i = 0;

void terminal_cursor() {
    printf("[Terminal]> ", -1, -1);
}

void terminal_init() {
    printf("\n", -1, -1);
    printf("                [ ETHIOPIC 32 BIT OPERATING SYSTEM ]             ", -1, -1);
    printf("                                         [ TERMINAL MODE ]                        ", -1, -1);
    printf("\n", -1, -1);
    printf("\n", -1, -1);
    printf("\n", -1, -1);
    terminal_cursor();
}

void terminal_accept_command(unsigned char newline) {
    command[i] = newline;
    if(newline != '\b')
        print_char(command[i], -1, -1, 0);
    else
        i = i - 2;          // if the backspace is pressed decrement the value of i

    i++;
    if(newline == '\n') {
        command[i-1] = '\0';
        // compare the strings
        if(string_compare(command, "help") > 0) {
            help();
        } else if(string_compare(command, "clear") > 0) {
            clear_screen();
        } else if(string_compare(command, "shutdown") > 0) {
            shutdown();
        } else if(string_compare(command, "restart") > 0) {
            restart();
        } else if(string_compare(command, "halt") > 0) {
            halt();
        } else {
            printf("Command not found\n", -1, -1);
        }
        i = 0;                     
        terminal_cursor();
    }
}

void help() {
    printf("\n", -1, -1);
    printf("help - to show this message\n", -1, -1);
    printf("clear - clear the screen\n", -1, -1);
    printf("shutdown - to shutdown the computer\n", -1, -1);
    printf("restart - restart the computer\n", -1, -1);
    printf("halt - to halt this computer\n", -1, -1);
    printf("\n", -1, -1);
}

void shutdown() {
	__asm__ __volatile__ ("outw %1, %0" : : "dN" ((uint16_t)0xB004), "a" ((uint16_t)0x2000));
}

void restart() {
    port_byte_out(0x64, 0xfe);
}

void halt() {
    __asm__("hlt");
}