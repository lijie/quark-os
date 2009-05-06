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

/* here! we are in the world of C!! */
void kernel_start(void)
{
	uint32_t beyond;

	puts("Welcome to my kernel world!\n");
	printf("This is printf %d\n", 1996);
	printf("mem size %d bytes\n", mem_size_kbytes * 1024);

	intr_init();
	time_init();
	traps_init();
	sti();

	/* task_init(); */
	beyond = 500;
	while (1) {
		if (jiffies > beyond) {
			beyond += 500;
			printf("jiffies %d\n", jiffies);
			delay();
		}
	}
}
