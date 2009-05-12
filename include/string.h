#ifndef	__STRING_H__
#define	__STRING_H__

#include "types.h"

extern size_t strnlen(const char * s, size_t count);
extern void * memcpy(void *dest, void *src, size_t count);
extern void memset(void *dest, int value, size_t count);

extern void mem_dump(uint8_t *addr, uint32_t blk, uint32_t size);
#define	mem_dump1(a, s)		mem_dump(a, 1, s)
#define	mem_dump2(a, s)		mem_dump(a, 2, s)
#define	mem_dump4(a, s)		mem_dump(a, 4, s)
#endif
