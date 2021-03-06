#include "config.h"
	
#define	START_SEG	CODE_SEG
#define	START_OFFSET	CODE_START
	
	.code16
	.text
	.globl _start
_start:
	movw $(START_SEG), %ax
	movw %ax, %es
#if 0
	movw $0x1000, %ax
	movw %ax, %ds
	
	movw $0xFFFF, %cx
	movw $0xFFFF, %ax
	leaw START_OFFSET, %di
	rep
	stosw
#endif

	movw $0, %bx	/* dest */
	movb $0, %ch		/* track */
b1:
b20:
	movb $0, %dh
	jmp b22
b21:	
	movb $1, %dh		/* head */
b22:
	movb $0, %dl		/* softdisk */
	movb $18, %al		/* number of sector */
	movb $1, %cl		/* offset */
	movb $2, %ah		/* function #2 */
	int $0x13
	addw $(512 * 18), %bx
	cmpw $0, %bx
	jne b3
	movw %es, %ax
	addw $0x1000, %ax
	movw %ax, %es
b3:
	cmpb $1, %dh
	jne b21 
	incb %ch
	cmpb $3, %ch		/* read 512 * 18 * (3 + 1) * 2 bytes */
	jbe b1

	call clrscr
	call get_key
	call check_mem

	cli
	lgdt gdtr

	movl %cr0,%eax          # The lsb of cr0 is the protected mode bit
	orb  $0x01,%al          # Set protected mode bit
	movl %eax,%cr0          # Mov modified word to the control register

	DATA32 ljmp $8, $pm

readsector:

get_key:	
	movb $0x00,%ah         
	int $0x16        # Get_key Fn 00h of 16h,read next character
	ret

	.align 4
clrscr:
	movw $0x0600,%ax # Fn 06 of int 10h,scroll windowup,if al = 0 clrscr
	movw $0x0000,%cx # Clear window from 0,0 
	movw $0x174f,%dx # to 23,79
	movb $0,%bh      # fill with colour 0
	int $0x10        # call bios interrupt 10h
	ret

	.align 4
check_mem:
	/* we use E801 first, if failed, goto 88 */
	stc
	xorw %cx, %cx
	xorw %dx, %dx
	movw $0xe801, %ax
	int $0x15
	jc m88

	cmpw $0, %cx
	jne e801cxdx
	cmpw $0, %dx
	jne e801cxdx
	movw %ax, %cx
	movw %bx, %dx
	
e801cxdx:
	andl $0xffff, %edx  # clear sign extend
	shll $6, %edx       # and go from 64k to 1k chunks
	movl %edx, 0x8000	# store extended memory size
	andl $0xffff, %ecx  # clear sign extend
	addl %ecx, 0x8000 # and add lower memory into
        		    # total size.
	ret
	/* 88 can only detect 64M or less mem */
m88:
	movb $0x88, %ah
	int $0x15
	mov %ax, 0x8000
	ret

	.code32
pm:
	movw	$16, %ax
	movw	%ax, %ds
	movw	%ax, %es
	movw	%ax, %fs
	movw	%ax, %gs
	movw	%ax, %ss
#if 0
	ljmp $START_SEG, $START_OFFSET
#endif
	ljmp $8, $(0x10000 + 0x200)

gdtr:
	.word (gdt_end - gdt-1)		# Length of the gdt
	.long gdt                      	# physical address of gdt
gdt:
	.quad 0x0000000000000000	/* NULL descriptor */
	.quad 0x00cf9a000000ffff	/* kernel 4GB code at 0x00000000 */
	.quad 0x00cf92000000ffff	/* kernel 4GB data at 0x00000000 */
gdt_end:
sector:
	.word 0

	.org 510
	.word 0xAA55
