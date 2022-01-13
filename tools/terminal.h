#ifndef __TERMINAL_H_
#define __TERMINAL_H_

void terminal_init();
void terminal_accept_command(unsigned char newline);
void help();
void shutdown();
void restart();
void halt();

#endif