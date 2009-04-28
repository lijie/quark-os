	.file	"irq.c"
	.text
	.p2align 4,,15
.globl _intr_init
	.def	_intr_init;	.scl	2;	.type	32;	.endef
_intr_init:
	pushl	%ebp
	movl	$_intr_handler, %eax
	movl	%esp, %ebp
	pushl	%ebx
	xorl	%ebx, %ebx
	movzwl	%bx,%edx
	movzwl	%ax, %ecx
	movl	%edx, %ebx
	andl	$-65536, %eax
	orl	%eax, %ebx
	andl	$-256, %ebx
	movzwl	%cx,%eax
	orl	$32768, %ebx
	movl	%eax, %ecx
	andl	$-24577, %ebx
	movl	%ebx, %eax
	orl	$524288, %ecx
	andl	$-1793, %eax
	movl	%eax, %ebx
	orl	$1536, %ebx
	xorl	%eax, %eax
	andl	$-4097, %ebx
	orl	$2048, %ebx
	.p2align 4,,15
L5:
	movl	%ecx, _idt_table(,%eax,8)
	movl	%ebx, _idt_table+4(,%eax,8)
	incl	%eax
	cmpl	$255, %eax
	jbe	L5
	movb	$32, %cl
	movb	$17, %al
/APP
	outb %al, $32
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	$-96, %dl
/APP
	outb %al, %dx
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	%cl, %al
/APP
	outb %al, $33
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	$-95, %dl
	movb	$40, %al
/APP
	outb %al, %dx
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	$4, %al
/APP
	outb %al, $33
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	$2, %al
/APP
	outb %al, %dx
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	$1, %al
/APP
	outb %al, $33
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
	outb %al, %dx
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	$-1, %al
/APP
	outb %al, $33
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
	outb %al, %dx
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	popl	%ebx
	popl	%ebp
	ret
	.section .rdata,"dr"
LC0:
	.ascii "I'm in trap_debug\12\0"
	.text
	.p2align 4,,15
	.def	_trap_debug;	.scl	3;	.type	32;	.endef
_trap_debug:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$LC0, (%esp)
	call	_printf
	leave
	ret
	.p2align 4,,15
.globl _trap_init
	.def	_trap_init;	.scl	2;	.type	32;	.endef
_trap_init:
	pushl	%ebp
	movl	$_trap_debug, %edx
	movzwl	%dx,%eax
	andl	$-65536, %edx
	orl	$524288, %eax
	orl	$36608, %edx
	movl	%eax, _idt_table
	movl	%esp, %ebp
	movl	%edx, _idt_table+4
/APP
	idivl $0
/NO_APP
	popl	%ebp
	ret
	.p2align 4,,15
.globl _irq_enable
	.def	_irq_enable;	.scl	2;	.type	32;	.endef
_irq_enable:
	pushl	%ebp
	movb	$-2, %al
	movl	%esp, %ebp
	movl	8(%ebp), %ecx
/APP
	outb %al, $33
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	movb	$-95, %dl
	movb	$-1, %al
/APP
	outb %al, %dx
	jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:jmp 1f; 1: jmp 1f; 1:
/NO_APP
	popl	%ebp
	movl	$_clock_handler, %eax
	movw	%ax, _idt_table(,%ecx,8)
	sarl	$16, %eax
	movw	%ax, _idt_table+6(,%ecx,8)
	ret
	.comm	_idt_table, 2048	 # 2048
	.def	_clock_handler;	.scl	3;	.type	32;	.endef
	.def	_intr_handler;	.scl	3;	.type	32;	.endef
	.def	_printf;	.scl	3;	.type	32;	.endef
