OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64") 
OUTPUT_ARCH(aarch64) 
ENTRY(_start)

PHDRS
{
  init   PT_LOAD FLAGS(5); /* read-only, executable */
  text   PT_LOAD FLAGS(5); /* read-only, executable */
  rodata PT_LOAD FLAGS(4); /* read-only */
  data   PT_LOAD FLAGS(6); /* read-write */
}

SECTIONS
{
  _kernel = 0xFFFF000000000000;
  _stack_size = 4096;

  .init 0x80000 : AT(0x80000) ALIGN(4096)
  {
    KEEP(*(.init .init.*))
  } : init

  _init_end = ALIGN(4096);

  .text _kernel + _init_end : AT(_init_end) ALIGN(4096)
  {
    *(.evec);
    _text_start = .;
    *(.text .text.*)
    _text_end = .;
  } : text

  .rodata : ALIGN(4096)
  {
    _rodata_start = .;
    *(.rodata .rodata.*)
    _rodata_end = .;
  } : rodata

  .data : ALIGN(4096)
  {
    _data_start = .;
    *(.data .data.*)
    _data_end = .;
  } : data

  .bss : ALIGN(65536)
  {
    _level0_pagetable = .;
    . = . + 65536;
    _level1_pagetable = .;
    . = . + 65536;
    _level2_pagetable = .;
    . = . + 65536;
    _level3_pagetable = .;
    . = . + 65536;

    _el0_stack = .;
    . = . + 65536;
    _el1_stack = .;
    . = . + 65536;
    _el2_stack = .;
    . = . + 65536;
    _el3_stack = .;
    . = . + 65536;
    
    
    _bss_start = .;
    *(.bss .bss.*)
    *(COMMON)
    _bss_end = .;
  }

  . += 32768; /* for physical memory manager bitmap */

  _brk_limit = ALIGN(4096);

  /DISCARD/ :
  {
    *(.ARM.*)
    *(.comment)
  }
}
