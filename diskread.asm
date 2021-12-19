[org 0x7c00]
mov [BOOT_DRIVE], dl        ; store the dl in boot drive

; before using the disk it is good to reset it
mov ah, 0x00
int 0x10

; now start reading sectors

mov ah, 0x02

mov dl, [BOOT_DRIVE]
mov ch, 0
mov dh, 0
mov cl, 2
mov al, 5

mov bx, 0x9000

int 0x13

jc disk_error
cmp al, 5
jne disk_error

call print_disk

jmp 0x9000

%include "print_string.asm"

disk_error:
    mov si, E_MSG
    call print_string
    jmp $

print_disk:
    mov si, S_MSG
    call print_string
    ; mov si, bx
    ; call print_string

; data

E_MSG:  db "disk error!", 0
S_MSG:  db "Read success!", 0
BOOT_DRIVE: db 0

times 510-($-$$) db 0
dw 0xaa55

mov ah, 0x0e
mov al, 'K'
int 0x10
mov al, 'E'
int 0x10
mov al, 'R'
int 0x10
mov al, 'N'
int 0x10
mov al, 'E'
int 0x10
mov al, 'L'
int 0x10