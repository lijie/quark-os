#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "io.h"
#include "time.h"
#include "irq.h"

static void init_8259A(void)
{
#if 0
	outb_p(0x20, 0x11);
	outb_p(0xA0, 0x11);
	outb_p(0x21, 0x20);
	outb_p(0xA1, 0x28);
	outb_p(0x21, 0x04);
	outb_p(0xA1, 0x02);
	outb_p(0x21, 0x01);
	outb_p(0xA1, 0x01);
	outb_p(0x21, 0xFF);
	outb_p(0xA1, 0xFF);
#endif
}

static void init_8253(void)
{
	outb_p(0x43, 0x36);		/* binary, mode 3, LSB/MSB, ch 0 */
	outb_p(0x40, LATCH & 0xff);	/* LSB */
	outb(0x40, LATCH >> 8);		/* MSB */
}

struct idt_desc idt_table[256];
struct idtr_struct __attribute__((aligned(2))) idtr;

extern struct idt_desc idt[];

/* default interruption handler */
extern void ignore_int(void);

#define	default_handler	ignore_int
void intr_init(void)
{
	uint32_t handler = (uint32_t)default_handler;
	struct idt_desc desc;
	size_t i;

	desc.offset_low = handler;
	desc.offset_hig = handler >> 16;
	desc.selector	= 8;
	desc.unused	= 0;
	desc.P		= 1;
	desc.DPL	= 0;
	desc.saved	= 6;
	desc.saved_0	= 0;
	desc.D		= 1;

	for (i = 0; i < 256; i++)
		idt[i] = desc;

	init_8259A();
	init_8253();

	set_intr_gate(8, (unsigned long)timer_interrupt);

	idtr.limit = 256 * 8 - 1;
	idtr.base  = (uint32_t)idt;
	__asm__ __volatile__ ("lidt %0" : : "m"(idtr));
}

void irq_enable(int irqno)
{
}

void irq_disable(int irqno)
{
}
