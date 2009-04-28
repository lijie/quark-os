#ifndef	__I386_DEBUG__
#define	__I386_DEBUG__

#include "types.h"
#include "stdio.h"

#define	__TRAP_DEBUG__

static inline void hexdebug(uint8_t *d, size_t len)
{
	int i;

	for (i = 0; i < len; i++)
		printf("%02X", d[i]);
	printf("\n");
}

#endif
