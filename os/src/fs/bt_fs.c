/**
 *	BitThunder - File-system Manager.
 *
 **/
#include <bitthunder.h>
#include <string.h>

BT_DEF_MODULE_NAME			("Filesystem Manager")
BT_DEF_MODULE_DESCRIPTION	("Filesystem Mountpoint management")
BT_DEF_MODULE_AUTHOR	  	("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_LIST g_oFileSystems = {0};
static BT_LIST g_oMountPoints = {0};

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

typedef struct _BT_FILESYSTEM {
	BT_LIST_ITEM oItem;
	BT_HANDLE	 hFS;
} BT_FILESYSTEM;

typedef struct _BT_MOUNTPOINT {
	BT_LIST_ITEM 	oItem;
	BT_HANDLE 		hMount;
	BT_i8 		   *szpPath;
	BT_FILESYSTEM  *pFS;
} BT_MOUNTPOINT;


BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS) {
	if(hFS->h.pIf->eType != BT_HANDLE_T_FILESYSTEM) {
		return BT_ERR_GENERIC;
	}

	BT_FILESYSTEM *pFilesystem = BT_kMalloc(sizeof(BT_FILESYSTEM));
	if(!pFilesystem) {
		return BT_ERR_NO_MEMORY;
	}

	pFilesystem->hFS = hFS;

	BT_ListAddItem(&g_oFileSystems, &pFilesystem->oItem);

	return BT_ERR_NONE;
}

static BT_MOUNTPOINT *GetMountPoint(const BT_i8 *szpPath) {
	BT_MOUNTPOINT *pMountPoint = (BT_MOUNTPOINT *) BT_ListGetHead(&g_oMountPoints);
	while(pMountPoint) {
		BT_i8 *common = (BT_i8 *) strstr(szpPath, pMountPoint->szpPath);
		if(common == szpPath) {	// Ensure the common section is from root of szpPath!
			return pMountPoint;
		}

		pMountPoint = (BT_MOUNTPOINT *) BT_ListGetNext(&pMountPoint->oItem);
	}

	return pMountPoint;
}

BT_ERROR BT_Mount(BT_HANDLE hVolume, const BT_i8 *szpPath) {
	BT_ERROR Error;

	if(hVolume->h.pIf->eType != BT_HANDLE_T_VOLUME &&
	   hVolume->h.pIf->eType != BT_HANDLE_T_PARTITION &&
	   hVolume->h.pIf->eType != BT_HANDLE_T_BLOCK &&
	   hVolume->h.pIf->eType != BT_HANDLE_T_INODE) {

		return BT_ERR_GENERIC;
	}

	BT_HANDLE hMount = NULL;

	// Is path already mounted?
	if(GetMountPoint(szpPath)) {
		return BT_ERR_GENERIC;
	}

	// Can any file-system mount this partition?
	BT_FILESYSTEM *pFilesystem = (BT_FILESYSTEM *) BT_ListGetHead(&g_oFileSystems);
	while(pFilesystem) {
		const BT_IF_FS *pFs = pFilesystem->hFS->h.pIf->oIfs.pFilesystemIF;
		hMount = pFs->pfnMount(pFilesystem->hFS, hVolume, &Error);
		if(hMount) {
			break;
		}

		pFilesystem = (BT_FILESYSTEM *) BT_ListGetNext(&pFilesystem->oItem);
	}

	if(!hMount) {
		BT_kPrint("FS: Could not mount volume, no compatible filesystem.");
		return BT_ERR_GENERIC;
	}

	// A filesystem was able to mount the volume, now we can handle this under our own namespace.
	BT_MOUNTPOINT *pMountPoint = (BT_MOUNTPOINT *) BT_kMalloc(sizeof(BT_MOUNTPOINT));
	if(!pMountPoint) {
		Error = BT_ERR_GENERIC;
		goto err_unmount_out;
	}

	pMountPoint->hMount 	= hMount;
	pMountPoint->pFS 		= pFilesystem;
	pMountPoint->szpPath 	= BT_kMalloc(strlen(szpPath) + 1);
	strcpy(pMountPoint->szpPath, szpPath);

	BT_ListAddItem(&g_oMountPoints, &pMountPoint->oItem);

	return BT_ERR_NONE;

err_unmount_out:
	pFilesystem->hFS->h.pIf->oIfs.pFilesystemIF->pfnUnmount(hMount);

	return Error;
}

