#ifndef	__STRING_H__
#define	__STRING_H__

#include "types.h"

extern size_t strnlen(const char * s, size_t count);
extern void * memcpy(void *dest, void *src, size_t count);
extern void memset(void *dest, int value, size_t count);

extern void mem_dump(uint8_t *addr, uint32_t size);
#endif
