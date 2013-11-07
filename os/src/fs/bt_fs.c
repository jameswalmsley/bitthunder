/**
 *	BitThunder - File-system Manager.
 *
 **/
#include <bitthunder.h>
#include <string.h>
#include <collections/bt_list.h>

BT_DEF_MODULE_NAME			("Filesystem Manager")
BT_DEF_MODULE_DESCRIPTION	("Filesystem Mountpoint management")
BT_DEF_MODULE_AUTHOR	  	("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_LIST_HEAD(g_filesystems);
BT_LIST_HEAD(g_mountpoints);

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS) {
	if(hFS->h.pIf->eType != BT_HANDLE_T_FILESYSTEM) {
		return BT_ERR_GENERIC;
	}

	BT_FILESYSTEM *pFilesystem = BT_kMalloc(sizeof(BT_FILESYSTEM));
	if(!pFilesystem) {
		return BT_ERR_NO_MEMORY;
	}

	pFilesystem->hFS = hFS;

	bt_list_add(&pFilesystem->item, &g_filesystems);

	return BT_ERR_NONE;
}

static BT_FILESYSTEM *getfs(const BT_i8 *name) {
	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_filesystems) {
		BT_FILESYSTEM *pFS = (BT_FILESYSTEM *) pos;
		if(!strcmp(name, pFS->hFS->h.pIf->oIfs.pFilesystemIF->name)) {
			return pFS;
		}
	}

	return NULL;
}

static BT_MOUNTPOINT *find_mountpoint(const BT_i8 *szpPath, BT_u32 len) {
	struct bt_list_head *pos;

	bt_list_for_each(pos, &g_mountpoints) {
		BT_MOUNTPOINT *pMountPoint = (BT_MOUNTPOINT *) pos;
		if(!strncmp(szpPath, pMountPoint->szpPath, len) && strlen(pMountPoint->szpPath) == len) {
			return pMountPoint;
		}
	}

	return NULL;
}

static BT_MOUNTPOINT *GetMountPoint(const BT_i8 *szpPath) {

	BT_MOUNTPOINT *pTarget = NULL;

	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_mountpoints) {
		BT_MOUNTPOINT *pMountPoint = (BT_MOUNTPOINT *) pos;
		BT_i8 *common = (BT_i8 *) strstr(szpPath, pMountPoint->szpPath);
		if(common == szpPath) {	// Ensure the common section is from root of szpPath!
			if(!pTarget) {
				pTarget = pMountPoint;
			} else {
				if(strlen(pMountPoint->szpPath) > strlen(pTarget->szpPath)) {
					pTarget = pMountPoint;
				}
			}
		}
	}

	return pTarget;
}

