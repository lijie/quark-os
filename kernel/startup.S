#include "config.h"

#define ALIGN .align 4, 0x90
#define ENTRY(name) \
  .globl name; \
  .align 4, 0x90; \
  name:

	.code32
	.text
ENTRY(startup)
	in $0x92, %al
	or $0x02, %al
	out %al, $0x92
#if 0
	lgdt gdtr
	sgdt gdtr
        ljmp $BOOT_CS_SELECTOR, $1f
#else
	jmp begin_boot
#endif
ENTRY(begin_boot)	
	cld

	/* enable paging */
#if 1
page_pde_offset = (__PAGE_OFFSET >> 20);
	movl $PAGE_TBL, %edi
	movl $PAGE_DIR, %edx
	movl $0x007, %eax			/* 0x007 = PRESENT+RW+USER */
10:
	leal 0x007(%edi),%ecx			/* Create PDE entry */
	/* map 0x00000000 -> 0x00000000 */
	movl %ecx,(%edx)			/* Store identity PDE entry */
	/* map 0xC0000000 -> 0x00000000 */
	movl %ecx,page_pde_offset(%edx)		/* Store kernel PDE entry */
	addl $4,%edx
	movl $1024, %ecx
11:
	stosl
	addl $0x1000,%eax
	loop 11b

	cmpl $0x800000, %eax
	jb 10b
/*	movl %edi,(init_pg_tables_end - __PAGE_OFFSET) */

	movl $PAGE_DIR, %eax
	movl %eax,%cr3		/* set the page table pointer.. */
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0		/* ..and set paging (PG) bit */
	ljmp $8,$1f	/* Clear prefetch and normalize %eip */
1:
	/* Now, the virtul addr comes to true! */
	/* give a stack to our best friend --> C ^_^ */
	movl $(__PAGE_OFFSET + KERNEL_STACK), %esp
	movl $(__PAGE_OFFSET + KERNEL_STACK), %ebp
/*
 * Initialize eflags.  Some BIOS's leave bits like NT set.  This would
 * confuse the debugger if this code is traced.
 * XXX - best to initialize before switching to protected mode.
 */
	pushl $0
	popfl
#endif
#if 0
	movw $0x2401, %ax
	int $0x15
#endif
	cli                 # Clear or disable interrupts
pmmode:
	call _intr_init
	lgdt gdtr
	lidt idt_descr
	movl $16, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	ljmp $8, $1f
1:	
	/* this is the end */

 	call _kernel_start
spin : 
	jmp spin          # Loop

	.globl _load_gdt
_load_gdt:
	lgdt gdtr

setup_idt:
	lea ignore_int,%edx
/* 	movl $(__KERNEL_CS << 16),%eax */
	mov $(8 << 16), %eax
	movw %dx,%ax		/* selector = 0x0010 = cs */
	movw $0x8E00,%dx	/* interrupt gate - dpl=0, present */

	lea _idt_table,%edi
	mov $256,%ecx
rp_sidt:
	movl %eax,(%edi)
	movl %edx,4(%edi)
	addl $8,%edi
	dec %ecx
	jne rp_sidt
	ret

/* This is the default interrupt "handler" :-) */
	ALIGN
ignore_int:
	call _printk
	movb $0x20, %al
	out %al, $0x20
	iret

	.globl _intr_handler
_intr_handler:	
	call _printk
	movb $0x20, %al
	out %al, $0x20
	iret

	.data
gdtr:
	.word (gdt_end - _gdt-1)		# Length of the gdt
	.long _gdt                      	# physical address of gdt

	.global _gdt
_gdt:
	.quad 0x0000000000000000	/* NULL descriptor */
	.quad 0x00cf9a000000fff0	/* kernel 4GB code at 0x00000000 */
	.quad 0x00cf92000000fff0	/* kernel 4GB data at 0x00000000 */
	.quad 0x0000000000000000
	
	.quad 0x0000000000000000
	.quad 0x0000000000000000
	.quad 0x0000000000000000
	.quad 0x0000000000000000
	
	.quad 0x0000000000000000
	.quad 0x0000000000000000
	.quad 0x0000000000000000
	.quad 0x0000000000000000

	.quad 0x0000000000000000
	.quad 0x0000000000000000
	.quad 0x0000000000000000
	.quad 0x0000000000000000
	
	.quad 0x0000000000000000	/* TSS descriptor */
	.quad 0x0000000000000000	/* LDT descriptor */

	.fill 12,8,0			/* space for LDT's and TSS's etc */
gdt_end:
	.global _ldt
_ldt:	
	.quad 0x0000000000000000	/* no use */
	.quad 0x00cffa0000000fff
	.quad 0x00cff20000000fff
	.quad 0x00cffa0000000fff
	.quad 0x00cff20000000fff

idt_descr:
	.word 256*8-1		# idt contains 256 entries
	.long _idt_table
