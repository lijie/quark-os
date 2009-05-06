#ifndef	__TIME_H__
#define	__TIME_H__

#define	HZ			100
#define	LATCH			(1193180 / HZ)

extern unsigned long volatile jiffies;

/* return the seconds from system boot */
extern uint32_t ktime(void);
#endif
