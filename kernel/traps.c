#include "kernel.h"
#include "stdio.h"
#include "io.h"

extern void die(void);

void do_divide_error(long esp, long errcode)
{
	printf("divide error %d!\n", errcode);
	die();
}

#ifdef	__TRAP_DEBUG__
static void trap_debug(void)
{
	printf("I'm in trap_debug\n");
	outb(0x20, 0x20);
}

static int trap_debug_div0(int i)
{
	__asm__ __volatile__ ("idivl %0"
			      :
			      : "a"(i));
}
#endif

extern void divide_error(void);
void traps_init(void)
{
#if 0
	set_trap_gate(0, (uint32_t)divide_error);
	set_trap_gate(13, (uint32_t)divide_error);
#endif
}
