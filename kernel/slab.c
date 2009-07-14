#include "system.h"
#include "list.h"
#include "mm.h"

typedef unsigned int kmem_bufctl_t;
#define BUFCTL_END	(((kmem_bufctl_t)(~0U))-0)
#define BUFCTL_FREE	(((kmem_bufctl_t)(~0U))-1)
#define	BUFCTL_ACTIVE	(((kmem_bufctl_t)(~0U))-2)
#define	SLAB_LIMIT	(((kmem_bufctl_t)(~0U))-3)

struct array_cache {
	unsigned int avail;
	unsigned int limit;
	unsigned int batchcount;
	unsigned int touched;
	void *entry[];	/*
			 * Must have this definition in here for the proper
			 * alignment of array_cache. Also simplifies accessing
			 * the entries.
			 */
};

/*
 * struct slab
 *
 * Manages the objs in a slab. Placed either at the beginning of mem allocated
 * for a slab, or allocated from an general cache.
 * Slabs are chained into three list: fully used, partial, fully free slabs.
 */
struct slab {
	struct list_head list;
	unsigned long colouroff;
	void *s_mem;		/* including colour offset */
	unsigned int inuse;	/* num of objs active in slab */
	kmem_bufctl_t free;	/* next free obj */
	unsigned short nodeid;
};

/*
 * The slab lists for all objects.
 */
struct kmem_list3 {
	struct list_head slabs_partial;	/* partial list first, better asm code */
	struct list_head slabs_full;
	struct list_head slabs_free;
	unsigned long free_objects;
	unsigned int free_limit;
	unsigned int colour_next;	/* Per-node cache coloring */

	struct array_cache *shared;	/* shared per node */
	struct array_cache **alien;	/* on other nodes */
	unsigned long next_reap;	/* updated without locking */
	int free_touched;		/* updated without locking */
};

struct kmem_cache {
	struct array_cache *array;
	unsigned int batchcount;
	unsigned int limit;
	unsigned int flags;		/* constant flags */
	unsigned int num;		/* # of objs per slab */
	
	unsigned int gfporder;		/* order of pgs per slab (2^n) */
	unsigned long gfpflags;

	size_t colour;			/* cache colouring range */
	unsigned int colour_off;	/* colour offset */

	unsigned int buffer_size; 	/* size of per obj */
	uint32_t reciprocal_buffer_size; /* ?? */
	struct kmem_cache *slabp_cache;	 /* ?? */
	unsigned int slab_size;

	/* constructor func */
	void (*ctor)(void *obj);
	struct list_head next;
	struct kmem_list3 *nodelist;
};

struct cache_sizes {
	size_t		 	cs_size;
	struct kmem_cache	*cs_cachep;
};

#define BOOT_CPUCACHE_ENTRIES	1
#define	BATCHREFILL_LIMIT	16

#define CFLGS_OFF_SLAB		(0x80000000UL)
#define	OFF_SLAB(x)		((x)->flags & CFLGS_OFF_SLAB)

struct cache_sizes malloc_sizes[] = {
#define CACHE(x) { .cs_size = (x) },
#include "kmalloc_sizes.h"
	CACHE(ULONG_MAX)
#undef CACHE
};

static void print_kmem_cache(struct kmem_cache *cachep, const char *str)
{
}

static struct kmem_cache cache_cache = {
	.batchcount = 1,
	.buffer_size = sizeof(struct kmem_cache),
};

static struct kmem_list3 initkmem_list3;
static struct array_cache initarray_cache = {
	.avail = 0,
	.limit = BOOT_CPUCACHE_ENTRIES,
	.batchcount = 1,
	.touched = 0,
};

static inline void *index_to_obj(struct kmem_cache *cache, struct slab *slab,
				 unsigned int idx)
{
	return slab->s_mem + cache->buffer_size * idx;
}

static inline kmem_bufctl_t *slab_bufctl(struct slab *slabp)
{
	return (kmem_bufctl_t *) (slabp + 1);
}

static void * kmem_getpages(struct kmem_cache *cachep, unsigned long flags)
{
	struct page *page;

	printf("%s: order: %d\n", __func__, cachep->gfporder);
	page = alloc_pages(flags, cachep->gfporder);
	if (!page) {
		printf("%s: alloc pages failed!\n", __func__);
		return NULL;
	}

	return page_address(page);
}

