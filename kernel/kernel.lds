/* #include "config.h" */

/* OUTPUT_FORMAT("elf32-i386", "elf32-i386", "elf32-i386") */
OUTPUT_ARCH(i386)
ENTRY(startup)
SECTIONS {
	 . = 0xC0000000 + 0x100000;
	 _text = .;			/* Text and read-only data */
	 .text : {
	       startup.o(.text)
	 	*(.text)
	 } = 0
	 _etext = .;			/* End of text section */

  . = ALIGN(4096);
  .data : {			/* Data */
	*(.data)
	CONSTRUCTORS
	}
  . = ALIGN(4096);
  .data.page_aligned : { *(.data.idt) }
  __bss_start = .;		/* BSS */
  .bss : {
	*(.bss.page_aligned)
	*(.bss)
  }
  . = ALIGN(4);
  __bss_stop = .; 

  _end = . ;
  . = ALIGN(4096);
  pg = .;
}