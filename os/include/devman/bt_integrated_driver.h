#ifndef _BT_INTEGRATED_DRIVER_H_
#define _BT_INTEGRATED_DRIVER_H_

#include "bt_types.h"
#include "bt_device.h"
#include "bt_resource.h"
#include "bt_i2c.h"

typedef enum {
	BT_DRIVER_STANDARD = 0,
	BT_DRIVER_I2C,
} BT_DRIVER_TYPE;

typedef struct _BT_INTEGRATED_DRIVER {
	const BT_i8    *name;
	BT_DRIVER_TYPE	eType;
	union {
		BT_HANDLE 		(*pfnProbe)		(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError);
		BT_HANDLE		(*pfnI2CProbe)	(BT_HANDLE hI2C, const BT_DEVICE *pDevice, BT_ERROR *pError);
	};
} BT_INTEGRATED_DRIVER;

#define BT_INTEGRATED_DRIVER_DEF	static const BT_ATTRIBUTE_SECTION(".bt.arch.drivers") BT_INTEGRATED_DRIVER

#endif
