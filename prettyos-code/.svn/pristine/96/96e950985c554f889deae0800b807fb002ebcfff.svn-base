[map symbols documentation/vidswtch.map]
[bits 16]

org 0x4000

SetDisplayStart:
    mov ax, 0x4F07
    xor bx, bx           ; Set bh to 0 and use it to set ds to 0, too.
    mov ds, bx
    mov bl, 0x00
    mov dx, word[0x3600] ; Set first Displayed Scan Line
    mov cx, word[0x3602] ; Set first Displayed Pixel in Scan Line
    int 0x10
    hlt          ; is translated as exit() at vm86.c

Enable8bitPalette:
    mov ax, 0x4F08
    xor bl, bl
    mov bh, 8
    int 0x10
    hlt          ; is translated as exit() at vm86.c

SwitchToTextMode:
    mov ax, 0x0002
    int 0x10
    mov ax, 0x1112
    xor bl, bl
    int 0x10
    hlt          ; is translated as exit() at vm86.c

SwitchToVideoMode:
    xor ax, ax
    mov ds, ax
    mov ax, 0x4F02
    mov bx, word [0x3600] ; video mode
    int 0x10
    hlt          ; is translated as exit() at vm86.c

GetVgaInfoBlock:
    xor ax, ax
    mov es, ax
    mov di, 0x3400
    mov ax, 0x4F00
    int 0x10
    hlt          ; is translated as exit() at vm86.c

GetModeInfoBlock:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov di, 0x3600
    mov ax, 0x4F01
    mov cx, word [0x3600] ; video mode
    int 0x10
    hlt          ; is translated as exit() at vm86.c
