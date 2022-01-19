get_vesa_bios_info:
    ; set storage memory area for vesa bios info
    mov di, 0x3000          ; ES:DI     ES:0x3000

    ; VESA bios info function
    mov ax, 0x4f00
    int 0x10

    cmp ax, 0x4f
    jne error

    ret

get_vesa_mode_information:
    ; bx is used as parameter here for vesa mode
    mov di, 0x3512

    mov ax, 0x4f01
    mov cx, bx
    int 0x10

    cmp ax, 0x4f
    jne error

    ret

set_vbe_mode:
    mov ax, 0x4f02          ; set vbe mode
    mov bx, 0x4115          ; LFB set, DM set
    int 0x10
    cmp ax, 0x4f
    jne error
    ret

error:
    mov si, VESA_error
    call print_string
    jmp $