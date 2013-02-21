#ifndef _BT_STRUCT_H_
#define _BT_STRUCT_H_

#include <stddef.h>

/**
 *	@brief		A special macro to cast a member of a structure out to the containing structure
 *
 *	@ptr:       the pointer to the member.
 * 	@type:   	the type of the container struct this is embedded in.
 * 	@member:    the name of the member within the struct.
 *	@t_member:	the type of the member -- linux uses typeof() but most compilers dont have this.
 *
 *	@citation:	Linux kernel source-code.
 */
#define bt_container_of(ptr, type, member, t_member) ((type *) (((char *) ((t_member *) (ptr))) - offsetof(type,member)))


/**
 *	Calculates the number of items within an array!
 *
 **/
#define BT_ARRAY_SIZE(array)	(sizeof(array) / sizeof((array[0])))

#define BT_SIZE_1K		(1024)
#define BT_SIZE_4K		(BT_SIZE_1K * 4)

/*
 *	@id		The reserved member number, e.g. reserved_1 ....
 *	@begin	Address that the field begins at, i.e. the last address of the previous field.
 *	@end	The address that the next field begins at.
 *
 *
 *
 */
#define BT_STRUCT_RESERVED_u32(id, begin, end)	BT_u32 reserved_##id[((end-begin)/4)-1]

/*
 *	@begin	The address that the array begins.
 *	@end	The address that the array ends... not including the beginning of the next field!
 */
#define BT_STRUCT_ARRAY_u32(begin, end)	(((end-begin)/4)+1)











#endif
