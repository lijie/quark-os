#include "i386.h"
#include "system.h"
#include "mm.h"
#include "page.h"
#include "string.h"

static size_t total_pages = 0;
static unsigned long free_mem_start;

static struct page * free_page_start;
struct page * mem_map;

struct zone zones[ZONE_NR];
struct zonelist zonelist;

static char * zone_names[] = {
	"ZONE_DMA",
	"ZONE_NORMAL",
	"ZONE_HIGHMEM",
};

static void free_area_stats(struct zone *);

static struct page * find_zone_mem_map(struct zone *zone)
{
	if (zone == &zones[ZONE_DMA]) {
		return mem_map;
	} else if (zone == &zones[ZONE_NORMAL]) {
		return &mem_map[ZONE_DMA_SIZE >> PAGE_SHIFT];
	} else {
		BUG();
	}
}

static void free_area_init(void)
{
	struct zone *zone;

	for (zone = zones; zone < zones + ZONE_NR - 1; zone++) {
		int order = MAX_ORDER;
		unsigned long pagenum = zone->spanned_pages;

		while (order >= 0) {
			struct free_area *area = &zone->free_areas[order];
			unsigned long unit, i;

			INIT_LIST_HEAD(&area->free_list);
			area->nr_free = 0;

			unit = 1 << order;
			for (i = 0; i < pagenum; i += unit) {
				struct page * page = &zone->zone_mem_map[i];
				if (page_isfree(page)) {
					list_add(&page->lru, &area->free_list);
					area->nr_free++;
#if 0
					if (order == 1)
						printf("%s: count %d pagenum %d unit %d free %d\n", 
						       zone->name, i, pagenum, unit, area->nr_free);
#endif
				}
			}

			pagenum -= area->nr_free * (1 << order);
			if (pagenum == 0)
				break;
			if (pagenum > zone->spanned_pages)
				BUG();

			order--;
		}

#if 0
		order = MAX_ORDER;
		while (order >= 0) {
			printf("free area %d\n", zone->free_areas[order].nr_free);
			order--;
		}
#endif
	}

}

static void zone_init(void)
{
	struct zone *zone;

	for (zone = zones; zone < zones + ZONE_NR - 1; zone++) {
		memset(zone, 0, sizeof(struct zone));

		zone->zone_mem_map = find_zone_mem_map(zone);
		zone->name = zone_names[zone - zones];
	}

	zonelist.zones[0] = &zones[ZONE_NORMAL];
	zonelist.zones[1] = &zones[ZONE_DMA];
	zonelist.zones[1] = NULL;
}

static void page_init(struct page *start, uint32_t num)
{
	struct page *end = start + num;
	struct zone *zone;

	printf("%s: %p, %p\n", __func__, start, end);
	for (; start != end; start++) {
//		memset(start, 0, sizeof(struct page));
		INIT_LIST_HEAD(&start->lru);

		if (start < mem_map + (ZONE_DMA_SIZE >> PAGE_SHIFT))
			zone = &zones[ZONE_DMA];
		else
			zone = &zones[ZONE_NORMAL];

		if (!PG_RVSD(start)) {
			zone->free_pages++;
		}

		zone->spanned_pages++;
		page_setzone(start, zone - zones);
	}

	for (zone = zones; zone < zones + ZONE_NR - 1; zone++) {
		printf("zone: %s, %d, %d\n", zone->name, zone->spanned_pages, zone->free_pages);
	}

//	printf("page flags %02X\n", mem_map[ZONE_DMA_SIZE >> PAGE_SHIFT].flags);
}

static void mm_reserved_init(void)
{
	struct page *end = virt_to_page(free_mem_start);
	struct page *start = mem_map;

	if (!end) {
		BUG();
	}

	for (; start != end; start++) {
		memset(start, 0, sizeof(struct page));
		atmoic_inc(&start->count);
		PG_SETRVSD(start);
	}
}

static void mm_test(void);

void mm_init(void)
{
	uint32_t pages = (mem_size_kbytes + 1024) >> 2;
	uint32_t pgtbl_size = pages * sizeof(unsigned long);

	free_mem_start = PAGE_ALIGN(((unsigned long)pg + PGDIR_SIZE + pgtbl_size));

	mem_map = (struct page *)free_mem_start;
	free_mem_start = PAGE_ALIGN(free_mem_start + pages * sizeof(struct page));

	free_page_start = virt_to_page(free_mem_start);
	total_pages = pages;

	mm_reserved_init();
	zone_init();
	page_init(mem_map, pages);

	free_area_init();

	printf("mem_map %X\n", mem_map);
	printf("free_mem_start %X\n", free_mem_start);
	printf("free mem %d bytes\n", (total_pages - (free_page_start - mem_map)) * PAGE_SIZE);

	mm_test();
}

