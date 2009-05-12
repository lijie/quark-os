#include "config.h"
#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "io.h"
#include "init.h"
#include "irq.h"
#include "task.h"
#include "time.h"
#include "mm.h"

static inline void delay(void)
{
	size_t i, j;

	for (i = 0; i < 0xFF; i++)
		for (j = 0; j < 0xFFFFFFF; j++);
}

extern unsigned long testv1;
extern unsigned long testv2;

/* here! we are in the world of C!! */
void kernel_start(void)
{
	uint32_t beyond;

	puts("Welcome to my kernel world!\n");
	printf("This is printf %d\n", 1996);
	printf("mem size %d bytes\n", mem_size_kbytes * 1024);
	printf("v1 %X v2 %X\n", testv1, testv2);

	time_init();
	intr_init();
	traps_init();
	mm_init();
	sti();

#if 0

//	__asm__ __volatile__ ("idivl 0");

#endif
	/* task_init(); */
	beyond = 2000;
	while (1) {
		if (jiffies > beyond) {
			beyond += 2000;
//			printf("jiffies %d\n", jiffies);
			delay();
		}
	}
}
