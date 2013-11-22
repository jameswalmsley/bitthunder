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

#include <collections/bt_list.h>
#include "bt_types.h"

struct bt_devfs_node;
struct _BT_DEVICE;

typedef struct _BT_DEVFS_OPS {
	BT_HANDLE (*pfnOpen) (struct bt_devfs_node *node, BT_ERROR *pError);
} BT_DEVFS_OPS;

struct bt_devfs_node {
	struct bt_list_head item;
	BT_i8			   *szpName;
	const BT_DEVFS_OPS *pOps;
};

typedef struct _BT_DEVFS_INODE {
	const char 					*szpName;	///< Device file-system entry name.
	const struct _BT_DEVICE		*pDevice;	///< Integrated device that the entry represents.
} BT_DEVFS_INODE;

#define BT_DEVFS_INODE_DEF 		static const BT_ATTRIBUTE_SECTION(".bt.devfs.entries") BT_DEVFS_INODE

BT_HANDLE BT_DeviceOpen(const char *szpDevicePath, BT_ERROR *pError);

BT_ERROR BT_DeviceRegister(struct bt_devfs_node *node, const char *szpName);
BT_i8 *BT_GetInodeName(BT_HANDLE h, BT_ERROR *pError);

#endif
