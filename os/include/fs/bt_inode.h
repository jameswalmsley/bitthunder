#ifndef _BT_INODE_H_
#define _BT_INODE_H_

typedef struct _BT_INODE {
	BT_u64	ullFilesize;
} BT_INODE;

BT_ERROR BT_ReadInode(BT_HANDLE hInode, BT_INODE *pInode);


#endif
