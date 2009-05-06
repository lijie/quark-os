#ifndef	__IO_H__
#define	__IO_H__

#include "types.h"

#define __SLOW_DOWN_IO "jmp 1f; 1: jmp 1f; 1:"
static inline void slow_down_io(void) {
	__asm__ __volatile__(
		__SLOW_DOWN_IO __SLOW_DOWN_IO __SLOW_DOWN_IO
		: : );
}

static inline void outb(uint8_t port, uint8_t val)
{
	__asm__ __volatile__ ("outb %0, %w1" : : "a"(val), "Nd"(port));
}

static inline uint8_t inb(uint8_t port)
{
	uint8_t val;
	__asm__ __volatile__ ("inb %w1, %0" : "=a"(val) : "Nd"(port));
	return val;
}

static inline void outb_p(uint8_t port, uint8_t val)
{
	outb(port, val);
	slow_down_io();
/* 	__asm__ __volatile ("nop\n\tnop\n\tnop\n\t"); */
}

#endif
