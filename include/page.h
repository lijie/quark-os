#ifndef	__PAGE_H__
#define	__PAGE_H__

#include "i386.h"

struct page {
	unsigned int  count;
	unsigned long flags;
};

#define	PGSTRUCT_SIZE		sizeof(struct page)

enum {
	PG_USED = 0,

	/* pages which saved kernel code or cannot be used */
	PG_RESERVED,
	PG_ERROR,
};


#define	PG_SET(pg, bit)		(pg->flags |=  (1 << bit))
#define	PG_CLR(pg, bit)		(pg->flags &= ~(1 << bit))
#define	PG_ISSET(pg, bit)	(pg->flags & (1 << bit))

#define	PG_SETUSED(pg)		PG_SET(pg, PG_USED)
#define	PG_CLRUSED(pg)		PG_CLR(pg, PG_USED)
#define	PG_USED(pg)		PG_ISSET(pg, PG_USED)

#define	PG_SETREVERSED(pg)	PG_SET(pg, PG_REVERSED)
#define	PG_CLRREVERSED(pg)	PG_CLR(pg, PG_REVERSED)
#define	PG_REVERSED(pg)		PG_ISSET(pg, PG_REVERSED)

#define	PG_SETERROR(pg)		PG_SET(pg, PG_ERROR)
#define	PG_CLRERROR(pg)		PG_CLR(pg, PG_ERROR)
#define	PG_ERROR(pg)		PG_ISSET(pg, PG_ERROR)

/* 
 * addr where saved page structures
 * defined in lds file
 */
extern struct page * mem_map;

#endif
