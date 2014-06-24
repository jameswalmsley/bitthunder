/**
 * BitThunder - Device Filesystem.
 *
 *
 **/

#include <bitthunder.h>
#include <collections/bt_list.h>
#include <bt_struct.h>
#include <string.h>

BT_DEF_MODULE_NAME			("devfs")
BT_DEF_MODULE_DESCRIPTION	("Device mount-point management")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

extern const BT_DEVFS_INODE __bt_devfs_entries_start;
extern const BT_DEVFS_INODE __bt_devfs_entries_end;

#ifdef BT_CONFIG_FS_DEV_DYNAMIC_REGISTRATION
static BT_LIST_HEAD(g_devfs_nodes);
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
	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_devfs_nodes) {
		struct bt_devfs_node *node = (struct bt_devfs_node *) pos;
		if(!strcmp(node->szpName, szpFilename)) {
			return node->pOps->pfnOpen(node, pError);
		}
	}
#endif

	return NULL;
}
BT_EXPORT_SYMBOL(BT_DeviceOpen);

#ifdef BT_CONFIG_FS_DEV_DYNAMIC_REGISTRATION

static const BT_IF_HANDLE oHandleInterface;
static const BT_IF_HANDLE oDirHandleInterface;

BT_ERROR BT_DeviceRegister(struct bt_devfs_node *node, const char *szpName) {

	node->szpName = BT_kMalloc(strlen(szpName) + 1);
	strcpy(node->szpName, szpName);

	bt_list_add(&node->item, &g_devfs_nodes);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DeviceRegister);

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

struct devfs_mount {
	BT_HANDLE_HEADER h;
};

struct devfs_dir {
	BT_HANDLE_HEADER h;
	BT_u32 ulCurrentEntry;
};

static BT_HANDLE devfs_mount(BT_HANDLE hFS, const void *data, BT_ERROR *pError) {
	BT_HANDLE hMount = BT_CreateHandle(&oHandleInterface, sizeof(struct devfs_mount), pError);
	return hMount;
}

static BT_HANDLE devfs_open(BT_HANDLE hMount, const BT_i8 *szpPath, BT_u32 ulModeFlags, BT_ERROR *pError) {

	BT_ERROR Error;
	BT_HANDLE hDev = BT_DeviceOpen(&szpPath[1], &Error);
	if(hDev) {
		return hDev;
	}

	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_devfs_nodes) {
		struct bt_devfs_node *node = (struct bt_devfs_node *) pos;
		if(!strcmp(node->szpName, &szpPath[1])) {
			return node->pOps->pfnOpen(node, pError);
		}
	}

	return NULL;
}

static BT_HANDLE devfs_opendir(BT_HANDLE hMount, const BT_i8 *szpPath, BT_ERROR *pError) {

	if(strcmp(szpPath, "/")) {
		return NULL;
	}

	BT_HANDLE hDir = BT_CreateHandle(&oDirHandleInterface, sizeof(struct devfs_dir), pError);
	return hDir;
}

static BT_ERROR devfs_readdir(BT_HANDLE hDir, BT_DIRENT *pDirent) {

	struct devfs_dir *pDir = (struct devfs_dir *) hDir;

	struct bt_list_head *pos;
	BT_u32 i = 0;

	BT_u32 size = (BT_u32) ((BT_u32) &__bt_devfs_entries_end - (BT_u32) &__bt_devfs_entries_start);
	size /= sizeof(BT_DEVFS_INODE);

	const BT_DEVFS_INODE *pInode = &__bt_devfs_entries_start;

	while(size--) {
		if(i++ == pDir->ulCurrentEntry) {
			pDirent->szpName = (BT_i8 *) pInode->szpName;
			pDirent->ullFileSize = 0;
			pDir->ulCurrentEntry += 1;
			return BT_ERR_NONE;
		}
		pInode++;
	}


	bt_list_for_each(pos, &g_devfs_nodes) {
		struct bt_devfs_node *node = (struct bt_devfs_node *) pos;
		if(i++ == pDir->ulCurrentEntry) {
			pDirent->szpName = node->szpName;
			pDirent->ullFileSize = 0;
			pDir->ulCurrentEntry += 1;
			return BT_ERR_NONE;
		}
	}

	return BT_ERR_GENERIC;
}

static const BT_IF_DIR oDirOperations = {
	.pfnReadDir = devfs_readdir,
};

static const BT_IF_FS oFilesystemInterface = {
	.ulFlags 		= BT_FS_FLAG_NODEV,
	.name			= "devfs",
	.pfnMountPseudo	= devfs_mount,
	.pfnOpen 		= devfs_open,
	.pfnOpenDir 	= devfs_opendir,
	//.pfnGetInode 	= devfs_get_inode,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pFilesystemIF = &oFilesystemInterface,
	},
	.eType = BT_HANDLE_T_FILESYSTEM,
};

static const BT_IF_HANDLE oDirHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDirIF = &oDirOperations,
	},
	.eType = BT_HANDLE_T_DIRECTORY,
};

static BT_ERROR devfs_init() {

	BT_ERROR Error;

	BT_HANDLE hFS = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hFS) {
		return Error;
	}

	BT_RegisterFilesystem(hFS);

	BT_Mount(NULL, "/dev/", "devfs", 0, NULL);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF devfs_init_tab = {
	.name = BT_MODULE_NAME,
	devfs_init,
};

#endif
