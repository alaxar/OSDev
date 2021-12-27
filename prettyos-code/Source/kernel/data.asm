;---------------------- shell --------------------------------------------------
global shell_start
global shell_end
shell_start:
    incbin "user/shell/SHELL.ELF"
shell_end:
;---------------------- shell --------------------------------------------------

; --------------------- VM86 ---------------------------------------------------
global vidswtch_com_start
global vidswtch_com_end
global apm_com_start
global apm_com_end
vidswtch_com_start:
    incbin "user/vm86/VIDSWTCH.COM"
vidswtch_com_end:
apm_com_start:
    incbin "user/vm86/APM.COM"
apm_com_end:
; --------------------- VM86 ---------------------------------------------------

;---------------------- bmp cursor ---------------------------------------------
global cursor_start
global cursor_end
cursor_start:
    incbin "user/vm86/cursor.bmp"
cursor_end:
;---------------------- bmp cursor ---------------------------------------------
