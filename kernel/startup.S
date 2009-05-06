#include "config.h"

#define ALIGN .align 4, 0x90
#define ENTRY(name) \
  .globl name; \
  .align 4, 0x90; \
  name:

#define	ENDLESS \
__endless:	jmp __endless

#define	LATCH	11930

	.code32
	.text
ENTRY(startup)
	in $0x92, %al
	or $0x02, %al
	out %al, $0x92

	/* copy kernel code from 0x10000 -> _text */
	leal _text, %edi
	subl $__PAGE_OFFSET, %edi
	leal _end, %ecx
	subl $__PAGE_OFFSET, %ecx
	subl %edi, %ecx
	movl $0x00010000, %esi
	rep
	movsb

	/* save memory size */
	movl 0x8000, %ebx
	movl %ebx, mem_size_kbytes - __PAGE_OFFSET

	jmp begin_boot

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

	/* map 8MB size only, it should be 896MB like linux */
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

	cli                 # Clear or disable interrupts
pmmode:
	lgdt gdtr
	movl $16, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss
	ljmp $8, $1f
1:
 	call kernel_start
spin : 
	jmp spin          # Loop

	.globl _load_gdt
_load_gdt:
	lgdt gdtr

/* This is the default interrupt "handler" :-) */
	ALIGN
	.global ignore_int
ignore_int:
	push %ds
	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
	pushl $0x42
	call putchar
	popl %eax
	popl %eax
	pop %ds
	iret

	.data
	.global idt
idt:
	.fill 256, 8, 0
gdtr:
	.word (gdt_end - gdt-1)		# Length of the gdt
	.long gdt                      	# physical address of gdt

	.global gdt
gdt:
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
	.global ldt
ldt:	
	.quad 0x0000000000000000	/* no use */
	.quad 0x00cffa000000fff0	/* code seg */
	.quad 0x00cff2000000fff0	/* data seg */
	.quad 0x00c0fa00000003ff
	.quad 0x00c0f200000003ff

	.global mem_size_kbytes
	.bss
	.align 4
	.type	mem_size, @object
	.size	mem_size, 4
mem_size_kbytes:
	.zero 4

