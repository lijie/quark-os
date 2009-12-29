#include "string.h"
#include "system.h"
#include "list.h"
#include "mm.h"

#define	slab_dbg(lv, fmt, ...)						\
	do {								\
		if (lv)							\
			printf("[%s,%d]:" fmt, __func__, __LINE__, __VA_ARGS__); \
	} while (0)

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
 * bootstrap: The caches do not work without cpuarrays anymore, but the
 * cpuarrays are allocated from the generic caches...
 */
#define BOOT_CPUCACHE_ENTRIES	1
struct arraycache_init {
	struct array_cache cache;
	void *entries[BOOT_CPUCACHE_ENTRIES];
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
	struct array_cache *array[NR_CPUS];
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

	const char *name;
	struct list_head next;
	struct kmem_list3 *nodelist;
};

static inline void __kmem_cache_dbg(int lv, struct kmem_cache *cache)
{
	if (lv) {
		printf("name %s order %d buffer_size %d slab_size %d\n", 
		       cache->name, cache->gfporder, cache->buffer_size, cache->slab_size);
	}
}

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

struct cache_names {
	char *name;
	char *name_dma;
};

static struct cache_names cache_names[] = {
#define CACHE(x) { .name = "size-" #x, .name_dma = "size-" #x "(DMA)" },
#include "kmalloc_sizes.h"
	{NULL,}
#undef CACHE
};

static int slab_break_gfp_order = 0;
static int slab_early_init = 1;
static struct list_head cache_chain;

static void print_kmem_cache(struct kmem_cache *cachep, const char *str)
{
}

static struct kmem_cache cache_cache = {
	.batchcount = 1,
	.buffer_size = sizeof(struct kmem_cache),
};

static struct kmem_list3 initkmem_list3;

static struct arraycache_init initarray_cache = {
	{
		.avail = 0,
		.limit = BOOT_CPUCACHE_ENTRIES,
		.batchcount = 1,
		.touched = 0,
	}
};

static struct arraycache_init initarray_generic = {
	{
		.avail = 0,
		.limit = BOOT_CPUCACHE_ENTRIES,
		.batchcount = 1,
		.touched = 0,
	}
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

