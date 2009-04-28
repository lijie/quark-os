#include "kernel.h"
#include "stdio.h"
#include "io.h"

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

struct idt_desc idt_table[256];
extern void intr_handler(void);

void intr_init(void)
{
	uint32_t handler = (uint32_t)intr_handler;
	struct idt_desc desc;
	size_t i;
#if 0
/* 	desc.a = handler; */
	desc.a = (8 << 16) | (handler & 0x0000FFFF);
	desc.b = | ((handler >> 16) & 0x0000FFFF);
#endif
	desc.offset_low = handler;
	desc.offset_hig = handler >> 16;
	desc.selector = 8;
	desc.unused = 0;
	desc.P = 1;
	desc.DPL = 0;
	desc.saved = 6;
	desc.saved_0 = 0;
	desc.D = 1;

	for (i = 0; i < 256; i++)
		idt_table[i] = desc;

	init_8259A();
}

void irq_enable(int irqno)
{
}

void irq_disable(int irqno)
{
}
