[org 0x7c00]

mov bp, 0x9000
mov sp, bp

mov si, msg_real_mode
call print_string

call switch_to_pm

jmp $

; Data
BOOT_DRIVE: db 0
WELCOME: db "Welcome...Loading the kernel....", 0
DISK_ERROR_MSG: db "Disk read Error!", 0
DISK_SUCC_MSG: db "Disk read success!", 0
msg_real_mode db "Started in 16-bit real mode", 0
msg_prot_mode db "Successfully landed in 32 bit protected mode", 0

%include "read_disk.asm"
%include "print_string.asm"
%include "gdt.asm"
%include "switch_to_pm.asm"

BEGIN_PM:
    mov esi, msg_prot_mode
    call print_string_pm

    int 0x10
    jmp $

times 510-($-$$) db 0
dw 0xaa55