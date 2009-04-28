#include "config.h"
#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "io.h"
#include "init.h"
#include "irq.h"
#include "task.h"
#include "time.h"

char *kernel_stack = (char *)KERNEL_STACK;

static inline void delay(void)
{
	size_t i, j;

	for (i = 0; i < 0xFF; i++)
		for (j = 0; j < 0xFFFFFFF; j++);
}

/* here! we are in the world of C!! */
void kernel_start(void)
{
	puts("Welcome to my kernel world!\n");
	printf("This is printf %d\n", 1996);
	printf("mem size %d\n", get_mem_size());

	time_init();
	traps_init();
	sti();
	task_init();
/* 	sched_init(); */
#if 0
	/* task module is not complete... */

	printf("current task is %d\n", current->pid);
	printf("init task is %d\n", task0.pid);

	__asm__ __volatile__ ("sti");
	__asm__ __volatile__ ("int $0x80");
	__asm__ __volatile__ ("int $0x80");
	__asm__ __volatile__ ("int $0x80");
#endif

	show_i386_regs();

/* 	while (1) { */
/* 		printf("main task %d\n", jiffies); */
/* 		delay(); */
/* 	} */

	/* end here */
/* 	while (jiffies < 500); */
/* 	printf("jiffies %d\n", jiffies); */
	while (1);
}
