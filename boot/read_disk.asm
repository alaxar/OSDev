disk_load:
    ; push all register unto the stack
    pusha
    ; before reading sectors from disk the disk must be reset
    mov ah, 0x00
    int 0x13

    ; Now begin to read sectors
    mov ah, 0x02        ; begin to read sectors
    mov al, 50
    mov ch, 0
    mov dh, 0
    mov cl, 2

    int 0x13

    jc disk_error

    ; check if the read sectors are equal to al value
    cmp al, 50
    jne disk_error

    ; if success
    jmp disk_success


disk_error:
    mov si, DISK_ERROR_MSG
    call print_string
    popa
    ret

disk_success:
    mov si, DISK_SUCC_MSG
    call print_string
    popa
    ret
