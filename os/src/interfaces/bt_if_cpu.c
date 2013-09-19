#include <bitthunder.h>

BT_ERROR BT_BootCore(BT_u32 ulCoreID, void *address, bt_register_t a, bt_register_t b, bt_register_t c, bt_register_t d) {
	BT_ERROR Error;

	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(!pMachine) {
		return BT_ERR_GENERIC;
	}

	if(pMachine->pfnBootCore) {
		return pMachine->pfnBootCore(ulCoreID, address, a, b, c, d);
	}

	return BT_ERR_GENERIC;
}
