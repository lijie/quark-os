#include "kernel.h"
#include "stdio.h"
#include "string.h"
#include "io.h"

extern void die(void);

#define	going_to_die() do {				\
		printf("%s %08X\n", __func__, errcode);	\
		mem_dump4((unsigned long)esp, 8);	\
		die();					\
	} while (0)

void do_divide_error(long esp, long errcode)
{
	going_to_die();
}

void do_bounds_error(long esp, long errcode)
{
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

void do_overflow_trap(long esp, long errcode)
{
	going_to_die();
}

void do_invalid_opcode(long esp, long errcode)
{
	going_to_die();
}

void do_dev_noavailable(long esp, long errcode)
{
	going_to_die();
}

void do_seg_nopresent(long esp, long errcode)
{
	going_to_die();
}

void do_stack_fault(long esp, long errcode)
{
	going_to_die();
}

void do_page_fault(long esp, long errcode)
{
	going_to_die();
}

void do_gp_exception(long esp, long errcode)
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
extern void overflow_trap(void);
extern void invalid_opcode(void);
extern void dev_noavailable(void);
extern void seg_nopresent(void);
extern void stack_fault(void);
extern void page_fault(void);
extern void gp_exception(void);

void traps_init(void)
{
	set_trap_gate( 0, (uint32_t)divide_error );
	set_trap_gate( 4, (uint32_t)overflow_trap);
	set_trap_gate( 5, (uint32_t)bounds_error );
	set_trap_gate( 6, (uint32_t)invalid_opcode);
	set_trap_gate( 7, (uint32_t)dev_noavailable);
	set_trap_gate(10, (uint32_t)invalid_tss  );
	set_trap_gate(11, (uint32_t)seg_nopresent);
	set_trap_gate(12, (uint32_t)stack_fault);
	set_trap_gate(13, (uint32_t)gp_exception);
	set_trap_gate(14, (uint32_t)page_fault);

	/* FIXME: bounds error ! why ? */
	set_trap_gate( 8, (uint32_t)double_fault);

#if 0
	/* FIXME: what are these? */
	outb_p(inb_p(0x21)&0xfb,0x21);
	outb(inb_p(0xA1)&0xdf,0xA1);
#endif
}
