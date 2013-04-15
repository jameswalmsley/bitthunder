/**
 *	BT Inode interface.
 *
 **/

#ifndef _BT_IF_INODE_H_
#define _BT_IF_INODE_H_

#include <fs/bt_inode.h>

typedef struct _BT_IF_INODE {
	BT_ERROR (*pfnReadInode)(BT_HANDLE hInode, BT_INODE *pInode);
} BT_IF_INODE;




#endif