static size_t slab_mgmt_size(size_t nr_objs, size_t align)
{
	return ALIGN(sizeof(struct slab) + nr_objs * sizeof(kmem_bufctl_t), align);
}

static void cache_estimate(int order, size_t buffer_size, size_t align, 
			   int flags, size_t *left_over, unsigned int *num)
{
	size_t slab_size = PAGE_SIZE << order;
	size_t mgmt_size;
	unsigned int nr_objs;

	if (flags & CFLGS_OFF_SLAB) {
		// TODO
	} else {
		nr_objs = (slab_size - sizeof(struct slab)) / (buffer_size + sizeof(kmem_bufctl_t));

		mgmt_size = slab_mgmt_size(nr_objs, align);
		if ((mgmt_size + (nr_objs * buffer_size)) > slab_size) {
			nr_objs--;
			mgmt_size = slab_mgmt_size(nr_objs, align);
		}

		printf("%s: mgmt_size %d, align %d\n", __func__, mgmt_size, align);
	}

	if (nr_objs > SLAB_LIMIT)
		nr_objs = SLAB_LIMIT;

	*num = nr_objs;
	*left_over = slab_size - nr_objs * buffer_size - mgmt_size;
}

static void kmem_list3_init(struct kmem_list3 *parent)
{
	INIT_LIST_HEAD(&parent->slabs_full);
	INIT_LIST_HEAD(&parent->slabs_partial);
	INIT_LIST_HEAD(&parent->slabs_free);
	parent->shared = NULL;
	parent->alien = NULL;
	parent->colour_next = 0;
//	spin_lock_init(&parent->list_lock);
	parent->free_objects = 0;
	parent->free_touched = 0;
}

static struct slab *alloc_slabmgmt(struct kmem_cache *cachep, void *objp,
				   int colour_off, gfp_t local_flags, int nodeid)
{
	struct slab *slabp;

	if (OFF_SLAB(cachep)) {
		/* TODO */
		BUG();
	} else {
		slabp = objp + colour_off;
		colour_off += cachep->slab_size;
	}

	slabp->inuse = 0;
	slabp->colouroff = colour_off;
	slabp->s_mem = objp + colour_off;
	slabp->nodeid = nodeid;
	slabp->free = 0;
	return slabp;
}

static void cache_init_objs(struct kmem_cache *cachep,
			    struct slab *slabp)
{
	int i;

	for (i = 0; i < cachep->num; i++) {
		void *obj = index_to_obj(cachep, slabp, i);
		if (cachep->ctor)
			cachep->ctor(obj);
		slab_bufctl(slabp)[i] = i + 1;
	}

	slab_bufctl(slabp)[i - 1] = BUFCTL_END;
}

static void slab_map_pages(struct kmem_cache *cache, struct slab *slab,
			   void *addr)
{
}

/*
 * Grow (by 1) the number of slabs within a cache.  This is called by
 * kmem_cache_alloc() when there are no active objs left in a cache.
 */
static int cache_grow(struct kmem_cache *cachep,
		      gfp_t flags, int nodeid, void *objp)
{
	struct kmem_list3 *l3 = cachep->nodelist;
	struct slab *slabp;
	unsigned int offset;
	gfp_t local_flags;

	/* Get colour for the slab, and cal the next value. */
	offset = l3->colour_next;
	l3->colour_next++;
	if (l3->colour_next >= cachep->colour)
		l3->colour_next = 0;

	/* 
	 * colour_off means X86_L1_CACHE_SIZE, 2^5 bytes
	 */
	offset *= cachep->colour_off;
	if (!objp)
		objp = kmem_getpages(cachep, flags);
	if (!objp)
		BUG();

	printf("%s: kmem_getpages return %08X\n", __func__, (int)objp);
#if 0
	pgd_info((unsigned long)objp);
	pgd_info(0xC0000000);
	pgd_info(0xC0100000);
	pgd_info(0xC0200000);
	pgd_info(0xC0300000);
	pgd_info(0xC0400000);
	pgd_info(0xC0500000);
	pgd_info(0xC0600000);
	pgd_info(0xC0700000);
#endif
	/* TODO... */
	slabp = alloc_slabmgmt(cachep, objp, offset,
			       local_flags, nodeid);

	cache_init_objs(cachep, slabp);
	list_add_tail(&slabp->list, &(l3->slabs_free));
	l3->free_objects += cachep->num;
	return 0;
}

