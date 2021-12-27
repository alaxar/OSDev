[map symbols documentation/apm.map]
[bits 16]

org 0x4000

CheckAPM:
    mov ah, 0x53
    xor al, al
    xor bx, bx
    mov ds, bx
    int 0x15
    jc .error
        mov byte [0x3400], 0  ; APM is available
        mov word [0x3401], ax ; Save APM version
        mov word [0x3403], bx ; Should contain "PM"
        mov word [0x3405], cx ; APM flags
        hlt                   ; is translated as exit() at vm86.c
    .error:
        mov byte [0x3400], 1  ; APM is not available due to errors
        mov byte [0x3401], ah ; Error code
        hlt                   ; is translated as exit() at vm86.c

APMInstall:
    xor ax, ax
    mov ds, ax
    call APMDisconnect
    call APMConnect
    call APMVersion
    call APMEnable
    mov byte [0x3400], 0  ; No error
    hlt                   ; is translated as exit() at vm86.c


APMDisconnect:
    mov ah, 0x53
    mov al, 0x04
    int 0x15
    jc .poterror ; Successful, if carry flag is not set
        ret
    .poterror:
        cmp ah, 0x03
        jne .error ; Successful, if error is returned because interface was not connected
            ret
        .error:
            mov byte [0x3400], 1  ; Error occured while disconnecting
            mov byte [0x3401], ah ; Error code
            hlt                   ; is translated as exit() at vm86.c

APMConnect:
    mov ah, 0x53
    mov al, 0x01
    xor bx, bx
    int 0x15
    jc .error
        ret
    .error:
        mov byte [0x3400], 2  ; Error occured while connecting
        mov byte [0x3401], ah ; Error code
        hlt                   ; is translated as exit() at vm86.c

APMVersion:
    ; Indicate that we are an APM 1.2 driver
    mov ah, 0x53
    mov al, 0x0E
    xor bx, bx
    mov ch, 1
    mov cl, 2
    int 0x15
    jc .error
        ret
    .error:
        mov byte [0x3400], 3  ; Error occured while handling out APM version
        mov byte [0x3401], ah ; Error code
        hlt                   ; is translated as exit() at vm86.c

APMEnable:
    mov ah, 0x53
    mov al, 0x08
    mov bx, 1
    mov cx, 1
    int 0x15
    jc .error
        ret
    .error:
        mov byte [0x3400], 4  ; Error occured while enabling
        mov byte [0x3401], ah ; Error code
        hlt                   ; is translated as exit() at vm86.c


SetAPMState:
    xor ax, ax
    mov ds, ax
    mov ah, 0x53
    mov al, 0x07
    mov bx, 1
    mov cx, word [0x3400]
    int 0x15
    jc .error
        mov byte [0x3400], 1
        mov byte [0x3401], ah ; Error code
        hlt                   ; is translated as exit() at vm86.c
    .error:
        mov byte [0x3400], 0
        hlt                   ; is translated as exit() at vm86.c
