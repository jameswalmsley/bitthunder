/**
 * BitThunder - Device Filesystem.
 *
 *
 **/

#include <bitthunder.h>
#include <string.h>

BT_DEF_MODULE_NAME			("Process Manager")
BT_DEF_MODULE_DESCRIPTION	("OS Process abstraction for the BitThunder Kernel")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

extern const BT_DEVFS_INODE __bt_devfs_entries_start;
extern const BT_DEVFS_INODE __bt_devfs_entries_end;

#ifdef BT_CONFIG_FS_DEV_DYNAMIC_REGISTRATION

static BT_LIST oInodeList;

typedef struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	BT_LIST_ITEM 		oItem;
	char 		   	   *szpName;
	BT_HANDLE			hDevice;
	BT_u32				ulReferences;
	BT_DEVFS_OPS	   *pOps;
} BT_DEVFS_INODE_ITEM;
#endif

BT_HANDLE BT_DeviceOpen(const char *szpFilename, BT_ERROR *pError) {

	const BT_INTEGRATED_DRIVER *pDriver;

	BT_u32 size = (BT_u32) ((BT_u32) &__bt_devfs_entries_end - (BT_u32) &__bt_devfs_entries_start);
	size /= sizeof(BT_DEVFS_INODE);

	const BT_DEVFS_INODE *pInode = &__bt_devfs_entries_start;

	while(size--) {
		if(!strcmp(pInode->szpName, (const char *) szpFilename)) {
			pDriver = BT_GetIntegratedDriverByName(pInode->pDevice->name);
			if(!pDriver) {
				break;
			}

			return pDriver->pfnProbe(pInode->pDevice, pError);
		}
		pInode++;
	}

#ifdef BT_CONFIG_FS_DEV_DYNAMIC_REGISTRATION
	BT_HANDLE hInode = (BT_HANDLE) oInodeList.pStart;
	while(hInode) {
		if(!strcmp(hInode->szpName, szpFilename)) {
			return hInode->pOps->pfnOpen(hInode->hDevice, pError);
		}

		hInode = (BT_HANDLE) BT_ListGetNext(&hInode->oItem);
	}
#endif

	return NULL;
}

#ifdef BT_CONFIG_FS_DEV_DYNAMIC_REGISTRATION

static const BT_IF_HANDLE oHandleInterface;

BT_HANDLE BT_DeviceRegister(BT_HANDLE hDevice, const char *szpName, const BT_DEVFS_OPS *pOps, BT_ERROR *pError) {
	BT_ERROR Error;
	BT_HANDLE hInode = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hInode) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hInode->szpName = BT_kMalloc(strlen(szpName) + 1);
	strcpy(hInode->szpName, szpName);

	hInode->hDevice = hDevice;

	BT_ListAddItem(&oInodeList, &hInode->oItem);

	if(pError) {
		*pError = BT_ERR_NONE;
	}

	return hInode;

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

static BT_ERROR bt_devfs_cleanup(BT_HANDLE hDevfs) {

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	{NULL},
	BT_HANDLE_T_INODE,
	bt_devfs_cleanup,
};

static BT_ERROR bt_devfs_init() {
	return BT_ListInit(&oInodeList);
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_devfs_init,
};
#endif
