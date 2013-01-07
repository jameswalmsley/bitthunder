#ifndef _BT_INTEGRATED_DRIVER_H_
#define _BT_INTEGRATED_DRIVER_H_

#include "bt_types.h"
#include "bt_resource.h"

typedef struct _BT_INTEGRATED_DRIVER {
	const BT_i8    *name;
	BT_HANDLE 		(*pfnProbe)		(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError);
//	BT_ERROR 		(*pfnRemove)	(BT_HANDLE hDevice);
} BT_INTEGRATED_DRIVER;

#define BT_INTEGRATED_DRIVER_DEF	static const BT_ATTRIBUTE_SECTION(".bt.arch.drivers") BT_INTEGRATED_DRIVER

#endif

