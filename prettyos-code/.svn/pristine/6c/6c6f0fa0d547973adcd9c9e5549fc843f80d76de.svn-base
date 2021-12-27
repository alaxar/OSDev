OUTPUT_FORMAT(binary)
OUTPUT_ARCH(i386)
STARTUP(object_files/kernel/kernel.o)
SECTIONS
{
    . = 0x00100000;
    _kernel_beg = .;
    .text   : { _ro_start = .;   *(.text*)                 KEEP(*(.text))   }
    .rodata : {                  *(.rodata*) _ro_end = .;  KEEP(*(.rodata)) }
    .data   : {                  *(.data*)                 KEEP(*(.data))   }
    .bss    : { _bss_start = .;  *(.bss*);   _bss_end = .; KEEP(*(.bss))    }
    .cdi    : { _cdi_start = .;  *(.cdi*);   _cdi_end = .; KEEP(*(.cdi))    }
    /DISCARD/ : { *(.eh_frame*); }
    _kernel_end = .;
}