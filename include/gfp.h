#ifndef	__GFP_H__
#define	__GFP_H__

#define	__GFP_WAIT	(0x10U)
#define	__GFP_IO	(0x40U)
#define	__GFP_FS	(0x80U)
#define	__GFP_ZERO	(0x8000U)

#define	GFP_KERNEL	(__GFP_WAIT | __GFP_IO | __GFP_FS)

#endif
