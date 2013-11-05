/**
 *	Block Device Manager for BitThunder.
 *
 **/

#include <bitthunder.h>
#include <collections/bt_list.h>

BT_DEF_MODULE_NAME			("Block-device Manager")
BT_DEF_MODULE_DESCRIPTION	("Block Device manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;
};

static BT_LIST_HEAD(g_block_devices);

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {

	BT_BLKDEV_DESCRIPTOR *blk = bt_container_of(node, BT_BLKDEV_DESCRIPTOR, node);
	if(!blk->ulRefCount) {
		blk->ulRefCount += 1;
		BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &blk->h);
		return (BT_HANDLE) blk;
	}

	if(pError) {
		*pError = BT_ERR_GENERIC;
	}

	return NULL;
}

static const BT_DEVFS_OPS oDevfsOps = {
	.pfnOpen = devfs_open,
};

static BT_BOOL isHandleValid(BT_HANDLE hBlock) {
	if(hBlock && hBlock->h.pIf->eType == BT_HANDLE_T_BLOCK) {
		return BT_TRUE;
	}
	return BT_FALSE;
}

BT_u32 BT_BlockRead(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {

	if(!isHandleValid(hBlock)) {
		if(pError) {
			*pError = BT_ERR_INVALID_HANDLE;
		}
		return 0;
	}

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	const BT_IF_BLOCK *pOps = blkdev->hBlkDev->h.pIf->oIfs.pDevIF->pBlockIF;
	return pOps->pfnReadBlocks(blkdev->hBlkDev, ulAddress, ulBlocks, pBuffer, pError);
}

BT_u32 BT_BlockWrite(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	if(!isHandleValid(hBlock)) {
		if(pError) {
			*pError = BT_ERR_INVALID_HANDLE;
		}
		return 0;
	}

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	const BT_IF_BLOCK *pOps = blkdev->hBlkDev->h.pIf->oIfs.pDevIF->pBlockIF;
	return pOps->pfnWriteBlocks(blkdev->hBlkDev, ulAddress, ulBlocks, pBuffer, pError);
}

BT_ERROR BT_GetBlockGeometry(BT_HANDLE hBlock, BT_BLOCK_GEOMETRY *pGeometry) {

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;
	*pGeometry = blkdev->oGeometry;

	return BT_ERR_GENERIC;
}

BT_ERROR BT_RegisterBlockDevice(BT_HANDLE hDevice, const BT_i8 *szpName, BT_BLKDEV_DESCRIPTOR *pDescriptor) {

	BT_ERROR Error;

	pDescriptor->h.pIf = &oHandleInterface;
	pDescriptor->hBlkDev = hDevice;
	bt_list_add(&pDescriptor->item, &g_block_devices);

	BT_LIST_INIT_HEAD(&pDescriptor->volumes);

	Error = BT_DeviceRegister(&pDescriptor->node, szpName);

	BT_kPrint("Block device: %s registered, enumerating partitions.", szpName);

	BT_EnumerateVolumes(pDescriptor);

	return Error;
}

/*BT_HANDLE BT_BlockGetInode(BT_HANDLE hDevice) {
	if(!isHandleValid(hDevice)) {
		return NULL;
	}

	return hDevice->hInode;
	}*/

BT_ERROR BT_UnregisterBlockDevice(BT_HANDLE hDevice) {
	// find the block device entry that owns this handle

	// notify any users that this block device is now invalid!

	// free memory.
	return BT_ERR_NONE;
}

static BT_ERROR bt_blockdev_cleanup(BT_HANDLE hBlockDev) {

	BT_BLKDEV_DESCRIPTOR *blk = (BT_BLKDEV_DESCRIPTOR *) hBlockDev;

	if(!blk->ulRefCount) {
		blk->ulRefCount -= 1;
	}

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_BLOCK,
	.pfnCleanup = bt_blockdev_cleanup,
};
