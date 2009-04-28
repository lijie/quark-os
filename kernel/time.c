#include <kernel.h>
#include <types.h>
#include <time.h>
#include <io.h>
#include <irq.h>
#include <sched.h>

#define	LATCH			(1193180 / 1)

void do_time_interrupt(void)
{
	schedule();
}

uint32_t ktime(void)
{
	return jiffies / HZ;
}

void time_init(void)
{
	jiffies = 0;

	/* init 8253 for 100hz timer interrupt */
	outb_p(0x43, 0x36);		/* binary, mode 3, LSB/MSB, ch 0 */
	outb_p(0x40, LATCH & 0xff);	/* LSB */
	outb(0x40, LATCH >> 8);		/* MSB */

	set_intr_gate(8, (unsigned long)time_interrupt);
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
