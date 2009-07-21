#ifndef	__SLAB_H__
#define	__SLAB_H__

#include "types.h"

#define	KMALLOC_MAX_SIZE	131072

struct kmem_cache;

extern int kmem_cache_init(void);
extern void * kmem_cache_alloc(struct kmem_cache *, gfp_t);

static inline void * kmem_cache_zalloc(struct kmem_cache *cachep, gfp_t flags)
{
	return kmem_cache_alloc(cachep, flags | __GFP_ZERO);
}

extern void * kmalloc(size_t size, gfp_t flags);
#endif
