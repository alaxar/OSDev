; The CPU needs a table that contains information about interrupts that may
;  occur. Each entry is 8 bytes in size and holds the interrupt-routine's
;  address as well as some flags related to that interrupt. The default flags
;  are "0x0008, 0x8E00". If user code shall be allowed to trigger a specific
;  interrupt, set the second flag word to 0xEE00 instead of 0x8E00.
; The interrupts numbered 0-32 are called "Interrupt Service Routines (ISRs)"
;  and are triggered automatically by the CPU when something bad happens, e.g.
;  at division by zero, occurance of broken instruction code or access to a
;  disallowed memory area.
; Interrupts 32-256 are called "Interrupt ReQuests (IRQs)", some of them
;  (no 32-48) are again triggered externally e.g. by the timer chip or the
;  keyboard and the rest can be fired internally using an "int 0xXX" assembler
;  instruction.
;
; This file contains:
;  - Code for each interrupt routine we are using. The code is highly redundant
;    so we use NASM's macro facility to create the routines. Each routine
;    possibly pushes an dummy error code, pushes it's interrupt number and
;    jumps to a "common" routine which proceeds. Because all our IDT entries
;    are set as "Interrupt gates", hardware interrupts are automatically
;    disabled.
;
;  - The common routine which is invoked by each interrupt routine. It saves
;    the registers and jumps to some C-code which finally handles the
;    interrupt.
;
;  - The initialization code:
;      * We fill the interrupt descriptor table with valid entries.
;      * The IRQs initially collide with the ISRs (both use 0-15) so we have to
;        "remap" the IRQs from 0-15 to 32-48.
;      * Perform the actual "load" operation.

global idt_install
global pic_remap
global pic_disable
global ir_common_stub
extern irq_handler

%define CONTEXT_SWITCH_CALL 126
%define SYSCALL_NUMBER 127



section .text


%assign NUM_IR 256

; Template for a single interrupt routine:
%macro IR_ROUTINE 1
    ir%1:
        %if (%1!=8) && (%1!=17) && (%1!=30) && (%1<10 || %1>14)
        push dword 0         ; Dummy error code needs to be pushed on some interrupts
        %endif
        push dword %1
        %if (%1 != NUM_IR-1) ; Last one does not need a jump, since it is directly followed by the stub
        jmp ir_common_stub
        %endif
%endmacro

; Create the interrupt-routines
%assign routine_nr 0
%rep NUM_IR
    IR_ROUTINE routine_nr
    %assign routine_nr routine_nr+1
%endrep

; Called from each interrupt routine, saves registers and jumps to C-code
ir_common_stub:
    push eax
    push ecx
    push edx
    push ebx
    push ebp
    push esi
    push edi

    push ds
    push es
    push fs
    push gs

    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    push esp          ; parameter of irq_handler
    call irq_handler
    mov esp, eax      ; return value: changed or unchanged esp

    pop gs
    pop fs
    pop es
    pop ds

    pop edi
    pop esi
    pop ebp
    pop ebx
    pop edx
    pop ecx
    pop eax

    add esp, 8
    iret


; Setup and load the IDT
idt_install:
    mov ecx, NUM_IR
    mov eax, idt_table + (NUM_IR-1)*8
    .loop:
        mov dx, [eax + 2]
        xchg dx, [eax + 6]   ; Swap words 1 and 3 of each IDT entry (see below)
        mov [eax + 2], dx
        sub eax, 8
        loop .loop

    ; Perform the actual load operation
    lidt [idt_descriptor]
    ret


pic_remap:
    ; Remap IRQ 0-15 to 32-47 (see http://wiki.osdev.org/PIC#Initialisation)
    ; Interrupt Vectors 0x20 for IRQ 0 to 7 and 0x28 for IRQ 8 to 15
    mov al, 0x11        ; INIT command
    out 0x20, al        ; send INIT to PIC1
    out 0xA0, al        ; send INIT to PIC2

    mov al, 0x20        ; PIC1 interrupts start at 0x20
    out 0x21, al        ; send the port to PIC1 DATA
    mov al, 0x28        ; PIC2 interrupts start at 0x28
    out 0xA1, al        ; send the port to PIC2 DATA

    mov al, 0x04        ; MASTER code
    out 0x21, al        ; set PIC1 as MASTER
    mov al, 0x02        ; SLAVE code
    out 0xA1, al        ; set PIC2 as SLAVE

    dec al              ; al is now 1. This is the x86 mode code for both 8259 PIC chips
    out 0x21, al        ; set PIC1
    out 0xA1, al        ; set PIC2

    dec al              ; al is now 0. This tells the PIC to unmask all interrupts
    out 0x21, al        ; set PIC1
    out 0xA1, al        ; set PIC2

    ret

pic_disable:            ; for the use of I/O-APIC
    ; Mask all interrupts in PIC
    mov al, 0xFF
    out 0xA1, al
    out 0x21, al
    ; Enable symmetric I/O Mode as per MPS 1.4
    mov al, 0x70        ; select the IMCR register
    out 0x22, al
    mov al, 0x01        ; instruct NMIs and 8259 INTs to go through APIC
    out 0x23, al
    ret  


section .data

; Interrupt Descriptor Table
; Holds NUM_IR entries each 8 bytes of size
idt_table:
    %macro CREATE_IDT_ENTRY 3
        dd ir%1 ; Due to restrictions of the linker, we have to swap words 1 and 3 in idt_install
        dw %3
        dw %2
    %endmacro

    ; Execute the macro to fill the interrupt table, unfilled entries remain zero.
    %assign COUNTER 0
    %rep NUM_IR
        %if (COUNTER == SYSCALL_NUMBER) || (COUNTER == CONTEXT_SWITCH_CALL)
            CREATE_IDT_ENTRY COUNTER, 0x0008, 0xEE00
        %else
            CREATE_IDT_ENTRY COUNTER, 0x0008, 0x8E00
        %endif
        %assign COUNTER COUNTER+1
    %endrep

; IDT Descriptor holds the IDT's size minus 1 and it's address
idt_descriptor:
    dw NUM_IR*8-1
    dd idt_table
