#include "i386.h"
#include "system.h"
#include "mm.h"
#include "page.h"

static size_t total_pages = 0;
static unsigned long free_mem_start;

static struct page * free_page_start;
struct page * mem_map;

static struct zone zone[ZONE_NR];
struct zonelist zonelist;

static void zone_init(void)
{
}

static void mm_reserved_init(void)
{
	struct page *end = virt_to_page(free_mem_start);
	struct page *start = mem_map;

	if (!end) {
		BUG();
	}

	for (; start != end; start++) {
		atmoic_inc(&start->count);
		PG_SETRVSD(start);
	}
}

void mm_init(void)
{
	uint32_t pages = (mem_size_kbytes + 1024) >> 2;
	uint32_t pgtbl_size = pages * sizeof(unsigned long);

	free_mem_start = PAGE_ALIGN(((unsigned long)pg + PGDIR_SIZE + pgtbl_size));

	mem_map = (struct page *)free_mem_start;
	free_mem_start = PAGE_ALIGN(free_mem_start + pages * sizeof(struct page));

	mm_reserved_init();
	printf("mem_map %X\n", mem_map);
	printf("free_mem_start %X\n", free_mem_start);

	zone_init();
}

