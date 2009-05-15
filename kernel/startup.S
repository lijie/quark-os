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
	/* get current real address, saved in %esi */
	call getr
getr:
	popl %esi
	leal getr, %eax
	subl $startup, %eax
	subl %eax, %esi

	leal startup, %ebx
	subl $__PAGE_OFFSET, %ebx
	subl %esi, %ebx

	movl $BOOT_DS, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs
	movw %ax, %ss

	/* we want use C ASAP, so give me a stack... */
	movl $(mini_stack - __PAGE_OFFSET), %esp
	subl %ebx, %esp

	cli                 # Clear or disable interrupts
	pushl %esi

	/* disable INTs, enable pageing here */
	call boot_main
	popl %esi

	/* relocate EIP */
	jmp 1f
1:
	movl $1f, %eax
	jmp *%eax
1:
	/* load ourselfs gdt */
	lgdt gdtr
	ljmp $(BOOT_CS), $1f
1:
	movl $BOOT_DS, %eax
	movw %ax, %ds
	movw %ax, %es
	movw %ax, %fs
	movw %ax, %gs

	lss start_stack, %esp

	/* clear eflags */
	pushl $0
	popfl

	/* save memory size */
	movl 0x8000, %eax
	movl %eax, mem_size_kbytes

 	call kernel_start
spin : 
	jmp spin          # Loop

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

	.align 4
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
	.global testv1
testv1:	
	.long 0
	.global testv2
testv2:	
	.long 0

	.align 4
	.fill 64, 8, 0
mini_stack:

	.global mem_size_kbytes
	.bss
	.align 4
	.type	mem_size_kbytes, @object
	.size	mem_size_kbytes, 4
mem_size_kbytes:
	.zero 4

