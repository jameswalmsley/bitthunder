/**
 *	BitThunder Directory Handling Interface.
 **/

#ifndef _BT_IF_DIR_H_
#define _BT_IF_DIR_H_

#include <fs/bt_dir.h>

typedef struct _BT_IF_DIR {
	BT_ERROR (*pfnReadDir)(BT_HANDLE hDir, BT_DIRENT *pDirent);
} BT_IF_DIR;









#endif
