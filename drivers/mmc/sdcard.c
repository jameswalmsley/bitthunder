/**
 *	SDCard Protocol Implementation.
 *
 *	This driver provides a block device wrapper for sd host I/O devices.
 *
 **/

#include <bitthunder.h>

typedef struct _MMC_HOST {
	BT_HANDLE 		hHost;
	//BT_IF_SDIO_OPS *pOps;
} MMC_HOST;





BT_ERROR BT_RegisterSDHostController(BT_HANDLE hSDIO) {
	return BT_ERR_NONE;
}
