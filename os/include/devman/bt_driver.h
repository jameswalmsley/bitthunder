#include "bt_types.h"
#include "bt_device.h"
#include "bt_resource.h"
#include "interfaces/bt_dev_if_spi.h"
#include <net/bt_phy.h>

typedef enum {
	BT_DRIVER_INTEGRATED = 0,
	BT_DRIVER_I2C,
	BT_DRIVER_SPI,
	BT_DRIVER_MII
} BT_DRIVER_TYPE;

struct bt_match_table_32 {
	BT_u32 id;
	BT_u32 id_mask;
};

typedef struct _BT_DRIVER {
	const BT_i8    *name;
	BT_u32			eType;							///< Enum from BT_DRIVER_TYPE
    #define BT_DRIVER_TYPE_CODE_MASK	0x000000FF	// Driver type code field
    #define BT_DRIVER_DEVFS_PROBE 		0x80000000	// Flag the driver to not be probed on startup but be probed from devfs open.

	union {
		BT_HANDLE 		(*pfnProbe)	(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError);
		BT_HANDLE		(*pfnI2CProbe)	(BT_HANDLE hI2CBus, const BT_DEVICE *pDevice, BT_ERROR *pError);
		BT_HANDLE 		(*pfnSPIProbe) 	(BT_SPI_DEVICE *pSpiDevice, const BT_DEVICE *pDevice, BT_ERROR *pError);
		BT_HANDLE		(*pfnMIIProbe)	(BT_HANDLE hMII, struct bt_mii_bus *pBus, BT_u32 addr, BT_ERROR *pError);
	};
	union {
		struct bt_match_table_32 *pMatch32;
	};
} BT_DRIVER;

#define BT_DRIVER_DEF	static const BT_ATTRIBUTE_SECTION(".bt.arch.drivers") BT_DRIVER
