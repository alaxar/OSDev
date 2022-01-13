[bits 32]
[extern main]

call main
%include "boot/interrups.asm"
jmp $