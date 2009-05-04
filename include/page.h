#ifndef	__PAGE_H__
#define	__PAGE_H__

struct page {
//	void *virt;
	unsigned long flags;
};

enum {
	PG_USED = 0,
};


#define	PG_SET(pg, bit)		(pg->flags |=  (1 << bit))
#define	PG_CLR(pg, bit)		(pg->flags &= ~(1 << bit))
#define	PG_ISSET(pg, bit)	(pg->flags & (1 << bit))

#define	PG_SETUSED(pg)		PG_SET(pg, PG_USED)
#define	PG_CLRUSED(pg)		PG_CLR(pg, PG_USED)
#define	PG_ISSETUSED(pg)	PG_ISSET(pg, PG_USED)

#endif
