OUTPUT_FORMAT("binary")
ENTRY(start)
SECTIONS
{
  .text  0x1000 : {
    *(.text)
  }
  .data  : {
    *(.data)
  }
  .bss  :
  { 					
    *(.bss)
  }
}