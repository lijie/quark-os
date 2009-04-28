#ifndef	__IRQ_H__
#define	__IRQ_H__

#define	__USE_BIOS_IRQ__

extern void intr_init(void);
extern void irq_enable(int irqno);
extern void irq_disable(int irqno);

#ifdef	__USE_BIOS_IRQ__
/* 
 * irq vector when system is starting
 * IRQ0		0x08
 * IRQ1		0x09
 * IRQ2		0x0A
 * IRQ3		0x0B
 * IRQ4		0x0C
 * IRQ5		0x0D
 * IRQ6		0x0E
 * IRQ7		0x0F
 * IRQ8		0x70
 * IRQ9		0x71
 * IRQ10	0x72
 * IRQ11	0x73
 * IRQ12	0x74
 * IRQ13	0x75
 * IRQ14	0x76
 * IRQ15	0x77
 */

#define	IRQ(n)		((n) < 8 ? (n) + 0x08 : (n) + (0x70 - 8))
#else
#error	"we don't define irq vectors right now!\n"
#endif

/* irq handlers */
extern void time_interrupt(void);
extern void realtime_interrupt(void);
extern void kdb_interrupt(void);
extern void serial1_interrupt(void);
extern void serial2_interrupt(void);
extern void parallel1_interrupt(void);
extern void parallel2_interrupt(void);
extern void ps2_interrupt(void);
#endif

