[bits 32]
[extern main]

call main
%include "boot/interrupts.asm"

jmp $