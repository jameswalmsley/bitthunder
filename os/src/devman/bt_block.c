/**
 *	Block Device Manager for BitThunder.
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("Block-device Manager")
BT_DEF_MODULE_DESCRIPTION	("Block Device manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

typedef struct _BLOCK_DEVICE {
	BT_LIST_ITEM 			oItem;			///< Can be a list item member.
	BT_HANDLE 				hBlkDev;		///< Handle to the block device instance.
	BT_BLKDEV_DESCRIPTOR 	oDescriptor;	///< To be populated with block geometry.
	BT_u32					ulRefCount;		///< Number of users.
} BLOCK_DEVICE;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
};

static BT_LIST g_oBlockDevices = {0};

BT_u32 BT_BlockRead(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	const BT_IF_BLOCK *pBlock = hBlock->h.pIf->oIfs.pBlockIF;
	return pBlock->pfnReadBlocks(hBlock, ulAddress, ulBlocks, pBuffer, pError);
}

BT_u32 BT_BlockWrite(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	const BT_IF_BLOCK *pBlock = hBlock->h.pIf->oIfs.pBlockIF;
	return pBlock->pfnWriteBlocks(hBlock, ulAddress, ulBlocks, pBuffer, pError);
}

BT_ERROR BT_RegisterBlockDevice(BT_HANDLE hDevice, BT_BLKDEV_DESCRIPTOR *pDescriptor) {
	BLOCK_DEVICE *pDevice = BT_kMalloc(sizeof(BLOCK_DEVICE));
	if(!pDevice) {
		return BT_ERR_NO_MEMORY;
	}

	pDevice->hBlkDev = hDevice;
	pDevice->oDescriptor = *pDescriptor;

	BT_ListAddItem(&g_oBlockDevices, &pDevice->oItem);

	return BT_ERR_NONE;
}

BT_ERROR BT_UnregisterBlockDevice(BT_HANDLE hDevice) {
	// find the block device entry that owns this handle

	// notify any users that this block device is now invalid!

	// free memory.
}

static BT_ERROR bt_block_device_manager_init() {

	BT_ERROR Error;

	Error = BT_ListInit(&g_oBlockDevices);

	return Error;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_block_device_manager_init,
};
