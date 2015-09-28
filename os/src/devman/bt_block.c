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
	BT_BLKDEV_DESCRIPTOR 	b;
};

static BT_LIST_HEAD(g_block_devices);

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {

	BT_BLKDEV_DESCRIPTOR *blk = bt_container_of(node, BT_BLKDEV_DESCRIPTOR, node);
	if(!blk->ulReferenceCount) {
		blk->ulReferenceCount += 1;
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
	if(hBlock && BT_HANDLE_TYPE(hBlock) == BT_HANDLE_T_BLOCK) {
		return BT_TRUE;
	}
	return BT_FALSE;
}

BT_s32 BT_BlockRead(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer) {

	if(!isHandleValid(hBlock)) {
		return BT_ERR_INVALID_HANDLE;
	}

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	const BT_IF_BLOCK *pOps = blkdev->hBlkDev->b.h.pIf->oIfs.pDevIF->pBlockIF;

	BT_kMutexPend(blkdev->kMutex, BT_INFINITE_TIMEOUT);
	BT_s32 ret = pOps->pfnReadBlocks(blkdev->hBlkDev, ulAddress, ulBlocks, pBuffer);
	BT_kMutexRelease(blkdev->kMutex);

	return ret;
}
BT_EXPORT_SYMBOL(BT_BlockRead);

BT_s32 BT_BlockWrite(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer) {
	if(!isHandleValid(hBlock)) {
		return BT_ERR_INVALID_HANDLE;
	}

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	const BT_IF_BLOCK *pOps = blkdev->hBlkDev->b.h.pIf->oIfs.pDevIF->pBlockIF;

	BT_kMutexPend(blkdev->kMutex, BT_INFINITE_TIMEOUT);
	BT_s32 ret = pOps->pfnWriteBlocks(blkdev->hBlkDev, ulAddress, ulBlocks, pBuffer);
	BT_kMutexRelease(blkdev->kMutex);

	return ret;
}
BT_EXPORT_SYMBOL(BT_BlockWrite);

BT_ERROR BT_GetBlockGeometry(BT_HANDLE hBlock, BT_BLOCK_GEOMETRY *pGeometry) {

	if(!isHandleValid(hBlock)) {
		return BT_ERR_INVALID_HANDLE;
	}

	BT_BLKDEV_DESCRIPTOR *blkdev = (BT_BLKDEV_DESCRIPTOR *) hBlock;
	if(pGeometry) {
		*pGeometry = blkdev->oGeometry;
	}

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_GetBlockGeometry);

BT_ERROR BT_RegisterBlockDevice(BT_HANDLE hDevice, const BT_i8 *szpName, BT_BLKDEV_DESCRIPTOR *pDescriptor) {

	BT_ERROR Error;

	pDescriptor->h.pIf = &oHandleInterface;
	pDescriptor->hBlkDev = hDevice;
	pDescriptor->kMutex = BT_kMutexCreate();
	bt_list_add(&pDescriptor->item, &g_block_devices);

	BT_LIST_INIT_HEAD(&pDescriptor->volumes);

	pDescriptor->node.pOps = &oDevfsOps;

	Error = BT_DeviceRegister(&pDescriptor->node, szpName);

	BT_kPrint("Block device: %s registered, enumerating partitions.", szpName);

	BT_EnumerateVolumes(pDescriptor);

	return Error;
}
BT_EXPORT_SYMBOL(BT_RegisterBlockDevice);

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
BT_EXPORT_SYMBOL(BT_UnregisterBlockDevice);

static BT_ERROR bt_blockdev_cleanup(BT_HANDLE hBlockDev) {

	BT_BLKDEV_DESCRIPTOR *blk = (BT_BLKDEV_DESCRIPTOR *) hBlockDev;

	if(blk->ulReferenceCount) {
		blk->ulReferenceCount -= 1;
	}

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.ulFlags = BT_HANDLE_FLAGS_NO_DESTROY,
	.eType = BT_HANDLE_T_BLOCK,
	.pfnCleanup = bt_blockdev_cleanup,
};
