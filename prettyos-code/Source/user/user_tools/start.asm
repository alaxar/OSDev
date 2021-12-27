[BITS 32]
extern main
extern exitProcess

global _start
global _syscall

_syscall dd syscall_wrapper ; Function pointer. Points to syscall_wrapper per default
oldEsp dd 0                 ; Contains esp in case of sysenter

_start:
    push ecx                ; argv
    push eax                ; argc
    cmp edx, 0              ; Set to 1 if CPU supports sysenter
    je .done
        ; Set function pointer to sysenter, if its supported
        mov DWORD [_syscall], sysenter_wrapper
    .done:
    call main               ; Start programm
    call exitProcess        ; Cleanup. Deletes this task. (Syscall)
    jmp  $

syscall_wrapper:
    int 0x7F                ; Call kernel by issueing an interrupt
    ret

sysenter_wrapper:
    push ebp
    mov [oldEsp], esp       ; Save esp
    mov ebp, .done          ; Write return address to ebp
    sysenter                ; Call kernel by executing sysenter instruction
    .done:
    mov esp, [oldEsp]       ; Restore esp
    pop ebp
    ret
