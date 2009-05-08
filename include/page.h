#ifndef	__PAGE_H__
#define	__PAGE_H__

#include "i386.h"
#include "atmoic.h"
#include "mm.h"
#include "list.h"

struct page {
	atmoic_t  	count;
	unsigned long 	flags;
};

#define	PGSTRUCT_SIZE		sizeof(struct page)

enum {
	PG_USED = 0,

	/* pages which saved kernel code or cannot be used */
	PG_RVSD,
	PG_ERROR,
};


#define	PG_SET(pg, bit)		(pg->flags |=  (1 << bit))
#define	PG_CLR(pg, bit)		(pg->flags &= ~(1 << bit))
#define	PG_ISSET(pg, bit)	(pg->flags & (1 << bit))

#define	PG_SETUSED(pg)		PG_SET(pg, PG_USED)
#define	PG_CLRUSED(pg)		PG_CLR(pg, PG_USED)
#define	PG_USED(pg)		PG_ISSET(pg, PG_USED)

#define	PG_SETRVSD(pg)		PG_SET(pg, PG_RVSD)
#define	PG_CLRRVSD(pg)		PG_CLR(pg, PG_RVSD)
#define	PG_RVSD(pg)		PG_ISSET(pg, PG_RVSD)

#define	PG_SETERROR(pg)		PG_SET(pg, PG_ERROR)
#define	PG_CLRERROR(pg)		PG_CLR(pg, PG_ERROR)
#define	PG_ERROR(pg)		PG_ISSET(pg, PG_ERROR)

/* 
 * addr where saved page structures
 */
extern struct page * mem_map;

#define	PAGE_ALIGN(a)		(((a) + PAGE_SIZE - 1) & ~PAGE_MASK)

static inline struct page * virt_to_page(unsigned long virt)
{
	if (virt < __PAGE_OFFSET)
		return NULL;

	return &mem_map[(virt - __PAGE_OFFSET) >> PAGE_SHIFT];
}

static inline void * page_address(struct page *page)
{
	return __va((page - mem_map) << PAGE_SHIFT);
}

enum {
	ZONE_DMA = 0,
	ZONE_NORMAL,
	ZONE_HIGHMEM,
	ZONE_NR,
};

#define	MAX_ORDER		11

struct free_area {
	struct list_head free_list;
	unsigned long nr_free;
};
	
struct zone {
	struct page * zone_mem_map;
	unsigned long zone_start_pfn;
	struct free_area free_area[MAX_ORDER];
	char *name;
};

struct zonelist {
	struct zone *zones[ZONE_NR + 1];
};

#endif
