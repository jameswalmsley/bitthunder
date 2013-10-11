#ifndef _BT_DIR_H_
#define _BT_DIR_H_

typedef struct _BT_DIRENT {
	BT_u64	ullFileSize;
	BT_i8  *szpName;
	BT_u32	attr;
#define BT_ATTR_DIR	0x00000001
} BT_DIRENT;


BT_ERROR BT_ReadDir(BT_HANDLE hDir, BT_DIRENT *pDirent);

#endif
