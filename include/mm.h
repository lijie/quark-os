#ifndef	__MM_H__
#define	__MM_H__

#include "config.h"
#include "types.h"

static inline unsigned long virt_to_phys(unsigned long virt)
{
	return virt - __PAGE_OFFSET;
}

static inline unsigned long phys_to_virt(unsigned long phys)
{
	return phys + __PAGE_OFFSET;
}

#define	__va(p)		((void *)(phys_to_virt(p)))
#define	__pa(v)		((void *)(virt_to_phys(v)))

struct mm_struct {
	/* nothing */
};

extern unsigned long mem_size_kbytes;

#define	ZONE_DMA_SIZE	(16 * 1024 * 1024)
#endif
