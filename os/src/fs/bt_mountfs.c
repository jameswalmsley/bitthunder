/**
 * BitThunder - Mountpoint Filesystem - The ROOT ("/")
 *
 **/

#include <bitthunder.h>
#include <collections/bt_list.h>
#include <bt_struct.h>
#include <string.h>

BT_DEF_MODULE_NAME			("mountfs")
BT_DEF_MODULE_DESCRIPTION	("Mount-point/root filesystem pseudo fs")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

extern struct bt_list_head g_mountpoints;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
};

struct mountfs_dir {
	BT_HANDLE_HEADER 	h;
	BT_ul32 			ulCurrentEntry;
};

static const BT_IF_HANDLE oHandleInterface;
static const BT_IF_HANDLE oDirHandleInterface;

static BT_HANDLE mountfs_mount(BT_HANDLE hFS, const void *data, BT_ERROR *pError) {
	BT_HANDLE hMount = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	return hMount;
}

static BT_HANDLE mountfs_opendir(BT_HANDLE hMount, const BT_i8 *szpPath, BT_ERROR *pError) {
	BT_HANDLE hDir = BT_CreateHandle(&oDirHandleInterface, sizeof(struct mountfs_dir), pError);
	return hDir;
}

static BT_ERROR mountfs_readdir(BT_HANDLE hDir, BT_DIRENT *pDirent) {

	struct mountfs_dir *pDir = (struct mountfs_dir *) hDir;
	BT_u32 i = 0;


	struct bt_list_head *pos;

	bt_list_for_each(pos, &g_mountpoints) {
		BT_MOUNTPOINT *pMountPoint = (BT_MOUNTPOINT *) pos;
		if(i++ == pDir->ulCurrentEntry) {
			pDirent->szpName = pMountPoint->szpName;
			pDirent->ullFileSize = 0;
			pDirent->attr = BT_ATTR_DIR;
			pDir->ulCurrentEntry += 1;
			return BT_ERR_NONE;
		}
	}

	return BT_ERR_GENERIC;
};

static const BT_IF_DIR oDirOperations = {
	.pfnReadDir = mountfs_readdir,
};

static const BT_IF_FS oFilesystemInterface = {
	.ulFlags 		= BT_FS_FLAG_NODEV,
	.name 			= "mountfs",
	.pfnMountPseudo	= mountfs_mount,
	.pfnOpenDir 	= mountfs_opendir,
};

static const BT_IF_HANDLE oDirHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDirIF = &oDirOperations,
	},
	.eType = BT_HANDLE_T_DIRECTORY,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pFilesystemIF = &oFilesystemInterface,
	},
	.eType = BT_HANDLE_T_FILESYSTEM,
};

static BT_ERROR mountfs_init() {
	BT_ERROR Error;

	BT_HANDLE hFS = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hFS) {
		return Error;
	}

	BT_RegisterFilesystem(hFS);

	BT_Mount(NULL, "/", "mountfs", 0, NULL);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF mountfs_init_tab = {
	.name = BT_MODULE_NAME,
	mountfs_init,
};
