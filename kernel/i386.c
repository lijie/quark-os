#include "config.h"
#include "i386.h"
#include "string.h"
#include "task.h"

#if 0
void arch_setup_task(task_t *task, void (*func)(void *))
{
	task->ldt_sel = LDT_SELECTOR;
	memcpy(&task->ldt0, &gdt[KERNEL_CODE], sizeof(gdt[0]));
	memcpy(&task->ldt1, &gdt[KERNEL_DATA], sizeof(gdt[0]));

	task->ldt0.attr1 = DA_C /* | PRIVILEGE_TASK << 5 */;
	task->ldt0.attr1 = DA_DRW /* | PRIVILEGE_TASK << 5 */;

	task->stack_frame.cs = 0 | SA_TIL /*| RPL_TASK*/;
	task->stack_frame.es = 8 | SA_TIL /*| RPL_TASK*/;
	task->stack_frame.ds = 8 | SA_TIL /*| RPL_TASK*/;
	task->stack_frame.ss = 8 | SA_TIL /*| RPL_TASK*/;
	task->stack_frame.fs = 8 | SA_TIL /*| RPL_TASK*/;
	task->stack_frame.gs = 8 | SA_TIL /*| RPL_TASK*/;

	task->stack_frame.esp = (uint32_t)task->stack_base + STACK_SIZE;
	task->stack_frame.eflags = 0x1202;

	task->stack_frame.eip = (uint32_t)func;

	/* tss */
	memset(&task->tss, 0, sizeof(task->tss));
	task->tss.ss0 = KERNEL_DATA_SELECTOR;

	init_desc(&gdt[TSS], (uint32_t)&task->tss, sizeof(task->tss) - 1, DA_386TSS);
	init_desc(&gdt[LDT], (uint32_t)&task->ldt0, sizeof(task->ldt0) * 2 - 1, DA_LDT);
	return;
}
#endif

void __switch_soft(task_t *task)
{
	__asm__ __volatile__ (
		"movl %0, 4(%%esp)"
		:
		: "r"(task->stack_frame.eip)
		);
}

void switch_to(task_t *task)
{
	__switch_soft(task);
#if 0
	uint32_t addr;

	load_ldt();
	load_tss();
	puts("load TSS ok\n");
	task->tss.esp0 = (uint32_t)&task->stack_frame + sizeof(task->stack_frame);

	asm __volatile__ (
		"movl %1, %%eax\n\t" 
		"movl %%eax, %0\n\t" 
		: "=r"(addr)
		: "r"(&task->stack_frame));

	printf("stack frame addr %X\n", addr);
#endif
#if 0
	asm __volatile__ ("movl %0, %%esp" : : "r"(&task->stack_frame));
	asm __volatile__ (
		"popl %%gs\n\t"
		"movl %%gs, %0"
		: "=r"(addr)
		:
		);
	printf("gs addr %X\n", addr);
#endif
#if 1
	asm __volatile__ ("movl %0, %%esp" : : "r"(&task->stack_frame));
	asm __volatile__ (
		"popl %gs\n\t"
		"popl %fs\n\t"
#if 1
		"popl %es\n\t"
		"popl %ds\n\t"
		"popal \n\t"
		"addl 4, %esp\n\t"
		"iret"
#endif
		);
#endif
#if 0
	__asm__ __volatile__ (
		"movl %0, %%eax\n\t"
		"movl %%eax, %%gs"
		:
		: "i"(8 | 0x04)
		);
#endif
}

void init_desc2(struct desc_struct *desc, uint32_t base, uint32_t limit, uint32_t attr)
{
	printf("base %X, limit %X\n", base, limit);

	desc->limit_low	       = limit & 0x0FFFF;
	desc->base_low	       = base & 0x0FFFF;
	desc->base_mid	       = (base >> 16) & 0x0FF;
	desc->attr1	       = attr & 0xFF;
	desc->limit_high_attr2 = (((limit >> 16) & 0x0F)) | ((attr >> 8) & 0xF0);
	desc->base_high	       = (base >> 24) & 0x0FF;
}

void init_desc(desc_t *desc, uint32_t base, uint32_t limit, uint32_t attr)
{
/* 	printf("base %X, limit %X\n", base, limit); */

	desc->l = (limit & 0x0FFFF) | ((base & 0x0FFFF) << 16);
	desc->h = ((base >> 16) & 0x0FF) | (base & 0xF0000000) | (attr << 8) | ((limit >> 16) & 0x0F);
}
