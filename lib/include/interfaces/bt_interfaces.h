#ifndef _BT_INTERFACES_H_
#define _BT_INTERFACES_H_

#include "interfaces/bt_if_device.h"
#include "interfaces/bt_if_block.h"
#include "interfaces/bt_if_fs.h"
#include "interfaces/bt_if_module.h"
#include "interfaces/bt_if_cpu.h"

#include <bt_module.h>

typedef BT_ERROR (*BT_HANDLE_CLEANUP)	(BT_HANDLE hHandle);

typedef enum _BT_HANDLE_TYPE {
	BT_HANDLE_T_INVALID 	= 0,
	BT_HANDLE_T_PROCESS,
	BT_HANDLE_T_THREAD,
	BT_HANDLE_T_SYSTEM,
	BT_HANDLE_T_MODULE,
	BT_HANDLE_T_CALLBACK,
#ifdef BT_CONFIG_OS
	BT_HANDLE_T_INODE,
	BT_HANDLE_T_DEVICE,
	BT_HANDLE_T_BLOCK,
	BT_HANDLE_T_VOLUME,
	BT_HANDLE_T_PARTITION,
	BT_HANDLE_T_MTD,
	BT_HANDLE_T_FILESYSTEM,
	BT_HANDLE_T_MOUNTPOINT,
	BT_HANDLE_T_FILE,
	BT_HANDLE_T_DIRECTORY,
	BT_HANDLE_T_FIFO,
	BT_HANDLE_T_SHELL,
	BT_HANDLE_T_RTC,
	BT_HANDLE_T_I2C_BUS,
#endif
} BT_HANDLE_TYPE;

typedef struct _BT_HANDLE_INTERFACE *BT_HANDLE_INTERFACE;

typedef union _BT_IF_INTERFACES {
	BT_HANDLE_INTERFACE	p;
#ifdef BT_CONFIG_OS
	const BT_IF_DEVICE *pDevIF;
	const BT_IF_FS	   *pFilesystemIF;
	const BT_IF_DIR    *pDirIF;
	const BT_IF_INODE  *pInodeIF;
#endif
} BT_UN_IFS;

typedef struct _BT_IF_HANDLE {
	BT_MODULE_INFO	   	oInfo;
	BT_u32				ulFlags;
#define BT_HANDLE_FLAGS_NO_DESTROY 	0x00000001
	const BT_UN_IFS	   	oIfs;
#ifdef BT_CONFIG_OS
	const BT_IF_FILE   *pFileIF;	///< If handle implements a file interface then use this!
#endif
	BT_HANDLE_TYPE 		eType;
	BT_ERROR			(*pfnCleanup)	(BT_HANDLE h);
} BT_IF_HANDLE;

#endif
