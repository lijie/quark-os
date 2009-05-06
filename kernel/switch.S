#include "config.h"
#if 0
	
	.text
	.align 4
	.globl _load_tss
_load_tss:
	xorl %eax, %eax
	movw $TSS_SELECTOR, %ax
	ltr %ax

	.align 4
	.globl _load_ldt
_load_ldt:
	xorl %eax, %eax
	movw $LDT_SELECTOR, %ax
# 	lldt %ax
#endif
	.globl _test_task
_test_task:
	movl $0x00, %eax

	.globl die
die:
1:	jb 1