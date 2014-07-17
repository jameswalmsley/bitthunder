#ifndef _BT_DIR_H_
#define _BT_DIR_H_

#include <bt_time.h>

typedef struct _BT_DIRENT {
	BT_i8  *szpName;
	union {
		BT_u64	ullFileSize;
		BT_u64 	ullFilesize;
	};
	BT_u32	attr;
	#define BT_ATTR_DIR			0x00000001
	#define	BT_ATTR_READONLY	0x00000002
	#define	BT_ATTR_HIDDEN		0x00000004
	#define	BT_ATTR_SYSTEM		0x00000008
	#define	BT_ATTR_ARCHIVE		0x00000010
	BT_DATETIME	    ctime;
	BT_DATETIME		atime;
	BT_DATETIME		mtime;
} BT_DIRENT;

BT_ERROR BT_ReadDir(BT_HANDLE hDir, BT_DIRENT *pDirent);

#endif
