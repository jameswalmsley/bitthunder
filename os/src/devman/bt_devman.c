/**
 *	BitThunder Device Manager
 *
 *
 **/


#include <bitthunder.h>

extern const BT_MACHINE_DESCRIPTION * __bt_arch_init_start;
extern const BT_MACHINE_DESCRIPTION * __bt_arch_init_end;

extern const BT_u32					  __bt_arch_init_size;

BT_MACHINE_DESCRIPTION *BT_GetMachineDescription(BT_ERROR *pError) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_init_end - (BT_u32) &__bt_arch_init_start);

	if(size != sizeof(BT_MACHINE_DESCRIPTION)) {
		if(pError) {
			*pError = BT_ERR_GENERIC;
		}
		return NULL;
	}


	return	(BT_MACHINE_DESCRIPTION *) &__bt_arch_init_start;
}