BT_ERROR BT_Mount(const BT_i8 *src, const BT_i8 *target, const BT_i8 *filesystem, BT_u32 mountflags, const void *data) {
	BT_ERROR Error;

	if(!target) {
		return BT_ERR_GENERIC;
	}

	BT_FILESYSTEM *fs = NULL;

	if(filesystem) {
		fs = getfs(filesystem);
		if(!fs) {
			return BT_ERR_GENERIC;
		}
	}

	if(!src && !filesystem) {
		return BT_ERR_GENERIC;
	}

	if(!target) {	
		return BT_ERR_GENERIC;
	}

	BT_u32 i = strlen(target);
	if(i>1) {
		if(target[i-1] == '/' || target[i-1] == '\\') {
			i -= 1;
		}
	}

	BT_HANDLE hMount;
	BT_MOUNTPOINT *pMountPoint = find_mountpoint(target, i);
	if(pMountPoint) {
		return BT_ERR_GENERIC;
	}

	if(!src) {
		const BT_IF_FS *pFs = fs->hFS->h.pIf->oIfs.pFilesystemIF;

		if(!fs->hFS->h.pIf->oIfs.pFilesystemIF->ulFlags & BT_FS_FLAG_NODEV) {
			return BT_ERR_GENERIC;
		}

		hMount = pFs->pfnMountPseudo(fs->hFS, data, &Error);
		if(!hMount) {
			return BT_ERR_GENERIC;
		}

		pMountPoint = BT_kMalloc(sizeof(BT_MOUNTPOINT));
		if(!pMountPoint) {
			return BT_ERR_NO_MEMORY;
		}

		pMountPoint->hMount = hMount;
		pMountPoint->pFS = fs;
		pMountPoint->szpPath = BT_kMalloc(i+1);
		strncpy(pMountPoint->szpPath, target, i);
		pMountPoint->szpPath[i] = 0;

		bt_list_add(&pMountPoint->item, &g_mountpoints);

		return BT_ERR_NONE;
	}

	BT_HANDLE hVolume = BT_Open(src, 0, &Error);

	if(fs) {
		const BT_IF_FS *pFs = fs->hFS->h.pIf->oIfs.pFilesystemIF;

		hMount = pFs->pfnMount(fs->hFS, hVolume, data, &Error);
		if(!hMount) {
			return BT_ERR_NO_MEMORY;
		}
	} else {
		struct bt_list_head *pos;
		bt_list_for_each(pos, &g_filesystems) {
			fs = (BT_FILESYSTEM *) pos;
			const BT_IF_FS *pFs = fs->hFS->h.pIf->oIfs.pFilesystemIF;
			if(!(pFs->ulFlags & BT_FS_FLAG_NODEV)) {
				hMount = pFs->pfnMount(fs->hFS, hVolume, data, &Error);
				if(hMount) break;
			}
		}
		if(!hMount) {
			return BT_ERR_GENERIC;
		}
	}

	// A filesystem was able to mount the volume, now we can handle this under our own namespace.
	pMountPoint = (BT_MOUNTPOINT *) BT_kMalloc(sizeof(BT_MOUNTPOINT));
	if(!pMountPoint) {
		Error = BT_ERR_GENERIC;
		goto err_unmount_out;
	}

	pMountPoint->hMount 	= hMount;
	pMountPoint->pFS 		= fs;
	pMountPoint->szpPath  	= BT_kMalloc(i+1);
	strncpy(pMountPoint->szpPath, target, i);
	pMountPoint->szpPath[i] = 0;

	bt_list_add(&pMountPoint->item, &g_mountpoints);

	return BT_ERR_NONE;

err_unmount_out:
	fs->hFS->h.pIf->oIfs.pFilesystemIF->pfnUnmount(hMount);

	return Error;
}

static const BT_i8 *get_relative_path(BT_MOUNTPOINT *pMount, const BT_i8 *szpPath) {
	BT_u32 mountlen = strlen(pMount->szpPath) + 1;
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

BT_ERROR BT_MkDir(const BT_i8 *szpPath) {
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

BT_ERROR BT_Remove(const BT_i8 *szpPath) {
	BT_MOUNTPOINT *pMount = GetMountPoint(szpPath);
	if(!pMount) {
		return BT_ERR_GENERIC;
	}

	const BT_i8 *path = get_relative_path(pMount, szpPath);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	return pFS->pfnRemove(pMount->hMount, path);
}

BT_ERROR BT_Rename(const BT_i8 *szpPathA, const BT_i8 *szpPathB) {
	BT_MOUNTPOINT *pMountA = GetMountPoint(szpPathA);
	if(!pMountA) {
		return BT_ERR_GENERIC;
	}

	BT_MOUNTPOINT *pMountB = GetMountPoint(szpPathB);
	if(!pMountB) {
		return BT_ERR_GENERIC;
	}

	if (pMountA != pMountB) {
		return BT_ERR_GENERIC;
	}

	const BT_i8 *pathA = get_relative_path(pMountA, szpPathA);
	const BT_i8 *pathB = get_relative_path(pMountB, szpPathB);

	const BT_IF_FS *pFS = pMountA->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	return pFS->pfnRename(pMountA->hMount, pathA, pathB);
}

BT_ERROR BT_GetCwd(BT_i8 *buf, BT_u32 len) {
	BT_u32 i = strlen(curtask->cwd);
	if(i >= len) {
		return BT_ERR_GENERIC;
	}

	strcpy(buf, curtask->cwd);

	return BT_ERR_NONE;
}

BT_ERROR BT_ChDir(const BT_i8 *path) {
	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 len = strlen(path);
	if(len >= BT_PATH_MAX) {
		return BT_ERR_GENERIC;
	}

	BT_HANDLE hDir = BT_OpenDir(path, &Error);
	if(!hDir) {
		return BT_ERR_GENERIC;
	}

	BT_CloseHandle(hDir);

	strncpy(curtask->cwd, path, BT_PATH_MAX);

	BT_u32 i;
	for(i = 0; i < len; i++) {
		if(curtask->cwd[i] == '\\') {
			curtask->cwd[i] = '/';
		}
	}

	if(curtask->cwd[len-1] == '/' && len > 1) {
		curtask->cwd[len-1] = '\0';
	}

	return BT_ERR_NONE;
}
