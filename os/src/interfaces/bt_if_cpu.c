#include <bitthunder.h>

BT_ERROR BT_BootCore(BT_u32 ulCoreID, void *pStart) {
	BT_ERROR Error;

	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(!pMachine) {
		return BT_ERR_GENERIC;
	}

	if(pMachine->pfnBootCore) {
		return pMachine->pfnBootCore(ulCoreID, pStart);
	}

	return BT_ERR_GENERIC;
}
