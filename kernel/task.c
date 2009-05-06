#include "stdio.h"
#include "task.h"
#include "arch.h"
#include "string.h"
#include "time.h"

static inline void delay(void)
{
	size_t i, j;

	for (i = 0; i < 0xFF; i++)
		for (j = 0; j < 0xFFF; j++);
}

task_t task0;
task_t task1;
#if 0
union thread_union *init_thread = (union thread_union *)(KERNEL_STACK - STACK_SIZE);
#else
#define	TASK0_STACK	0xC0400000
#define	TASK1_STACK	(TASK0_STACK + STACK_SIZE)
union thread_union *init_thread = (union thread_union *)(TASK0_STACK - STACK_SIZE);
#endif

#define __switch(task) {				\
		asm __volatile__ (			\
			"pushfl\n\t"			\
			"andl $0xFFFFBFFF, (%%esp)\n\t"	\
			"popfl\n\t"			\
			"push $0x17\n\t"		\
			"push %%esp\n\t"		\
			"pushfl\n\t"			\
			"push $0x0f\n\t"		\
			"pushl %%eax\n\t"		\
			"iret\n\t"			\
			:				\
			: "a"((task)->entry)		\
			);				\
	}

static void creat_task1(void);
void second_task(void *arg);
static void hexdebug(uint8_t *d, size_t len);

static void first_task(void *arg)
{
	__asm__ __volatile__ (			\
		"movw %%ax,%%ds\n\t"            \
		"movw %%ax,%%es\n\t"            \
		"movw %%ax,%%fs\n\t"            \
		"movw %%ax,%%gs\n\t"		\
		"movw %%ax,%%ss\n\t"		\
		:				\
		: "a"(0x17)			\
		);

	printf("first task\n");
	pause();
}

void second_task(void *arg)
{
#if 0
	int i = 0;
	struct tss_struct *tss = &task0.tss;

	printf("second task %d\n", jiffies);
	printf("task 0: %X,%X, %X\n", tss->esp, tss->ebp, tss->eip);
	while (i < 500) {
		i++;
		printf("second task %d\n", jiffies);
		delay();
	}
#endif
	pause();
}

static void creat_task0(void)
{
	struct tss_struct *tss = &task0.tss;
	task_t *task = &task0;

	/* init tss */
	memset(tss, 0, sizeof(struct tss_struct));
	tss->esp0 = TASK0_STACK + STACK_SIZE;
	tss->ss0  = KERNEL_DATA_SELECTOR;
	tss->ldt  = SEG_SELECTOR(LDT);
	tss->iobase = 0x8000000;
//	tss->cr3 = 0x00200000;

	task->entry = (uint32_t)first_task;
	printf("first entry %X\n", task->entry);

	init_desc(&gdt[TSS], (uint32_t)&((task)->tss), sizeof(struct tss_struct) - 1, DA_386TSS | DA_DPL3);
	init_desc(&gdt[LDT], (uint32_t)(&ldt[0]), 0x40, DA_LDT | DA_DPL3);

	mem_dump((uint8_t *)&gdt[TSS], sizeof(gdt[0]));
}

extern void test_task(void);
static char task2_stack[1024];

#define move_to_user_mode() \
__asm__ ("movl %%esp,%%eax\n\t" \
	"pushl $0x0f\n\t" \
	"pushl %%eax\n\t" \
	"pushfl\n\t" \
	"pushl $0x0f\n\t" \
	"pushl $1f\n\t" \
	"iret\n" \
	"1:\tmovl $0x17,%%eax\n\t" \
	"movw %%ax,%%ds\n\t" \
	"movw %%ax,%%es\n\t" \
	"movw %%ax,%%fs\n\t" \
	"movw %%ax,%%gs" \
	:::"ax")

void task_init(void)
{
	/* creat task 0, and go to user mode here! */
	creat_task0();
	load_ldt(SEG_SELECTOR(LDT));
	load_tss(SEG_SELECTOR(TSS));
	__switch(&task0);
}