static const BT_i8 *get_relative_path(BT_MOUNTPOINT *pMount, const BT_i8 *szpPath) {
	BT_u32 mountlen = strlen(pMount->szpPath);
	return szpPath+mountlen-1;
}

#define BT_FS_MODE_READ				0x01
#define	BT_FS_MODE_WRITE			0x02
#define BT_FS_MODE_APPEND			0x04
#define	BT_FS_MODE_CREATE			0x08
#define BT_FS_MODE_TRUNCATE			0x10

BT_u32 get_mode_flags(BT_i8 *mode) {
	BT_u32 ulModeFlags = 0x00;
	while(*mode) {
		switch(*mode) {
			case 'r':	// Allow Read
			case 'R':
				ulModeFlags |= BT_FS_MODE_READ;
				break;

			case 'w':	// Allow Write
			case 'W':
				ulModeFlags |= BT_FS_MODE_WRITE;
				ulModeFlags |= BT_FS_MODE_CREATE;	// Create if not exist.
				ulModeFlags |= BT_FS_MODE_TRUNCATE;
				break;

			case 'a':	// Append new writes to the end of the file.
			case 'A':
				ulModeFlags |= BT_FS_MODE_WRITE;
				ulModeFlags |= BT_FS_MODE_APPEND;
				ulModeFlags |= BT_FS_MODE_CREATE;	// Create if not exist.
				break;

			case '+':	// Update the file, don't Append!
				ulModeFlags |= BT_FS_MODE_READ;	// RW Mode
				ulModeFlags |= BT_FS_MODE_WRITE;	// RW Mode
				break;

			/*case 'D':	// Internal use only!
				ModeBits |= FF_MODE_DIR;
				break;*/

			default:	// b|B flags not supported (Binary mode is native anyway).
				break;
		}
		mode++;
	}

	return ulModeFlags;
}

BT_HANDLE BT_Open(const BT_i8 *szpPath, BT_i8 *mode, BT_ERROR *pError) {
	BT_MOUNTPOINT *pMount = GetMountPoint(szpPath);
	if(!pMount) {
		return NULL;
	}

	const BT_i8 *path = get_relative_path(pMount, szpPath);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	return pFS->pfnOpen(pMount->hMount, path, get_mode_flags(mode), pError);
}

BT_ERROR BT_MkDir(BT_i8 *szpPath) {
	BT_MOUNTPOINT *pMount = GetMountPoint(szpPath);
	if(!pMount) {
		return BT_ERR_GENERIC;
	}

	const BT_i8 *path = get_relative_path(pMount, szpPath);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	return pFS->pfnMkDir(pMount->hMount, path);
}

BT_HANDLE BT_OpenDir(const BT_i8 *szpPath, BT_ERROR *pError) {
	BT_MOUNTPOINT *pMount = GetMountPoint(szpPath);
	if(!pMount) {
		return NULL;
	}

	const BT_i8 *path = get_relative_path(pMount, szpPath);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	return pFS->pfnOpenDir(pMount->hMount, path, pError);
}

BT_HANDLE BT_GetInode(const BT_i8 *szpPath, BT_ERROR *pError) {
	BT_MOUNTPOINT *pMount = GetMountPoint(szpPath);
	if(!pMount) {
		return NULL;
	}

	const BT_i8 *path = get_relative_path(pMount, szpPath);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	return pFS->pfnGetInode(pMount->hMount, path, pError);
}

static BT_ERROR bt_fs_init() {
	BT_ListInit(&g_oFileSystems);
	BT_ListInit(&g_oMountPoints);
	return BT_ERR_NONE;
}

BT_MODULE_INIT_0_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_fs_init,
};
