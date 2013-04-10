/**
 *	Block Device Manager for BitThunder.
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("Block-device Manager")
BT_DEF_MODULE_DESCRIPTION	("Block Device manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	BT_LIST_ITEM 			oItem;			///< Can be a list item member.
	BT_HANDLE 				hBlkDev;		///< Handle to the block device instance.
	BT_BLKDEV_DESCRIPTOR 	oDescriptor;	///< To be populated with block geometry.
	BT_u32					ulRefCount;		///< Number of users.
	BT_HANDLE				hInode;
};

static BT_LIST g_oBlockDevices = {0};

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(BT_HANDLE hDevice, BT_ERROR *pError) {
	if(!hDevice->ulRefCount) {
		hDevice->ulRefCount += 1;
		return hDevice;
	}

	return NULL;
}

static const BT_DEVFS_OPS oDevfsOps = {
	.pfnOpen = devfs_open,
};

BT_u32 BT_BlockRead(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	const BT_IF_BLOCK *pBlock = hBlock->hBlkDev->h.pIf->oIfs.pBlockIF;
	return pBlock->pfnReadBlocks(hBlock->hBlkDev, ulAddress, ulBlocks, pBuffer, pError);
}

BT_u32 BT_BlockWrite(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	const BT_IF_BLOCK *pBlock = hBlock->hBlkDev->h.pIf->oIfs.pBlockIF;
	return pBlock->pfnWriteBlocks(hBlock->hBlkDev, ulAddress, ulBlocks, pBuffer, pError);
}

BT_ERROR BT_RegisterBlockDevice(BT_HANDLE hDevice, const char *szpName, BT_BLKDEV_DESCRIPTOR *pDescriptor) {

	BT_ERROR Error;
	BT_HANDLE hBlock = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hBlock) {
		return BT_ERR_NO_MEMORY;
	}

	hBlock->hBlkDev = hDevice;
	hBlock->oDescriptor = *pDescriptor;

	BT_ListAddItem(&g_oBlockDevices, &hBlock->oItem);

	hBlock->hInode = BT_DeviceRegister(hBlock, szpName, &oDevfsOps, &Error);

	return BT_ERR_NONE;
}

BT_ERROR BT_UnregisterBlockDevice(BT_HANDLE hDevice) {
	// find the block device entry that owns this handle

	// notify any users that this block device is now invalid!

	// free memory.
	return BT_ERR_NONE;
}

static BT_ERROR bt_blockdev_inode_cleanup(BT_HANDLE hBlockdev) {
	if(hBlockdev->ulRefCount) {
		hBlockdev->ulRefCount -= 1;
		return BT_HANDLE_DO_NOT_FREE;	// Ensure handle is not free'd!
	} else {
		// Request to destroy the inode!
	}

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	{NULL},
	BT_HANDLE_T_INODE,
	bt_blockdev_inode_cleanup,
};

static BT_ERROR bt_block_device_manager_init() {

	BT_ERROR Error;

	Error = BT_ListInit(&g_oBlockDevices);

	return Error;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_block_device_manager_init,
};
