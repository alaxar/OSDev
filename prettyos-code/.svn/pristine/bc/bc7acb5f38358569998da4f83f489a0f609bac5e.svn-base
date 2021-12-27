%ifndef A20_INC
%define A20_INC

;******************************************************************************
;   Enable A20 address line
;******************************************************************************

[BITS 16]

EnableA20:
    in  al, 0x92      ; switch A20 gate via fast A20 port 92
    cmp al, 0xff      ; if 0xFF, nothing's implemented on this port
    je .no_fast_A20

    or  al, 2         ; set A20_Gate_Bit (bit 1)
    and al, ~1        ; clear INIT_NOW bit (don't reset pc...)
    out 0x92, al
    ret

.no_fast_A20:         ; no fast shortcut -> use the slow kbc...
    call empty_8042

    mov al, 0xD1      ; kbc command: write to output port
    out 0x64, al
    call empty_8042

    mov al, 0xDF      ; writing this to kbc output port enables A20
    out 0x60, al
    call empty_8042
    ret

;******************************************************************************
;   calls
;******************************************************************************

empty_8042:
    call Waitingloop
    in al, 0x64
    cmp al, 0xff      ; ... no real kbc at all?
    je .done
    test al, 1        ; something in input buffer?
    jz .no_output
    call Waitingloop
    in al, 0x60       ; yes: read buffer
    jmp empty_8042    ; and try again
.no_output:
    test al, 2        ; command buffer empty?
    jnz empty_8042    ; no: we can't send anything new till it's empty
.done:
    ret

Waitingloop:
    mov bx,0xFFFF
.loop_start:
    dec bx
    jnz .loop_start
    ret

%endif
