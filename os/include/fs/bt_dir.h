#ifndef _BT_DIR_H_
#define _BT_DIR_H_

#include <bt_time.h>

typedef struct _BT_DIRENT {
	BT_i8  *szpName;
	BT_u64	ullFileSize;
	BT_u32	attr;
	#define BT_ATTR_DIR	0x00000001
	BT_DATETIME	    ctime;
	BT_DATETIME		atime;
	BT_DATETIME		mtime;
} BT_DIRENT;

BT_ERROR BT_ReadDir(BT_HANDLE hDir, BT_DIRENT *pDirent);

#endif
