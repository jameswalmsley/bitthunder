#ifndef _BT_IF_CHARDEV_H_
#define _BT_IF_CHARDEV_H_

#include "bt_types.h"

/**
 *	Here we define the main "function" prototypes that such a device may encounter.
 **/

typedef struct {
	BT_ERROR	(*pfnRead) 	(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest);
	BT_ERROR	(*pfnWrite)	(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource);
	BT_ERROR	(*pfnGetch)	(BT_HANDLE hDevice, BT_u32 ulFlags);
	BT_ERROR	(*pfnPutch)	(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u8 ucData);
} BT_IF_CHARDEV;

BT_ERROR BT_CharDeviceRead	(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest);
BT_ERROR BT_CharDeviceWrite	(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource);

#endif
