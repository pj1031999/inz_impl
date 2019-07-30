OUTPUT_FORMAT("elf64-littleaarch64", "elf64-littleaarch64", "elf64-littleaarch64") 
OUTPUT_ARCH(aarch64) 
ENTRY(_start)

PHDRS
{
  init   PT_LOAD FLAGS(5); /* read-only, executable */
  us_text PT_LOAD FLAGS(5);/* read-only, executable */
  text   PT_LOAD FLAGS(5); /* read-only, executable */
  rodata PT_LOAD FLAGS(4); /* read-only */
  data   PT_LOAD FLAGS(6); /* read-write */
}

PROVIDE(_page_table_size = 4096);	/*512 entries 8 bytes each*/
PROVIDE(_stack_size = 32768);
PROVIDE(_kernel = 0xffffFFFF00000000);

SECTIONS
{
  .init 0x80000 : AT(0x80000) ALIGN(4096)
  {
    KEEP(*(.init .init.*))
  } : init

  .us_text : ALIGN(4096)
  {
    _us_text_start = .;
    *(.us_text .us_text.*)
    _us_text_end = .;
  } : us_text
  
  _init_end = ALIGN(4096);


  .text _kernel + _init_end : AT(_init_end) ALIGN(4096)
  {
    *(.evec);
    . = ALIGN(2048);
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
  
  .pagetable : ALIGN(4096)
  {
    _level1_pagetable = .;
    . = . + _page_table_size;	/*entry == 1GB, contain 2 entries*/
    . = ALIGN(4096);

    _level2_pagetable = .;
    . = . + _page_table_size;	/*first  GB, entry == 2MB, contain 512 entries*/
    . = ALIGN(4096);

    . = . + _page_table_size;	/*second GB, entry == 2MB, contain 1 entry*/
    . = ALIGN(4096);
    
    _level3_pagetable = .;
    . = . + _page_table_size;	/*first  GB, entry == 4KB, contain 512 entries (kernel space)*/
    . = ALIGN(4096);
  }

  .stack : ALIGN(_stack_size)
  {
    . = . + _stack_size;	/* stack increases by subtraction */
    _el1_stack = .;		/* core0 kernel space stack */
  }

  .mail_buffer : ALIGN(4096)
  {
    _mail_buffer = .;
    . = . + 1024;    
  }

  .bss : ALIGN(32768)
  {
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
