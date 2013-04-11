#ifndef _BT_IF_FS_H_
#define _BT_IF_FS_H_

#include "bt_if_file.h"

typedef struct _BT_IF_FS {
	BT_HANDLE 	(*pfnMount)		(BT_HANDLE hFS, BT_HANDLE hVolume, BT_ERROR *pError);
	BT_ERROR	(*pfnUnmount)	(BT_HANDLE hMount);
	const BT_IF_FILE *pFileOps;
} BT_IF_FS;






#endif
