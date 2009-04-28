#ifndef	__TASK_H__
#define	__TASK_H__

#include "config.h"
#include "types.h"
#include "i386.h"

#define	STACK_SIZE	(8 * 1024)

struct tss_struct {
	uint32_t back_link;
	uint32_t esp0;
	uint32_t ss0;
	uint32_t esp1;
	uint32_t ss1;
	uint32_t esp2;
	uint32_t ss2;
	uint32_t cr3;
	uint32_t eip;
	uint32_t eflags;
	uint32_t eax;
	uint32_t ecx;
	uint32_t edx;
	uint32_t ebx;
	uint32_t esp;
	uint32_t ebp;
	uint32_t esi;
	uint32_t edi;
	uint32_t es;
	uint32_t cs;
	uint32_t ss;
	uint32_t ds;
	uint32_t fs;
	uint32_t gs;
	uint32_t ldt;
	uint32_t iobase;
};

struct stack_frame {
	uint32_t gs;
	uint32_t fs;
	uint32_t es;
	uint32_t ds;
	uint32_t edi;
	uint32_t esi;
	uint32_t ebp;
	uint32_t kernel_esp;
	uint32_t ebx;
	uint32_t edx;
	uint32_t ecx;
	uint32_t eax;
	uint32_t retaddr;
	uint32_t eip;
	uint32_t cs;
	uint32_t eflags;
	uint32_t esp;
	uint32_t ss;
};

typedef	uint32_t pid_t;

struct task {
	struct stack_frame	 stack_frame;
	void			*stack_base;
	struct tss_struct	 tss;
	u16_t			 ldt_sel;
	struct desc_struct	 desc;
	struct desc_struct	 ldt0;
	struct desc_struct	 ldt1;
	volatile long		 state;
	struct mm_struct	*mm;
	pid_t			 pid;
	struct regs_struct	 regs;
	uint32_t		 entry;
};

typedef struct task task_t;

union thread_union {
	task_t *task;
	unsigned long stack[STACK_SIZE / sizeof(unsigned long)];
};

static inline union thread_union * current_thread(void)
{
	union thread_union *thread;
	__asm__("andl %%esp,%0; ":"=r" (thread) : "0" (~(STACK_SIZE - 1)));
	return thread;
}
#define	current	(current_thread()->task)

extern union thread_union *init_thread;
extern task_t task0;
extern task_t task1;

static inline void pause(void)
{
	while (1);
}

#define	TSS_INIT(task)	init_desc(&gdt[TSS], (uint32_t)&((task)->tss), sizeof(struct tss_struct) - 1, DA_386TSS);
#define	LDT_INIT(task)	init_desc(&gdt[LDT], (uint32_t)&((task)->ldt0), sizeof((task)->ldt0) * 2 - 1, DA_LDT);

extern void task_init(void);
extern void arch_setup_task(task_t *task, void (*func)(void *));

#endif
