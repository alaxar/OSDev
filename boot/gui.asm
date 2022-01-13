; first change the mode to 13
draw_gui:
    ; change video mode to 13
    mov al, 13
    mov ah, 0
    int 0x10
    mov cx, 0  ; x value
    mov dx, 0  ; y value
    jmp draw_background

draw_background:
    mov ah, 0x0c
    mov al, 0x7
    int 0x10
    add cx, 1
    cmp cx, 64000               ; check if the drawing of the background has finished
    je setup_x_values           ; jmp if finished
    jmp draw_background

setup_x_values
    mov cx, 0
    mov dx, 30                  ; the difference is 30 from 0 30
    draw_x

draw_x:
    mov ah, 0x0c
    mov al, 0x9
    int 0x10
    add cx, 1
    cmp cx, 70*320*2            ; the difference is 30 from 70 to 100
    je finished
    jmp draw_x
    ret

finished:
    ret