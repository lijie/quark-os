#ifndef	__STRING_H__
#define	__STRING_H__

#include "types.h"

#define	HAVE_MEMCPY
#define	__memcpy_byte(dest, src, size) {				\
		__asm__ __volatile__ (					\
			"cld\n\t"					\
			"rep\n\t"					\
			"movsb\n\t"					\
			:						\
			: "S"(src), "D"(dest), "c"(size)		\
			);						\
	}

static inline void * memcpy(void *dest, void *src, size_t size)
{
	__memcpy_byte(dest, src, size);
	return dest;
}

extern size_t strnlen(const char * s, size_t count);
#ifndef	HAVE_MEMCPY
extern void * memcpy(void *dest, void *src, size_t count);
#endif
extern void memset(void *dest, int value, size_t count);

extern void mem_dump(unsigned long addr, uint32_t blk, uint32_t size);
#define	mem_dump1(a, s)		mem_dump(a, 1, s)
#define	mem_dump2(a, s)		mem_dump(a, 2, s)
#define	mem_dump4(a, s)		mem_dump(a, 4, s)
#endif
