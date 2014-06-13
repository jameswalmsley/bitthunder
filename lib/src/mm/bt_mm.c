/**
 *	Memory Manager for BitThunder.
 *
 *
 **/

#include <bitthunder.h>
#include <stdlib.h>


void *BT_Calloc(BT_u32 ulSize) {
	return calloc(1, ulSize);
}
BT_EXPORT_SYMBOL(BT_Calloc);
