#ifndef	__I386_H__
#define	__I386_H__

#include "types.h"
#include "stdio.h"
#include "bios.h"
#include "i386_debug.h"
/* 
 * selectors of descriptor
 * descriptors are defined in startup.S
 */

/* index */
#define	KERNEL_CODE	1
#define	KERNEL_DATA	2
#define	TSS		16
#define	LDT		17

#define	BOOT_CS		KERNEL_CODE
#define	BOOT_DS		KERNEL_DATA

/* selector */
#define	SEG_SELECTOR(index)	((index) << 3)
#define	KERNEL_CODE_SELECTOR	SEG_SELECTOR(KERNEL_CODE)
#define	KERNEL_DATA_SELECTOR	SEG_SELECTOR(KERNEL_DATA)
#define	TSS_SELECTOR		SEG_SELECTOR(TSS)
#define	LDT_SELECTOR		SEG_SELECTOR(LDT)

#define	BOOT_CS_SELECTOR	KERNEL_CODE_SELECTOR
#define	BOOT_DS_SELECTOR	KERNEL_DATA_SELECTOR

#define	DA_32			0x4000
#define	DA_LIMIT_4K		0x8000
/* Descriptor privilege level: used by protection mechanism */
#define	DA_DPL0			0x00 /* kernel */
#define	DA_DPL1			0x20  
#define	DA_DPL2			0x40  
#define	DA_DPL3			0x60  

#define	DA_DR			0x90  
#define	DA_DRW			0x92  
#define	DA_DRWA			0x93  
#define	DA_C			0x98  
#define	DA_CR			0x9A  
#define	DA_CCO			0x9C  
#define	DA_CCOR			0x9E  

#define	DA_LDT			0x82  
#define	DA_TaskGate		0x85  
#define	DA_386TSS		0x89  
#define	DA_386CGate		0x8C  
#define	DA_386IGate		0x8E  
#define	DA_386TGate		0x8F  

/* Request privilege level: used by protection mechanism */
#define	SA_RPL_MASK		0xFFFC
#define	SA_RPL0			0 /* kernel */
#define	SA_RPL1			1
#define	SA_RPL2			2
#define	SA_RPL3			3

/* Table indicator: specifies to which descriptor table the selector refers */
#define	SA_TI_MASK		0xFFFB
#define	SA_TIG			0 /* gdt */
#define	SA_TIL			4 /* ldt */

#define	PRIVILEGE_KRNL	0
#define	PRIVILEGE_TASK	1
#define	PRIVILEGE_USER	3
/* RPL */
#define	RPL_KRNL	SA_RPL0
#define	RPL_TASK	SA_RPL1
#define	RPL_USER	SA_RPL3

#define	INT_VECTOR_DIVIDE		0x0
#define	INT_VECTOR_DEBUG		0x1
#define	INT_VECTOR_NMI			0x2
#define	INT_VECTOR_BREAKPOINT		0x3
#define	INT_VECTOR_OVERFLOW		0x4
#define	INT_VECTOR_BOUNDS		0x5
#define	INT_VECTOR_INVAL_OP		0x6
#define	INT_VECTOR_COPROC_NOT		0x7
#define	INT_VECTOR_DOUBLE_FAULT		0x8
#define	INT_VECTOR_COPROC_SEG		0x9
#define	INT_VECTOR_INVAL_TSS		0xA
#define	INT_VECTOR_SEG_NOT		0xB
#define	INT_VECTOR_STACK_FAULT		0xC
#define	INT_VECTOR_PROTECTION		0xD
#define	INT_VECTOR_PAGE_FAULT		0xE
#define	INT_VECTOR_COPROC_ERR		0x10

#define	INT_VECTOR_IRQ0			0x20
#define	INT_VECTOR_IRQ8			0x28

struct regs_struct {
	u32_t eax;
	u32_t ebx;
	u32_t ecx;
	u32_t edx;
	u32_t esp;
	u32_t ebp;
};

struct desc_struct {
	uint16_t	limit_low;	/* Limit */
	uint16_t	base_low;	/* Base */
	uint8_t		base_mid;	/* Base */
	uint8_t		attr1;		/* P(1) DPL(2) DT(1) TYPE(4) */
	uint8_t		limit_high_attr2;	/* G(1) D(1) 0(1) AVL(1) LimitHigh(4) */
	uint8_t		base_high;	/* Base */
};

struct __desc_struct {
	uint32_t l;
	uint32_t h;
};

typedef struct __desc_struct desc_t;

struct idt_desc {
	uint16_t	offset_low;
	uint16_t 	selector;
	uint8_t		unused;
	uint8_t		saved : 3;
	uint8_t		D : 1;
	uint8_t		saved_0 : 1;
	uint8_t		DPL : 2;
	uint8_t		P : 1;
	uint16_t	offset_hig;
};

struct idtr_struct {
	uint16_t	limit;
	uint32_t	base;
} __attribute__((packed));

extern void init_desc(desc_t *desc, uint32_t base, uint32_t limit, uint32_t attr);

#define	sti()		__asm__ __volatile__ ("sti")
#define	cli()		__asm__ __volatile__ ("cli")
#define load_ldt(sel)	__asm__ __volatile__ ("lldt %%ax" : : "a"(sel));
#define load_tss(sel)	__asm__ __volatile__ ("ltr %%ax" : : "a"(sel));

extern struct idt_desc idt[256];

#define	INTR_GATE	14
#define	TRAP_GATE	15
#define	_set_gate(gate, offset, type, dpl)				\
	do {								\
		*(uint32_t *)gate = 0x80000 | (offset & 0x0FFFF);	\
		*((uint32_t *)gate + 1) = (offset & 0xFFFF0000) | 0x8000 | (type << 8) | (dpl << 13); \
	} while (0)

/* extern desc_t idt_table; */
#define	set_trap_gate(n, offset) \
	_set_gate(&idt[n], offset, TRAP_GATE, 0)

#define	set_intr_gate(n, offset) \
	_set_gate(&idt[n], offset, INTR_GATE, 0)

/* some i386 debug functions */
static inline void show_i386_regs(void)
{
	uint32_t reg_cs;
	uint32_t reg_ds;
	uint32_t reg_es;
	uint32_t reg_fs;
	uint32_t reg_gs;
	uint32_t reg_ss;

	asm volatile ("movl %%cs, %0" : "=a"(reg_cs) : );
	asm volatile ("movl %%ds, %0" : "=a"(reg_ds) : );
	asm volatile ("movl %%es, %0" : "=a"(reg_es) : );
	asm volatile ("movl %%fs, %0" : "=a"(reg_fs) : );
	asm volatile ("movl %%gs, %0" : "=a"(reg_gs) : );
	asm volatile ("movl %%ss, %0" : "=a"(reg_ss) : );

	printf("CS %X, DS %X, ES %X, FS %X, GS %X, SS %X\n", 
	       reg_cs, reg_ds, reg_es, reg_fs, reg_gs, reg_ss);
}

extern desc_t gdt[];
extern desc_t ldt[];

#define	PAGE_SIZE	(1 << 12)

#endif
