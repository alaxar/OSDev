print_string:
    pusha
    mov ah, 0x0e        ; tele-type scrolling 
    ; Loop through the strings located in bx register
    loop:
        lodsb           ; AL = SI
        cmp al, 0       ; Compare if al is equal to 0
        je done         ; Jump to done if equal
        int 0x10        ; otherwise call teletype interrupt
        jmp short loop  ; and loop

done:
    popa
    ret


[bits 32]

VIDEO_MEMORY equ 0xb8000
WHITE_ON_BLACK  equ 0x0f

; prints a null-terminated string pointed to by EDX
print_string_pm:
    pusha
    mov edx, VIDEO_MEMORY

print_string_pm_loop:
    lodsb
    mov ah, WHITE_ON_BLACK

    cmp al, 0
    je done_pm

    mov [edx], ax
    add edx, 2

    jmp print_string_pm_loop

done_pm:
    popa
    ret    