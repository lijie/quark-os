#ifndef	__CONFIG_H__
#define	__CONFIG_H__

#define	ARCH_I386
#undef	ARCH_ARM

/* default physical address of kernel image %es:%bx */
#define	CODE_SEG	0x1000
#define	CODE_START	0x0000

/* the physical addrss of kernel stack bottom */
#define	KERNEL_STACK	0x300000

/* the physical address of default page dir and page tbl */
#define	PAGE_DIR	0x200000
#define	PAGE_TBL	0x201000

/* kernel space start */
#define	__PAGE_OFFSET	0xC0000000

/* page */
#define	PAGE_SHIFT		12
#define	PAGE_SIZE		(1 << PAGE_SHIFT)
#define	PAGE_MASK		(PAGE_SIZE - 1)

#define	THREAD_SIZE		(PAGE_SIZE << 1)
#define	STACK_SIZE		THREAD_SIZE
#endif
