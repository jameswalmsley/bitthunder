#ifndef _BT_INTEGRATED_DRIVER_H_
#define _BT_INTEGRATED_DRIVER_H_

#include "bt_types.h"
#include "bt_device.h"
#include "bt_resource.h"
#include "bt_i2c.h"
#include <net/bt_phy.h>

typedef enum {
	BT_DRIVER_STANDARD = 0,
	BT_DRIVER_I2C,
	BT_DRIVER_SPI,
	BT_DRIVER_MII
} BT_DRIVER_TYPE;

struct bt_match_table_32 {
	BT_u32 id;
	BT_u32 id_mask;
};

typedef struct _BT_INTEGRATED_DRIVER {
	const BT_i8    *name;
	BT_DRIVER_TYPE	eType;
	union {
		BT_HANDLE 		(*pfnProbe)		(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError);
		BT_HANDLE		(*pfnI2CProbe)	(const BT_I2C_BUS *pBus, const BT_DEVICE *pDevice, BT_ERROR *pError);
		BT_HANDLE		(*pfnMIIProbe)	(BT_HANDLE hMII, struct bt_mii_bus *pBus, BT_u32 addr, BT_ERROR *pError);
	};
	union {
		struct bt_match_table_32 *pMatch32;
	};
} BT_INTEGRATED_DRIVER;

#define BT_INTEGRATED_DRIVER_DEF	static const BT_ATTRIBUTE_SECTION(".bt.arch.drivers") BT_INTEGRATED_DRIVER

#endif
