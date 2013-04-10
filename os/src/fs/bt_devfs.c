/**
 * BitThunder - Device Filesystem.
 *
 *
 **/

#include <bitthunder.h>
#include <string.h>

extern const BT_DEVFS_INODE __bt_devfs_entries_start;
extern const BT_DEVFS_INODE __bt_devfs_entries_end;


#ifdef BT_CONFIG_FS_DEV_DYNAMIC_REGISTRATION
typedef struct _BT_DEVFS_INODE_LIST {
	BT_LIST_ITEM 	oItem;
	BT_DEVFS_INODE 	oInode;
} BT_DEVFS_INODE_LIST;
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

#endif

	return NULL;
}

#ifdef BT_CONFIG_FS_DEV_DYNAMIC_REGISTRATION

BT_ERROR BT_DeviceRegister(const BT_INTEGRATED_DEVICE *pDevice, const char *szpName) {

}

#endif
