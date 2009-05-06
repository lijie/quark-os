#ifndef	__MM_H__
#define	__MM_H__

#include "config.h"
#include "types.h"
#include "task.h"

struct page_struct {
	int used;
	uint32_t addr;
};

typedef struct page_struct page_t;

static inline unsigned long virt_to_phys(unsigned long virt)
{
	return virt - __PAGE_OFFSET;
}

static inline unsigned long phys_to_virt(unsigned long phys)
{
	return phys + __PAGE_OFFSET;
}

struct mm_struct {
	/* nothing */
};

extern void * __get_free_pages(size_t count);

static inline void * get_free_pages(size_t count)
{
	return __get_free_pages(count);
}

static inline void * get_one_page(void)
{
	return __get_free_pages(1);
}

#if 0
static inline task_t * alloc_one_task(void)
{
	return (task_t *)__get_free_pages();
}
#endif

extern unsigned long mem_size_kbytes;
#endif
