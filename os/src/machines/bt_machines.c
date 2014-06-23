/**
 *	Some APIs for accessing information from the architectures machine description.
 *
 *	@author James Walmsley
 **/

#include <bt_types.h>
#include <machines/bt_machines.h>
#include <devman/bt_devman.h>

BT_u32 BT_GetCpuClockFrequency() {

	BT_ERROR Error;

	BT_MACHINE_DESCRIPTION *pMachine = BT_GetMachineDescription(&Error);
	if(!pMachine) {
		return 0;
	}

	return pMachine->pfnGetCpuClockFrequency();
}
BT_EXPORT_SYMBOL(BT_GetCpuClockFrequency);
