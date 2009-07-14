#ifndef	__PAGE_H__
#define	__PAGE_H__

#include "i386.h"
#include "atmoic.h"
#include "mm.h"
#include "list.h"

typedef	unsigned long pgd_t;
typedef	unsigned long pte_t;

static inline void pgd_info(unsigned long addr)
{
	unsigned long *pgdir = (unsigned long *)pg;
	unsigned long *ptdir = (unsigned long *)(pg + 4096);

	printf("addr %p, pg %p pge %p pte %p\n", addr, pg, 
	       pgdir[addr >> 22], ptdir[(addr >> 12) & 0x03FF]);
}

struct page {
	atmoic_t  		count;
	unsigned long 		flags;
	unsigned long 		private;
	struct list_head 	lru;
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

#define	ALIGN(a, b)		(((a) + (b) - 1) & ~(b - 1))
#define	PAGE_ALIGN(a)		(((a) + PAGE_SIZE - 1) & ~PAGE_MASK)

static inline unsigned long page_to_pfn(struct page *page)
{
	return page - mem_map;
}

static inline struct page * virt_to_page(unsigned long virt)
{
	if (virt < __PAGE_OFFSET)
		return NULL;

	return &mem_map[(virt - __PAGE_OFFSET) >> PAGE_SHIFT];
}

static inline void * page_address(struct page *page)
{
	return __va(page_to_pfn(page) << PAGE_SHIFT);
}

static inline unsigned long page_count(struct page *page)
{
	return page->count;
}

static inline void set_page_count(struct page *page, int count)
{
	page->count = count;
}

static inline void set_page_refcounted(struct page *page)
{
	set_page_count(page, 1);
}

static inline int page_isfree(struct page *page)
{
	return ((page_count(page) == 0) && (!PG_RVSD(page)));
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
	struct free_area free_areas[MAX_ORDER + 1];
	unsigned long spanned_pages;
	unsigned long free_pages;
	char *name;
};

struct zonelist {
	struct zone *zones[ZONE_NR + 1];
};

extern struct zone zones[];

static inline void page_setzone(struct page *page, int zone_type)
{
	page->flags |= zone_type << 30;
}

static inline struct zone * page_zone(struct page *page)
{
	return zones + (page->flags >> 30);
}

extern struct page * alloc_pages(unsigned long gfp_mask, int order);

static inline struct page * alloc_page(unsigned long gfp_mask)
{
	return alloc_pages(gfp_mask, 0);
}

static inline void * get_free_pages(unsigned long gfp_mask, int order)
{
	struct page * page = alloc_pages(gfp_mask, order);

	if (page) 
		return page_address(page);
	else
		return NULL;
}

static inline void * get_free_page(unsigned long gfp_mask)
{
	return get_free_pages(gfp_mask, 0);
}

extern void __free_pages(struct page *page, int order);

static inline void free_pages(unsigned long addr, int order)
{
	__free_pages(virt_to_page(addr), order);
}
#endif
