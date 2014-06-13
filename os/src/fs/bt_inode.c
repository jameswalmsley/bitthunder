#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

BT_ERROR BT_ReadInode(BT_HANDLE hInode, BT_INODE *pInode) {
	return hInode->h.pIf->oIfs.pInodeIF->pfnReadInode(hInode, pInode);
}
BT_EXPORT_SYMBOL(BT_ReadInode);
