#ifndef _BT_IF_FS_H_
#define _BT_IF_FS_H_

#include "bt_if_file.h"

typedef struct _BT_IF_FS {
	BT_HANDLE 	(*pfnMount)		(BT_HANDLE hFS, BT_HANDLE hVolume, BT_ERROR *pError);
	BT_ERROR	(*pfnUnmount)	(BT_HANDLE hMount);
	BT_HANDLE	(*pfnOpen)		(BT_HANDLE hMount, BT_i8 *szpPath, BT_u32 ulModeFlags, BT_ERROR *pError);
} BT_IF_FS;






#endif
