#ifndef _BT_IF_CHARDEV_H_
#define _BT_IF_CHARDEV_H_

#include "bt_types.h"

/**
 *	Here we define the main "function" prototypes that such a device may encounter.
 **/

typedef BT_ERROR (*BT_IF_CHARDEV_READ)		(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest);
typedef BT_ERROR (*BT_IF_CHARDEV_WRITE)		(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource);
typedef BT_ERROR (*BT_IF_CHARDEV_GETCH)		(BT_HANDLE hDevice, BT_u32 ulFlags);
typedef BT_ERROR (*BT_IF_CHARDEV_PUTCH)		(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u8 ucData);

typedef struct {
	BT_IF_CHARDEV_READ	pfnRead;
	BT_IF_CHARDEV_WRITE	pfnWrite;
	BT_IF_CHARDEV_GETCH	pfnGetch;
	BT_IF_CHARDEV_PUTCH	pfnPutch;
} BT_IF_CHARDEV;

BT_ERROR BT_CharDeviceRead(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest);
BT_ERROR BT_CharDeviceWrite(BT_HANDLE hDevice, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource);

#endif
