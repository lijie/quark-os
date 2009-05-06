#include <kernel.h>
#include <types.h>
#include <time.h>
#include <io.h>
#include <irq.h>
#include <sched.h>

void do_time_interrupt(void)
{
	schedule();
}

uint32_t ktime(void)
{
	return jiffies / HZ;
}

void msleep(uint32_t ms)
{
	uint32_t j = jiffies;

	while ((jiffies - j) < (ms / 10))
		schedule();
}

void sleep(uint32_t s)
{
	msleep((s * 1000));
}

void time_init(void)
{
}
