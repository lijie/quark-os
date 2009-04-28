#ifndef	__TIME_H__
#define	__TIME_H__

#define	HZ		100

extern unsigned long volatile jiffies;

/* return the seconds from system boot */
extern uint32_t ktime(void);
#endif
