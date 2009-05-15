#include "stdio.h"
#include "string.h"
#include "config.h"
#include "i386.h"
#include "io.h"

struct boot_params {
	unsigned long p_count;
	unsigned long p_memsize;
};

static inline void disable_NMI(void)
{
	outb_p(0x80, 0x70);
}

static inline void enable_A20(void)
{
	uint8_t port;

	port = inb(0x92) | 0x02;
	outb_p(port, 0x92);
}

static inline void mask_all_ints(void)
{
	outb_p(0xFF, 0xA1);
	outb_p(0xFB, 0x21);
}

extern unsigned char _text[];
extern unsigned char _end[];
static inline void move_kernel_code(unsigned long from)
{
	unsigned int size = (int)_end - (int)_text;

	/* copy code from current to _text - __PAGE_OFFSET */
	memcpy((void *)_text - __PAGE_OFFSET, (void *)from, size);
}

void default_page_map(void)
{
	unsigned long *pde = (unsigned long *)pg;
	unsigned long *pte;
	unsigned int i, j;
	unsigned long attr = 0x07;

	/* offset __PAGE_OFFSET */
	pde = pde - (__PAGE_OFFSET >> 2);

	/* pde needs 4K */
	pte = pde + 1024;

	/* mapping 8M size */
	j = 0;
	for (i = 0; i < (0x800000 >> PDE_SHIFT); i++) {
		pde[i] = (unsigned long)(pte + i * 1024) + attr;
//		pde[i] = (unsigned long)pte;
		pde[(__PAGE_OFFSET / (1 << PDE_SHIFT)) + i] = pde[i];
//		pde[0x300 + i] = pde[i];
		for (;j < 2048; j++) {
			pte[j] = PAGE_SIZE * j + attr;
		}
	}

	/* enable */
	__asm__ __volatile__ (				\
		"movl %%eax, %%cr3\n\t"			\
		"movl %%cr0, %%ebx\n\t"			\
		"orl $0x80000000, %%ebx\n\t"		\
		"movl %%ebx, %%cr0\n\t"			\
		:					\
		: "a"(pg - __PAGE_OFFSET)		\
		: "ebx"					\
		);
}

void boot_main(unsigned long from)
{
	disable_NMI();
	enable_A20();
	mask_all_ints();
	move_kernel_code(from);

	default_page_map();
}
