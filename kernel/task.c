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
			"push $0x0f\n\t"		\
			"push %%esp\n\t"		\
			"pushfl\n\t"			\
			"push $0x07\n\t"		\
			"pushl $1f\n\t"			\
			"iret\n\t"			\
			"1:\tmovl $0x0f,%%eax\n\t"	\
			"movw %%ax,%%ds\n\t"		\
			"movw %%ax,%%es\n\t"		\
			"movw %%ax,%%fs\n\t"		\
			"movw %%ax,%%gs"		\
			:				\
			: "a"((task)->entry)		\
			);				\
	}

static void creat_task1(void);
void second_task(void *arg);
static void hexdebug(uint8_t *d, size_t len);

static void first_task(void *arg)
{
	while (1) {
		printf("first task\n");
		pause();
	}
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
	tss->es = 0x08 | 7;
	tss->cs = 0x00 | 7;
	tss->ds = 0x08 | 7;
	tss->ss	= 0x08 | 7;
	tss->fs	= 0x08 | 7;
	tss->gs = 0x08 | 7;
	tss->ldt  = SEG_SELECTOR(LDT_START);
	tss->iobase = 0x8000000;
	tss->cr3 = 0x00200000;

	task->entry = (uint32_t)first_task;

	memcpy(&task->ldt0, &ldt[1], sizeof(gdt[0]));
	memcpy(&task->ldt1, &ldt[2], sizeof(gdt[0]));

	TSS_INIT(task);
	init_desc(&gdt[LDT_START], (uint32_t)(&(task->ldt0)), 15, DA_LDT | DA_DPL3);
}

extern void test_task(void);
static char task2_stack[1024];
static void creat_task1(void)
{
	struct tss_struct *tss = &task1.tss;
	task_t *task = &task1;

	/* init tss */
	memset(tss, 0, sizeof(struct tss_struct));
	tss->esp0 = KERNEL_STACK + STACK_SIZE;
	tss->ss0  = KERNEL_DATA_SELECTOR;
	tss->eip  = (uint32_t)second_task;
	tss->eflags  = 0x200;
	tss->esp = TASK1_STACK + STACK_SIZE;
	tss->ebp = TASK1_STACK + STACK_SIZE;
	tss->es = 0x08 | 7;
	tss->cs = 0x00 | 7;
	tss->ds = 0x08 | 7;
	tss->ss	= 0x08 | 7;
	tss->fs	= 0x08 | 7;
	tss->gs = 0x08 | 7;
	tss->ldt  = SEG_SELECTOR(LDT_START);
	tss->iobase = 0x8000000;
	tss->cr3 = 0x00200000;

	task->entry = (uint32_t)second_task;

	memcpy(&task->ldt0, &ldt[1], sizeof(gdt[0]));
	memcpy(&task->ldt1, &ldt[2], sizeof(gdt[0]));
#if 0
	memcpy(&task->ldt0, &gdt[KERNEL_CODE], sizeof(gdt[0]));
	memcpy(&task->ldt1, &gdt[KERNEL_DATA], sizeof(gdt[0]));
#endif
#if 0
	memcpy(&ldt[3], &gdt[KERNEL_CODE], sizeof(gdt[0]));
	memcpy(&ldt[4], &gdt[KERNEL_DATA], sizeof(gdt[0]));
#endif
/* 	task->ldt0.attr1 = DA_C   | PRIVILEGE_USER << 5; */
/* 	task->ldt1.attr1 = DA_DRW | PRIVILEGE_USER << 5; */

#if 0
	TSS_INIT(task);
	init_desc(&gdt[LDT_START], (uint32_t)(&(task->ldt0)), 15, DA_LDT | DA_DPL3);
	hexdebug((uint8_t *)&gdt[TSS], sizeof(gdt[0]));
	hexdebug((uint8_t *)&(task->ldt0), sizeof(ldt[0]));
	hexdebug((uint8_t *)&(gdt[1]), sizeof(ldt[0]));
	hexdebug((uint8_t *)&(ldt[1]), sizeof(ldt[0]));
#endif
/* 	pause(); */
}

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
#if 0
	task0.pid = 0;
	task0.stack_base = (unsigned long *)TASK0_STACK;
	init_thread->task = &task0;

	task0.ldt_sel = LDT_SELECTOR;
	arch_setup_task(&task0, first_task);

	printf("arch_setup_task over\n");
	printf("task0 addr %X\n", (uint32_t)&task0);
/* 	switch_to(&task0); */
#endif

	/* creat task 0, and go to user mode here! */
	creat_task0();
	load_ldt();
	load_tss();
	__switch(&task0);
/* 	pause(); */
#if 1
	/* creat task 1, running in user mode */
	creat_task1();
/* 	asm __volatile__ ("ljmp $0x80, $0"); */
#endif
}

