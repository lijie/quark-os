#include "i386.h"
#include "mm.h"

#define	MM_START_ADDR	0
#define	MM_END_ADDR	0

static size_t total_pages = 0;

void mm_init(void)
{
	total_pages = (MM_END_ADDR - MM_START_ADDR) / PAGE_SIZE;
}

