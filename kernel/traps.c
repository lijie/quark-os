#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "io.h"

extern void die(void);

#define	going_to_die() do {				\
		printf("%s %d\n", __func__, errcode);	\
		die();					\
	} while (0)

void do_divide_error(long esp, long errcode)
{
	going_to_die();
}

void do_bounds_error(long esp, long errcode)
{
	mem_dump4(esp, 4);
	going_to_die();
}

void do_invalid_tss(long esp, long errcode)
{
	going_to_die();
}

void do_double_fault(long esp, long errcode)
{
	going_to_die();
}

/* 
 * trap entries.
 * defined in irq_entry.S
 */
extern void divide_error(void);
extern void bounds_error(void);
extern void invalid_tss(void);
extern void double_fault(void);

void traps_init(void)
{
	set_trap_gate( 0, (uint32_t)divide_error);
	set_trap_gate( 5, (uint32_t)bounds_error);
	set_trap_gate(10, (uint32_t)invalid_tss );

#if 0
	/* FIXME: bounds error ! why ? */
	set_trap_gate( 8, (uint32_t)double_fault);

	/* FIXME: what are these? */
	outb_p(inb_p(0x21)&0xfb,0x21);
	outb(inb_p(0xA1)&0xdf,0xA1);
#endif
}
