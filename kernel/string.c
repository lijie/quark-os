#include "types.h"
#include "config.h"

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
