#include <kernel.h>
#include <io.h>
#include <time.h>
#include <irq.h>
#include <task.h>
#include <sched.h>

static int schedinit = 0;
static task_t *cur_task = NULL;

static task_t *get_next_task(void)
{
	if (cur_task == &task1)
		return &task0;
	else
		return &task1;
}

void sched_init(void)
{
	schedinit = 1;
}

void schedule(void)
{
	task_t *next;

	if (!schedinit)
		return;

	next = get_next_task();
	if (next == cur_task)
		return;

	cur_task = next;
	TSS_INIT(next);
	init_desc(&gdt[LDT_START], (uint32_t)(&(next->ldt0)), 15, DA_LDT | DA_DPL3);
/* 	__switch_to(next); */
	asm __volatile__ (
		"push %%ds\n\t"
		"push %%eax\n\t"
		"movl $0x10, %%eax\n\t"
		"movw %%ax, %%ds\n\t"
		"ljmp $0x80, $0\n\t"
		"popl %%eax\n\t"
		"popl %%ds"
		::
		);
}

