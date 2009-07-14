#ifndef	__CACHE_H__
#define	__CACHE_H__

#define	CONFIG_X86_L1_CACHE_SHIFT	5

#define	cache_line_shift		CONFIG_X86_L1_CACHE_SHIFT
#define	cache_line_size()		(1 << cache_line_shift)

#endif
