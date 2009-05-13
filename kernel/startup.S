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
	/* disable NMI */
	movb $0x80, %al
	outb %al, $0x70
	nop

	/* enable A20 */
	in $0x92, %al
	or $0x02, %al
	out %al, $0x92

	/* mask all INTs */
	movb $0xFF, %al
	outb %al, $0xA1
	nop
	movb $0xFB, %al
	outb %al, $0x21
	nop

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
	leal pg, %edi
	subl $__PAGE_OFFSET, %edi
	addl $PAGE_SIZE, %edi
	leal pg, %edx
	subl $__PAGE_OFFSET, %edx

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

	movl %edx, %eax
	movl %eax,%cr3		/* set the page table pointer.. */
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0		/* ..and set paging (PG) bit */
	ljmp $8,$1f	/* Clear prefetch and normalize %eip */
1:
	/* Now, the virtul addr comes to true! */
	/* give a stack to our best friend --> C ^_^ */
#if 0
	movl $(__PAGE_OFFSET + KERNEL_STACK), %esp
	movl $(__PAGE_OFFSET + KERNEL_STACK), %ebp
#endif
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
	lss start_stack, %esp
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
	pushl %eax
	pushl $8
	pushl $4
	pushl %esp
	call mem_dump

	movl $0, 24(%esp)

	movl 24(%esp), %eax
	movl %eax, testv1
	call show_v1v2

	popl %eax
	popl %eax
	popl %eax
	popl %eax

	iret

	.data

/*
 * We use init_task's stack as default kernel stack
 */
ENTRY(start_stack)
	.long init_thread + THREAD_SIZE - 4
	.long 0x10

	.align 4096
	.global init_thread
init_thread:
	.fill THREAD_SIZE / 8, 8, 0

	.align 4096
	.global idt
idt:
	.fill 256, 8, 0
gdtr:
	.word (gdt_end - gdt-1)		# Length of the gdt
	.long gdt                      	# physical address of gdt

	.align 8
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

	.align 4
int_msg:
	.asciz "Unknown interrupt or fault at: %p %p %p %p\n"

	.global mem_size_kbytes
	.bss
	.align 4
	.type	mem_size_kbytes, @object
	.size	mem_size_kbytes, 4
mem_size_kbytes:
	.zero 4

	.global testv1
	.bss
	.align 4
	.type	testv1, @object
	.size	testv1, 4
testv1:
	.zero 4

	.global testv2
	.bss
	.align 4
	.type	testv2, @object
	.size	testv2, 4
testv2:
	.zero 4

