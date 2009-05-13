#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "io.h"
#include "time.h"
#include "irq.h"

static void init_8259A(void)
{
	/* ICW1 */
	outb_p(0x11, 0x20);
	outb_p(0x11, 0xA0);

	/* ICW2 */
	outb_p(MASTER_OFFSET, 0x21);	/* master offset 0x20 of the IDT */
	outb_p(SLAVE_OFFSET,  0xA1);	/* slave offset 0x28 of the IDT */

	/* ICW3 */
	outb_p(0x04, 0x21);	/* slave attached to IR 2 */
	outb_p(0x02, 0xA1);	/* this slave in IR line 2 of master */

	/* ICW4 */
	outb_p(0x01, 0x21);	/* set as master */
	outb_p(0x01, 0xA1);	/* set as slave */

	/* mask all INTs */
	outb_p(0xFB, 0x21);
	outb_p(0xFF, 0xA1);
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
void irq_init(void)
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
#if 0
	init_8253();

	set_intr_gate(PIC_IRQ_RT, (unsigned long)timer_interrupt);
#endif

	idtr.limit = 256 * 8 - 1;
	idtr.base  = (uint32_t)idt;
	__asm__ __volatile__ ("lidt %0" : : "m"(idtr));
	__asm__ __volatile__ ("cli");
}

void irq_enable(int irqno)
{
}

void irq_disable(int irqno)
{
}

#if 0
int request_irq(unsigned int irq, irq_handler_t handler, 
		unsigned long irqflags, const char *devname, void *dev_id)
{
	return 0;
}
#endif
