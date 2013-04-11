#ifndef _BT_INTERFACES_H_
#define _BT_INTERFACES_H_

#include "interfaces/bt_if_device.h"
#include "interfaces/bt_if_block.h"
#include "interfaces/bt_if_fs.h"
#include "interfaces/bt_if_module.h"

#include <bt_module.h>

typedef BT_ERROR (*BT_HANDLE_CLEANUP)	(BT_HANDLE hHandle);

typedef enum _BT_HANDLE_TYPE {
	BT_HANDLE_T_INVALID 	= 0,
	BT_HANDLE_T_PROCESS,
	BT_HANDLE_T_SYSTEM,
	BT_HANDLE_T_MODULE,
#ifdef BT_CONFIG_OS
	BT_HANDLE_T_INODE,
	BT_HANDLE_T_DEVICE,
	BT_HANDLE_T_BLOCK,
	BT_HANDLE_T_VOLUME,
	BT_HANDLE_T_PARTITION,
	BT_HANDLE_T_FILESYSTEM,
	BT_HANDLE_T_MOUNTPOINT,
#endif
} BT_HANDLE_TYPE;

typedef struct _BT_HANDLE_INTERFACE *BT_HANDLE_INTERFACE;

typedef union _BT_IF_INTERFACES {
	BT_HANDLE_INTERFACE	p;
#ifdef BT_CONFIG_OS
	const BT_IF_DEVICE *pDevIF;
	const BT_IF_BLOCK  *pBlockIF;
	const BT_IF_FS	   *pFilesystemIF;
#endif
} BT_UN_IFS;

typedef struct _BT_IF_HANDLE {
	BT_MODULE_INFO	   oInfo;

	const BT_UN_IFS	   	oIfs;

	BT_HANDLE_TYPE 		eType;
	BT_ERROR			(*pfnCleanup)	(BT_HANDLE h);
} BT_IF_HANDLE;

#define BT_HANDLE_DO_NOT_FREE	0x40000001	///< Cleanup interfaces can return this to mean success, but DO not free the handle!

#endif
