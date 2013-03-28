/**
 * 	BitThunder Device File-system.
 *
 *	This provides a really simple way for the machine file to put non-managed devices into the
 *	file-system for use by applications.
 *
 *	Basically, a call to BT_Open("/dev/{.szpName}") will call the driver's probe function and return a
 *	a handle.
 *
 *	Functionality of that handle is determined by the device driver and its provided interfaces,
 *	but most devices should support some kind of access with BT_{Read|Write}().
 */

#ifndef _BT_DEVFS_H_
#define _BT_DEVFS_H_

#include "bt_types.h"

typedef struct _BT_DEVFS_INODE {
	const char 					*szpName;	///< Device file-system entry name.
	const BT_INTEGRATED_DEVICE 	*pDevice;	///< Integrated device that the entry represents.
} BT_DEVFS_INODE;

typedef struct _BT_DEVFS_OPS {
	BT_HANDLE (*pfnOpen) (BT_HANDLE hDevice, BT_ERROR *pError);

} BT_DEVFS_OPS;

#define BT_DEVFS_INODE_DEF 		static const BT_ATTRIBUTE_SECTION(".bt.devfs.entries") BT_DEVFS_INODE

BT_HANDLE BT_DeviceOpen(const char *szpDevicePath, BT_ERROR *pError);

BT_HANDLE BT_DeviceRegister(BT_HANDLE hDevice, const char *szpName, BT_ERROR *pError);

#endif
