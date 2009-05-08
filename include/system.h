#ifndef	__SYSTEM_H__
#define	__SYSTEM_H__

#define offsetof(TYPE,MEMBER)	__builtin_offsetof(TYPE,MEMBER)

/**
 * container_of - cast a member of a structure out to the containing structure
 * @ptr:        the pointer to the member.
 * @type:       the type of the container struct this is embedded in.
 * @member:     the name of the member within the struct.
 *
 */
#define container_of(ptr, type, member) ({                      \
        const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
        (type *)( (char *)__mptr - offsetof(type,member) );})

#define	BUG() \
	do {								\
		printf("BUG here !! %s %d\n", __func__, __LINE__);	\
		while (1);						\
	} while (0)

#endif
