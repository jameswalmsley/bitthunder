#ifndef _BT_STRUCT_H_
#define _BT_STRUCT_H_

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
