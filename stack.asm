mov ah, 0x0e

mov bp, 0x8000
mov sp, bp

push 'A'
push 'B'
push 'C'

pop bx          ; 0x0043
mov al, bl
int 0x10


times 510-($-$$) db 0

dw 0xaa55