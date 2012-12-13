#ifndef _BT_IF_POWER_H_
#define _BT_IF_POWER_H_

#include "bt_types.h"

typedef enum {
	BT_POWER_STATE_ASLEEP,
	BT_POWER_STATE_AWAKE,
	//	Potential to add other power states, like low-power mode. Devices can choose what to support.
} BT_POWER_STATE;

/**
 *	All device drivers should implement a device sleep/wake function.
 *	This should allow any device to go into a low-power mode if supported, using a single API for any device.
 **/
typedef BT_ERROR	(*BT_DEVICE_SET_POWERSTATE_FN) (BT_HANDLE hDevice, BT_POWER_STATE ePowerState);
typedef BT_ERROR	(*BT_DEVICE_GET_POWERSTATE_FN) (BT_HANDLE hDevice, BT_POWER_STATE *pePowerState);

typedef struct {
	BT_DEVICE_SET_POWERSTATE_FN	pfnSetPowerState;
	BT_DEVICE_GET_POWERSTATE_FN	pfnGetPowerState;
} BT_IF_POWER;

BT_ERROR BT_SetPowerState(BT_HANDLE hDevice, BT_POWER_STATE eState);
BT_ERROR BT_GetPowerState(BT_HANDLE hDevice, BT_POWER_STATE *peState);


#endif
