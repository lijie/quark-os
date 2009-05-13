/*
 * irq_entry.S
 * Lijie, eltshanli@gmail.com
 */

	.text
	.globl timer_interrupt
timer_interrupt:
	push %ds
	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
	movb $0x20, %al
	out %al, $0x20	
	incl jiffies
	popl %eax
	pop %ds
	iret

	.global divide_error
divide_error:
	pushl $do_divide_error
no_error_code:
	xchgl %eax,(%esp)
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl $0		# "error code"
	lea 44(%esp),%edx
	pushl %edx
	movl $0x10,%edx
	mov %dx,%ds
	mov %dx,%es
	mov %dx,%fs
	call *%eax
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

	.global dev_noavailable
dev_noavailable:
	pushl $do_dev_noavailable
	jmp no_error_code

	.global invalid_opcode
invalid_opcode:
	pushl $do_invalid_opcode
	jmp no_error_code

	.global overflow_trap
overflow_trap:
	pushl $do_overflow_trap
	jmp no_error_code

	.global bounds_error
bounds_error:
	pushl $do_bounds_error
	jmp no_error_code

	.global double_fault
double_fault:
	pushl $do_double_fault
error_code:
	xchgl %eax,4(%esp)		# error code <-> %eax
	xchgl %ebx,(%esp)		# &function <-> %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl %eax			# error code
	lea 44(%esp),%eax		# offset
	pushl %eax
	movl $0x10,%eax
	mov %ax,%ds
	mov %ax,%es
	mov %ax,%fs
	call *%ebx
	addl $8,%esp
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

	.global gp_exception
gp_exception:
	pushl $do_gp_exception
	jmp error_code

	.global page_fault
page_fault:
	pushl $do_page_fault
	jmp error_code

	.global stack_fault
stack_fault:
	pushl $do_stack_fault
	jmp error_code

	.global seg_nopresent
seg_nopresent:
	pushl $do_seg_nopresent
	jmp error_code

	.global invalid_tss
invalid_tss:
	pushl $do_invalid_tss
	jmp error_code
