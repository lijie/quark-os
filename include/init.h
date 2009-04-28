#ifndef	__INIT_H__
#define	__INIT_H__

#include "types.h"
extern char *kernel_stack;

static inline uint32_t get_mem_size(void)
{
	uint32_t size = *(int *)0x8000;
	return size;
}

#endif
