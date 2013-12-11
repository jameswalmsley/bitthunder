#ifndef _BT_INODE_H_
#define _BT_INODE_H_

#include "bt_dir.h"

typedef struct _BT_INODE {
	union {
		BT_u64			ullFileSize;
		BT_u64 			ullFilesize;
	};
	BT_u32			attr;
	BT_DATETIME	    ctime;
	BT_DATETIME		atime;
	BT_DATETIME		mtime;
} BT_INODE;

BT_ERROR BT_ReadInode(BT_HANDLE hInode, BT_INODE *pInode);


#endif