struct kmem_cache * kmem_cache_create(const char *name, size_t size, size_t align,
				      unsigned long flags, void (*ctor)(void *))
{
	struct kmem_cache *cachep;

	/* 1. caculate align */

	/* 2. alloc cachep */
	cachep = kmem_cache_zalloc(&cache_cache, 0 /* GFP_KERNEL*/);
	return cachep;
}

static void *slab_get_obj(struct kmem_cache *cachep, 
			  struct slab *slabp, int nodeid)
{
	void *objp = index_to_obj(cachep, slabp, slabp->free);
	kmem_bufctl_t next;

	slabp->inuse++;
	next = slab_bufctl(slabp)[slabp->free];
	slabp->free = next;

	return objp;
}

static void *cache_alloc_refill(struct kmem_cache *cachep, gfp_t flags)
{
	struct array_cache *ac = cachep->array;
	struct kmem_list3 *l3 = cachep->nodelist;
	int batchcount;

	printf("%s, %d\n", __func__, cachep->buffer_size);

retry:
	batchcount = ac->batchcount;
	if (!ac->touched && batchcount > BATCHREFILL_LIMIT) {
		/*
		 * If there was little recent activity on this cache, then
		 * perform only a partial refill.  Otherwise we could generate
		 * refill bouncing.
		 */
		batchcount = BATCHREFILL_LIMIT;
	}

	while (batchcount > 0) {
		struct list_head *entry = l3->slabs_partial.next;
		struct slab *slabp;

		if (entry == &l3->slabs_partial) {
			/* no free objs in partial list */
			l3->free_touched = 1;
			entry = l3->slabs_free.next;
			if (entry == &l3->slabs_free)
				/* no free objs... */
				goto must_grow;
		}

		/* find a slab */
		slabp = list_entry(entry, struct slab, list);
		while (slabp->inuse < cachep->num && batchcount--) {
//			STATS_INC_ALLOCED(cachep);
//			STATS_INC_ACTIVE(cachep);
//			STATS_SET_HIGH(cachep);

			ac->entry[ac->avail++] = 
				slab_get_obj(cachep, slabp, 0);
		}

		list_del(&slabp->list);
		if (slabp->free == BUFCTL_END)
			list_add(&slabp->list, &l3->slabs_full);
		else
			list_add(&slabp->list, &l3->slabs_partial);
	}

must_grow:	
	if (!ac->avail) {
		if (cache_grow(cachep, flags, 0, NULL)) {
			printf("%s: cache_grow failed! no mem!\n", __func__);
			return NULL;
		}

		goto retry;
	}

	ac->touched = 1;
	return ac->entry[--ac->avail];
}

void *kmem_cache_alloc(struct kmem_cache *cachep, gfp_t flags)
{
	void *ret = NULL;
	struct array_cache *ac = cachep->array;

	if (ac->avail)
		ret = ac->entry[--ac->avail];
	else {
		/* TODO ... */
		ret = cache_alloc_refill(cachep, flags);
	}

	if ((flags & __GFP_ZERO) && ret)
		memset(ret, 0, cachep->buffer_size);

	return ret;
}

int kmem_cache_init(void)
{
	struct cache_sizes *cs = malloc_sizes;
	int i, order;
	unsigned int left_over;

	for (i = 0; i < 3; i++) {
		kmem_list3_init(&initkmem_list3);
	}

	cache_cache.nodelist = &initkmem_list3;
	cache_cache.array = &initarray_cache;

	cache_cache.colour_off = cache_line_size();

	/* init cache_cache */
	for (order = 0; order < MAX_ORDER; order++) {
		cache_estimate(order, cache_cache.buffer_size,
			cache_line_size(), 0, &left_over, &cache_cache.num);
		if (cache_cache.num)
			break;
	}

	printf("cache_cache order %d, num %d\n", order, cache_cache.num);
	cache_cache.slab_size = 
		ALIGN(cache_cache.num * sizeof(kmem_bufctl_t) + 
		      sizeof(struct slab), cache_line_size());

	printf("cache_cache slab_size %d\n", cache_cache.slab_size);

	/* init kmalloc */
	/* TODO... */
	while (cs->cs_size != ULONG_MAX) {
		cs->cs_cachep = kmem_cache_create("kmalloc", cs->cs_size, 
						  cache_line_size(), 0, NULL);
		
		break;
	}

	return 0;
}
