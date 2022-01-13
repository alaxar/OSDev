[bits 16]

; Switch to protected mode
switch_to_pm:
    cli             ; this will clear interrupts

    lgdt[gdt_descriptor]        ; load our global descirptor table, which defines the protecte mode segments

    ; setting the bit in cr0 makes protected mode
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax

    jmp CODE_SEG:init_pm    ; far jump


[bits 32]

; Initialise registers and the stack once in PM
init_pm:
    mov ax, DATA_SEG
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov ebp, 0x90000
    mov esp, ebp

    call BEGIN_PM

; DATA

INIT_PM: db "Initializing Protected mode...", 0