#include "types.h"
#include "config.h"
#include "system.h"

#ifndef	HAVE_MEMCPY
void * memcpy(void *dest, void *src, size_t count)
{
	char *d = (char *)dest;
	char *s = (char *)src;

	while (count--)
		*d++ = *s++;

	return dest;
}
#endif

#ifndef	HAVE_MEMSET
void memset(void *dest, int value, size_t count)
{
	char *d = (char *)dest;

	while (count--)
		*d++ = (char)value;
}
#endif

size_t strnlen(const char * s, size_t count)
{
	const char *sc;

	for (sc = s; count-- && *sc != '\0'; ++sc)
		/* nothing */;
	return sc - s;
}

void mem_dump(unsigned long addr, uint32_t blk, uint32_t size)
{
	static char * mdformat[] = {
		"%02X", "%04X", "%08X",
	};

	int i = size;
	int ps; 

#define	_MEMPRINT_ \
	for (i = 0; i < size; i++) \
		printf(mdformat[ps], pt[i]); \
	printf("\n");

	if (blk == 1) {
		uint8_t *pt = (uint8_t *)addr;
		ps = 0;
		_MEMPRINT_;
	} else if (blk == 2) {
		uint16_t *pt = (uint16_t *)addr;
		ps = 1;
		_MEMPRINT_;
	} else if (blk == 4) {
		uint32_t *pt = (uint32_t *)addr;
		ps = 2;
		_MEMPRINT_;
	} else {
		BUG();
	}

#undef	_MEMPRINT_
}
