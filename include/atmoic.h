#ifndef	__ATMOIC_H__
#define	__ATMOIC_H__

/* FIXME: not implemented! */

typedef unsigned int atmoic_t;

static inline void atmoic_inc(atmoic_t *t)
{
	(*t)++;
}

static inline void atmoic_dec(atmoic_t *t)
{
	(*t)--;
}

#endif