	slab_dbg(1, "order: %d\n", cachep->gfporder);
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
		BUG();
	} else {
		nr_objs = (slab_size - sizeof(struct slab)) / (buffer_size + sizeof(kmem_bufctl_t));

		mgmt_size = slab_mgmt_size(nr_objs, align);
		if ((mgmt_size + (nr_objs * buffer_size)) > slab_size) {
			nr_objs--;
			mgmt_size = slab_mgmt_size(nr_objs, align);
		}

//		printf("%s: mgmt_size %d, align %d\n", __func__, mgmt_size, align);
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

/**
 * calculate_slab_order - calculate size (page order) of slabs
 * @cachep: pointer to the cache that is being created
 * @size: size of objects to be created in this cache.
 * @align: required alignment for the objects.
 * @flags: slab allocation flags
 *
 * Also calculates the number of objects per slab.
 *
 * This could be made much more intelligent.  For now, try to avoid using
 * high order pages for slabs.  When the gfp() functions are more friendly
 * towards high-order requests, this should be changed.
 */
static size_t calculate_slab_order(struct kmem_cache *cachep,
				   size_t size, size_t align, unsigned long flags)
{
	unsigned long offslab_limit;
	size_t left_over = 0;
	int gfporder;

	for (gfporder = 0; gfporder <= MAX_ORDER; gfporder++) {
		unsigned int num;
		size_t remainder;

		cache_estimate(gfporder, size, align, flags, &remainder, &num);
		if (!num) {
			slab_dbg(0, "order %d is too small, %d\n", gfporder, num);
			continue;
		}

		if (flags & CFLGS_OFF_SLAB) {
			/*
			 * Max number of objs-per-slab for caches which
			 * use off-slab slabs. Needed to avoid a possible
			 * looping condition in cache_grow().
			 */
			offslab_limit = size - sizeof(struct slab);
			offslab_limit /= sizeof(kmem_bufctl_t);

 			if (num > offslab_limit)
				break;
		}

		/* Found something acceptable - save it away */
		cachep->num = num;
		cachep->gfporder = gfporder;
		left_over = remainder;

#if 0
		/*
		 * A VFS-reclaimable slab tends to have most allocations
		 * as GFP_NOFS and we really don't want to have to be allocating
		 * higher-order pages when we are unable to shrink dcache.
		 */
		if (flags & SLAB_RECLAIM_ACCOUNT)
			break;

#endif
		/*
		 * Large number of objects is good, but very large slabs are
		 * currently bad for the gfp()s.
		 */
		if (gfporder >= slab_break_gfp_order)
			break;

		/*
		 * Acceptable internal fragmentation?
		 */
		if (left_over * 8 <= (PAGE_SIZE << gfporder))
			break;
	}

	return left_over;
}

static void __kmem_cache_destroy(struct kmem_cache *cachep)
{
}

/*
 * chicken and egg problem: delay the per-cpu array allocation
 * until the general caches are up.
 */
static enum {
	NONE,
	PARTIAL_AC,
	PARTIAL_L3,
	EARLY,
	FULL
} g_cpucache_up;

static inline struct array_cache * cpu_cache_get(struct kmem_cache *cachep)
{
	return cachep->array[smp_processor_id()];
}

/* ??? */
static void set_up_list3s(struct kmem_cache *cachep, int index)
{
}

static int setup_cpu_cache(struct kmem_cache *cachep, gfp_t gfp)
{
	if (g_cpucache_up == NONE) {
		cachep->array[smp_processor_id()] = &initarray_generic.cache;
		g_cpucache_up = PARTIAL_AC;
	}
	else {
		printf("kmalloc cpu cache.\n");
		cachep->array[smp_processor_id()] = 
			kmalloc(sizeof(struct arraycache_init), gfp);
	}
}

static int enable_cpucache(struct kmem_cache *cachep, gfp_t gfp)
{
}

struct kmem_cache * kmem_cache_create(const char *name, size_t size, size_t align,
				      unsigned long flags, void (*ctor)(void *))
{
	struct kmem_cache *cachep;
	size_t left_over;
	size_t slab_size;
	gfp_t gfp = 0;

	/* 1. caculate align */
	/* TODO */

	/* 2. alloc cachep */
	cachep = kmem_cache_zalloc(&cache_cache, 0 /* GFP_KERNEL*/);

	if (!cachep)
		goto oops;

	/*
	 * Determine if the slab management is 'on' or 'off' slab.
	 * (bootstrapping cannot cope with offslab caches so don't do
	 * it too early on.)
	 */
	if ((size >= (PAGE_SIZE >> 3)) && !slab_early_init)
		/*
		 * Size is large, assume best to place the slab management obj
		 * off-slab (should allow better packing of objs).
		 */
		flags |= CFLGS_OFF_SLAB;

	size = ALIGN(size, align);
	left_over = calculate_slab_order(cachep, size, align, flags);

	if (!cachep->num) {
		slab_dbg(1, "kmem_cache_create: couldn't create cache %s.\n", name);
		kmem_cache_free(&cache_cache, cachep);
		cachep = NULL;
		goto oops;
	}

	slab_size = ALIGN(cachep->num * sizeof(kmem_bufctl_t)
			  + sizeof(struct slab), align);

	if (flags & CFLGS_OFF_SLAB) {
		/* TODO */
		BUG();
	}

	cachep->colour_off = cache_line_size();
	/* Offset must be a multiple of the alignment. */
	if (cachep->colour_off < align)
		cachep->colour_off = align;

	/*
	 * UNKNOWN
	 */
	cachep->colour = left_over / cachep->colour_off;
	cachep->slab_size = slab_size;
	cachep->flags = flags;
	cachep->gfpflags = 0;

	cachep->buffer_size = size;

	/* 
	 * UNKNOWN
	 */
//	cachep->reciprocal_buffer_size = reciprocal_value(size);

	/*
	 * If the slab has been placed off-slab, and we have enough space then
	 * move it on-slab. This is at the expense of any extra colouring.
	 */
	if (flags & CFLGS_OFF_SLAB && left_over >= slab_size) {
		flags &= ~CFLGS_OFF_SLAB;
		left_over -= slab_size;
	}

	cachep->ctor = ctor;
	cachep->name = name;

	if (setup_cpu_cache(cachep, gfp)) {
		__kmem_cache_destroy(cachep);
		cachep = NULL;
		goto oops;
	}

	/* cache setup completed, link it into the list */
	list_add(&cachep->next, &cache_chain);

	return cachep;

oops:
	return NULL;
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
	struct array_cache *ac;
	struct kmem_list3 *l3 = cachep->nodelist;
	int batchcount;

	slab_dbg(1, "cachep %X, buffer_size %d\n", (int)cachep, cachep->buffer_size);

retry:
	ac = cpu_cache_get(cachep);
	batchcount = ac->batchcount;
	slab_dbg(1, "batchcount %d\n", batchcount);
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
		printf("We have slab %X %X %X!\n", (int)slabp, (int)l3, (int)entry);

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
	struct array_cache *ac;

	ac = cpu_cache_get(cachep);

	slab_dbg(1, "avail %d\n", ac->avail);

	if (ac->avail)
		ret = ac->entry[--ac->avail];
	else {
		/* TODO ... */
		ret = cache_alloc_refill(cachep, flags);
	}

	if ((flags & __GFP_ZERO) && ret)
		memset(ret, 0, cachep->buffer_size);

	slab_dbg(1, "obj %X\n", ret);
	return ret;
}

void kmem_cache_free(struct kmem_cache *cachep, void *objp)
{
	/* TODO */
}

void * kmalloc(size_t size, gfp_t flags)
{
	struct cache_sizes *cs = malloc_sizes;
	struct kmem_cache *cache;

	while (size > cs->cs_size)
		cs++;

	cache = cs->cs_cachep;
	if (!cache)
		return NULL;

	slab_dbg(1, "find general size %X, %d\n", (int)cache, cs->cs_size);
	return kmem_cache_alloc(cache, flags);
}

int kmem_cache_init(void)
{
	struct cache_sizes *cs = malloc_sizes;
	struct cache_names *names = cache_names;
	int i, order;
	unsigned int left_over;

	for (i = 0; i < 3; i++) {
		kmem_list3_init(&initkmem_list3);
	}

	cache_cache.nodelist = &initkmem_list3;
	cache_cache.array[smp_processor_id()] = &initarray_cache.cache;

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

	INIT_LIST_HEAD(&cache_chain);
	list_add(&cache_cache.next, &cache_chain);

	/* init kmalloc */
	/* TODO... */
	while (cs->cs_size != ULONG_MAX) {
		cs->cs_cachep = kmem_cache_create(names->name, cs->cs_size, 
						  cache_line_size(), 0, NULL);

		__kmem_cache_dbg(1, cs->cs_cachep);
		cs++;
		names++;
//		break;
	}

	return 0;
}
