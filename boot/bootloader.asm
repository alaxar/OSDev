[org 0x7c00]


jmp short start

; BIOS Parameter Block
; FAT12 header

oem_identifier				db "MSWIN4.1"
bytes_per_sector			dw 512
sectors_per_cluster			db 1
no_of_reserved_sectors		dw 1
no_of_file_alloc_tbl		db 2
no_of_dirs_entries			dw 0xe0
total_sectors				dw 2880
media_descriptor_type		db 0x0f0
no_of_sectors_per_fat		dw 9
no_of_sectors_per_track		dw 18
no_of_heads_or_slides		dw 2
no_of_hidden_sectors		dd 0
no_of_large_sectors_count	dd 0

; Extended boot record
drive_no					db 0x00
win_nt_reserved				db 0x00
signiture					db 0x29
volume_id					db 12h, 34h, 56h, 78h
volume_label_string			db "ETHIOPIC OS"
system_identifier_string	db "        "


start:
	mov bp, 0x9000
	mov sp, bp

	mov si, msg_real_mode
	call print_string

	call load_kernel
	; call draw_gui


	call switch_to_pm

jmp $

; Data
BOOT_DRIVE: db 0
KERNEL_OFFSET EQU 0x1000
WELCOME: db "Welcome...Loading the kernel....", 0
DISK_ERROR_MSG: db "Disk read Error!", 0
DISK_SUCC_MSG: db "Disk read success!", 0
msg_real_mode db "Started in 16-bit real mode", 0
msg_prot_mode db "Successfully landed in 32 bit protected mode", 0
%include "boot/gui.asm"
%include "boot/read_disk.asm"
%include "boot/print_string.asm"
%include "boot/gdt.asm"
%include "boot/switch_to_pm.asm"

[bits 16]
load_kernel:
	mov si, WELCOME
	call print_string

	mov [BOOT_DRIVE], dl
	mov bx, KERNEL_OFFSET
	mov dl, [BOOT_DRIVE]
	call disk_load
	ret

[bits 32]
BEGIN_PM:
    mov esi, msg_prot_mode
    call print_string_pm

    jmp KERNEL_OFFSET
    jmp $

times 510-($-$$) db 0
dw 0xaa55
