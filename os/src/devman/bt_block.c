/**
 *	Block Device Manager for BitThunder.
 *
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("BitThunder Block-device Manager")
BT_DEF_MODULE_DESCRIPTION	("Block Device manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

typedef struct _BLOCK_DEVICE {
	BT_LIST_ITEM 			oItem;			///< Can be a list item member.
	BT_HANDLE 				hBlkDev;		///< Handle to the block device instance.
	BT_BLKDEV_DESCRIOPTOR 	oDescriptor;	///< To be populated with block geometry.
} BLOCK_DEVICE;


BT_ERROR BT_BlockRead(BT_HANDLE hBlkDev, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer);
BT_ERROR BT_BlockWrite(BT_HANDLE hBlkDev, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer);








static BT_ERROR bt_block_device_manager_init() {

};

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_block_device_manager_init,
};
