ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386)
SECTIONS
{
    . = 0x1400000;
    .text   : { *(.text*)   }
    .data   : { *(.data*)   }
    .rodata : { *(.rodata*) }
    .bss    : { *(.bss*)    }
}
