isr1:
    pusha
    push gs
    push fs
    push ds
    push es

    ; call your function
    [extern interrupt_handler]
    call interrupt_handler

    pop es
    pop ds
    pop fs
    pop gs
    popa

    iret
    GLOBAL isr1