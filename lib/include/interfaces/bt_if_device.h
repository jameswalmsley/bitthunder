#ifndef _BT_IF_DEVICE_H_
#define _BT_IF_DEVICE_H_

#include "bt_types.h"

typedef BT_HANDLE	(*BT_DEVICE_HANDLE_OPEN)	(BT_u32 nDeviceID, BT_ERROR *pError);

typedef struct _BT_IF_DEVICE {
	BT_u32					ulTotalDevices;
	BT_DEVICE_HANDLE_OPEN	pfnOpen;



} BT_IF_DEVICE;




#endif
