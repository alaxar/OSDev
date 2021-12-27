[org 0x7c00]

mov bp, 0x9000
mov sp, bp

mov si, msg_real_mode
call print_string

call load_kernel

mov ah, 0x00
mov al, 0x13
int 0x10

call switch_to_pm

jmp $

; Data
BOOT_DRIVE: db 0
WELCOME: db "Welcome...Loading the kernel....", 0
DISK_ERROR_MSG: db "Disk read Error!", 0
DISK_SUCC_MSG: db "Disk read success!", 0
msg_real_mode db "Started in 16-bit real mode", 0
msg_prot_mode db "Successfully landed in 32 bit protected mode", 0
KERNEL_OFFSET EQU 0x1000
%include "boot/read_disk.asm"
%include "boot/print_string.asm"
%include "boot/gdt.asm"
%include "boot/switch_to_pm.asm"

[bits 16]
load_kernel:
	mov si, WELCOME
	call print_string

	mov [BOOT_DRIVE], dl
	mov bx, KERNEL_OFFSET
	mov dl, [BOOT_DRIVE]
	call disk_load
	ret

[bits 32]
BEGIN_PM:
    mov esi, msg_prot_mode
    call print_string_pm

    jmp KERNEL_OFFSET
    jmp $

times 510-($-$$) db 0
dw 0xaa55
