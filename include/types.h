#ifndef	__TYPES_H__
#define	__TYPES_H__

typedef	char		int8_t;
typedef	short		int16_t;
typedef	int		int32_t;

typedef	unsigned char	u8_t;
typedef	unsigned short	u16_t;
typedef	unsigned int	u32_t;

typedef	char		s8_t;
typedef	short		s16_t;
typedef	int		s32_t;

typedef	unsigned char	uint8_t;
typedef	unsigned short	uint16_t;
typedef	unsigned int	uint32_t;

typedef	unsigned int	size_t;
typedef	int		ssize_t;

typedef	long int	long_t;
typedef	unsigned long	ulong_t;

typedef	unsigned int	gfp_t;

#if 1
typedef char *va_list;
#define va_arg(list, type)	((type *)(list += sizeof(type)))[-1]
#define va_end(list)
#define va_start(list, last_arg)	(list = (va_list)(&last_arg + 1))
#endif

#define	NULL		((void *)0)

#define	ULONG_MAX	(~((unsigned long)0))
#endif
