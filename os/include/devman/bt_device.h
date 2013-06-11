#ifndef _BT_DEVICE_H_
#define _BT_DEVICE_H_

#include "bt_types.h"
#include "bt_resource.h"

typedef enum {
	BT_DEVICE_INTEGRATED= 0,	///< Platform/Integrated device type.
	BT_DEVICE_I2C,				///< I2C device type.
} BT_DEVICE_TYPE;

typedef struct _BT_DEVICE {
	const BT_i8		   *name;
	BT_DEVICE_TYPE 		eType;
	BT_u32				ulTotalResources;
	const BT_RESOURCE  *pResources;
} BT_DEVICE;

#define BT_DEVICE_DEF	static const BT_ATTRIBUTE_SECTION(".bt.devices") BT_DEVICE

BT_u32 BT_GetTotalDevicesByType(BT_DEVICE_TYPE eType);
const BT_DEVICE *BT_GetDeviceByType(BT_DEVICE_TYPE eType, BT_u32 ulIndex);
const BT_RESOURCE *BT_GetDeviceResource(const BT_DEVICE *pDevice, BT_u32 ulType, BT_u32 ulNum);

#endif