static void free_area_stats(struct zone *zone)
{
	int order = MAX_ORDER;

	printf("%s: free area: ", zone->name);

	while (order >= 0) {
		printf("%d", zone->free_areas[order].nr_free);
		order--;
	}

	printf("\n");
}

static inline void remove_page(struct page *page, int order)
{
	list_del(&page->lru);
	page_zone(page)->free_areas[order].nr_free--;
}

static struct page * rmqueue(struct zone *zone, int order)
{
	struct free_area *area;
	struct page *page;
	uint32_t curr_order = order;

	for (; curr_order <= MAX_ORDER; curr_order++) {
		area = zone->free_areas + curr_order;
		if (area->nr_free != 0)
			goto find_blk;
	} 

	return NULL;

find_blk:
	page = list_entry(area->free_list.next, struct page, lru);
	list_del(&page->lru);
	area->nr_free--;

	while (curr_order > order) {
		struct page *buddy;

		area--;
		curr_order--;
		buddy = page + (1 << curr_order);
		list_add(&buddy->lru, &area->free_list);
		area->nr_free++;
	}

	return page;
}

static int prep_new_page(struct page *page, int order, unsigned long gfp_mask)
{
	set_page_refcounted(page);
	return 0;
}

/* 
 * TODO:
 * gfp_mask is nouse now.
 */
struct page * alloc_pages(unsigned long gfp_mask, int order)
{
	struct page *page;
	struct zone **zone;

	if (order > MAX_ORDER || order < 0)
		return NULL;

	zone = zonelist.zones;
	while (*zone) {
		page = rmqueue(*zone, order);
		if (page) {
			prep_new_page(page, order, gfp_mask);
			return page;
		}

		zone++;
	}

	return NULL;
}

/*
 * Locate the struct page for both the matching buddy in our
 * pair (buddy1) and the combined O(n+1) page they form (page).
 *
 * 1) Any buddy B1 will have an order O twin B2 which satisfies
 * the following equation:
 *     B2 = B1 ^ (1 << O)
 * For example, if the starting buddy (buddy2) is #8 its order
 * 1 buddy is #10:
 *     B2 = 8 ^ (1 << 1) = 8 ^ 2 = 10
 *
 * 2) Any buddy B will have an order O+1 parent P which
 * satisfies the following equation:
 *     P = B & ~(1 << O)
 *
 * Assumption: *_mem_map is contiguous at least up to MAX_ORDER
 */
static inline struct page *
page_find_buddy(struct page *page, unsigned long page_idx, unsigned int order)
{
	unsigned long buddy_idx = page_idx ^ (1 << order);

	return page + (buddy_idx - page_idx);
}

static int page_is_buddy(struct page *page, int order)
{
	if (page_count(page) == 0 && !PG_RVSD(page))
		return 1;

	return 0;
}

void __free_pages(struct page * page, int order)
{
	struct zone *zone = page_zone(page);
	struct free_area *area;
	unsigned long page_idx;

	while (order < MAX_ORDER) {
		struct page *buddy;

		page_idx = page_to_pfn(page) & ((1 << MAX_ORDER) - 1);
		buddy = page_find_buddy(page, page_idx, order);
//		area = zone->free_areas + order;

		if (!page_is_buddy(buddy, order))
			break;

		list_del(&page->lru);
		zone->free_areas[order].nr_free--;

		if (buddy < page)
			page = buddy;

		order++;
	}

	list_add(&page->lru, &zone->free_areas[order].free_list);
	set_page_count(page, 0);
	zone->free_areas[order].nr_free++;
}

static void mm_test(void)
{
	struct page *page;

	free_area_stats(&zones[ZONE_NORMAL]);
	page = alloc_pages(0, 0);
	printf("alloc page %p, %p\n", page, page_address(page));
	free_area_stats(&zones[ZONE_NORMAL]);
	__free_pages(page, 0);
	free_area_stats(&zones[ZONE_NORMAL]);

#if 0
	page = alloc_pages(0, 0);
	printf("alloc page %p, %p\n", page, page_address(page));

	free_area_stats(page_zone(page));

	__free_pages(page, 0);
#endif
}
