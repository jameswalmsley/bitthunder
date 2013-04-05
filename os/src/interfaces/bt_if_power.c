/**
 *	POWER Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

BT_ERROR BT_SetPowerState(BT_HANDLE hDevice, BT_POWER_STATE eState) {
	if ((!hDevice) || (!BT_IF_DEVICE(hDevice)))
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;

	if (BT_IF_DEVICE(hDevice)->pPowerIF == NULL) return BT_ERR_UNIMPLEMENTED;

	return BT_IF_DEVICE(hDevice)->pPowerIF->pfnSetPowerState(hDevice, eState);
}

BT_ERROR BT_GetPowerState(BT_HANDLE hDevice, BT_POWER_STATE *peState) {
	if ((!hDevice) || (!BT_IF_DEVICE(hDevice)))
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;

	if (BT_IF_DEVICE(hDevice)->pPowerIF == NULL) return BT_ERR_UNIMPLEMENTED;

	return BT_IF_DEVICE(hDevice)->pPowerIF->pfnGetPowerState(hDevice, peState);
}
