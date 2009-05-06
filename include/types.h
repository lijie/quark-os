#ifndef	__TYPES_H__
#define	__TYPES_H__

typedef	char	int8_t;
typedef	short	int16_t;
typedef	int	int32_t;

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

#if 1
typedef char *va_list;
#define va_arg(list, type)	((type *)(list += sizeof(type)))[-1]
#define va_end(list)
#define va_start(list, last_arg)	(list = (va_list)(&last_arg + 1))
#endif

#if 0
typedef __builtin_va_list __gnuc_va_list;
#define va_start(v,l)	__builtin_va_start(v,l)
#define va_end(v)	__builtin_va_end(v)
#define va_arg(v,l)	__builtin_va_arg(v,l)
typedef __gnuc_va_list va_list;
#endif

#define	NULL		((void *)0)
#endif
