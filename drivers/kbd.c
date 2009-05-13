#include "stdio.h"
#include "string.h"
#include "system.h"
#include "irq.h"

static irqreturn_t kbd_interrupt(void *dev_id)
{
	printf("%s \n", __func__);
}

int kbd_init(void)
{
	request_irq(PIC_IRQ_KBD, (irq_handler_t)kbd_interrupt, 0, "kbd", NULL);
	return 0;
}
