#ifndef _BT_INODE_H_
#define _BT_INODE_H_

#include "bt_dir.h"

typedef struct _BT_INODE {
	BT_u64	ullFileSize;
	BT_u32	attr;
} BT_INODE;

BT_ERROR BT_ReadInode(BT_HANDLE hInode, BT_INODE *pInode);


#endif
