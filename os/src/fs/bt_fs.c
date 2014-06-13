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

#ifdef BT_CONFIG_PROCESS_CWD
static BT_ERROR to_absolute_path(BT_i8 *buf, BT_u32 len, const BT_i8 *path, BT_BOOL isDir) {

	BT_u32 path_len = strlen(path);
	BT_u32 cwd_len = strlen(curtask->cwd);

	if(path_len + cwd_len >= BT_PATH_MAX) {
		return BT_ERR_GENERIC;
	}

	if(path[0] != '/' && path[0] != '\\') {
		strncpy(buf, curtask->cwd, len);
		if(buf[cwd_len-1] != '/') {
			strcat(buf, "/");
		}

		strcat(buf, path);
	} else {
		strncpy(buf, path, len);
	}

	if(isDir && path_len > 1) {
		strcat(buf, "/");
	}

	BT_i8 *p = buf;
	while(*p) {
		if(*p == '\\') {
			*p = '/';
		}
		p++;
	}

	return BT_ERR_NONE;
}
#endif

BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS) {
	if(BT_HANDLE_TYPE(hFS) != BT_HANDLE_T_FILESYSTEM) {
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
BT_EXPORT_SYMBOL(BT_RegisterFilesystem);

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

BT_MOUNTPOINT *BT_GetMountPoint(const BT_i8 *szpPath) {

	BT_MOUNTPOINT *pTarget = NULL;
	BT_u32 path_len = strlen(szpPath);
	BT_u32 target_len = 0;

	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_mountpoints) {
		BT_MOUNTPOINT *pMountPoint = (BT_MOUNTPOINT *) pos;
		BT_i8 *common = (BT_i8 *) strstr(szpPath, pMountPoint->szpPath);
		if(common == szpPath) {	// Ensure the common section is from root of szpPath!
			BT_u32 mount_len = strlen(pMountPoint->szpPath);
			if(mount_len > path_len) {
				continue;
			}

			if(!pTarget) {
				pTarget = pMountPoint;
				target_len = strlen(pTarget->szpPath);
			} else {
				if(mount_len > target_len) {
					pTarget = pMountPoint;
					target_len = strlen(pTarget->szpPath);
				}
			}

			if(mount_len == path_len) {
				break;
			}
		}
	}

	return pTarget;
}
BT_EXPORT_SYMBOL(BT_GetMountPoint);

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

		pMountPoint->hMount 	= hMount;
		pMountPoint->hVolume 	= NULL;
		pMountPoint->pFS 		= fs;
		pMountPoint->szpPath 	= BT_kMalloc(i+1);
		strncpy(pMountPoint->szpPath, target, i);
		pMountPoint->szpPath[i] = 0;

		bt_list_add(&pMountPoint->item, &g_mountpoints);

		return BT_ERR_NONE;
	}

	BT_HANDLE hVolume = BT_Open(src, 0, &Error);
	if(!hVolume) {
		// -- !
	}

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
	pMountPoint->hVolume	= hVolume;
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
BT_EXPORT_SYMBOL(BT_Mount);

static const BT_i8 *get_relative_path(BT_MOUNTPOINT *pMount, const BT_i8 *szpPath) {
	BT_u32 mountlen = strlen(pMount->szpPath);
	if(mountlen > 1) {
		return szpPath+mountlen;
	}

	return szpPath;
}

BT_u32 BT_GetModeFlags(const BT_i8 *mode) {
	BT_u32 ulModeFlags = 0x00;

	if(!mode) {
		return 0;
	}

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
			ulModeFlags |= BT_FS_MODE_READ;		// RW Mode
			ulModeFlags |= BT_FS_MODE_WRITE;	// RW Mode
			break;

		default:	// b|B flags not supported (Binary mode is native anyway).
			break;
		}
		mode++;
	}

	return ulModeFlags;
}
BT_EXPORT_SYMBOL(BT_GetModeFlags);

BT_HANDLE BT_Open(const BT_i8 *szpPath, BT_u32 mode, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE h = NULL;
	BT_i8 *absolute_path = (BT_i8 *) szpPath;

#ifdef BT_CONFIG_PROCESS_CWD
	absolute_path = BT_kMalloc(BT_PATH_MAX);
	if(!absolute_path) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	Error = to_absolute_path(absolute_path, BT_PATH_MAX, szpPath, BT_FALSE);
	if(Error) {
		goto err_free_out;
	}
#endif

	BT_MOUNTPOINT *pMount = BT_GetMountPoint(absolute_path);
	if(!pMount) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	const BT_i8 *path = get_relative_path(pMount, absolute_path);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	h = pFS->pfnOpen(pMount->hMount, path, mode, pError);

err_free_out:
#ifdef BT_CONFIG_PROCESS_CWD
	BT_kFree(absolute_path);
err_out:
#endif

	if(pError) {
		*pError = Error;
	}

	return h;
}
BT_EXPORT_SYMBOL(BT_Open);

BT_ERROR BT_MkDir(const BT_i8 *szpPath) {
	BT_MOUNTPOINT *pMount = BT_GetMountPoint(szpPath);
	if(!pMount) {
		return BT_ERR_GENERIC;
	}

	const BT_i8 *path = get_relative_path(pMount, szpPath);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	return pFS->pfnMkDir(pMount->hMount, path);
}
BT_EXPORT_SYMBOL(BT_MkDir);

BT_ERROR BT_RmDir(const BT_i8 *szpPath) {
	BT_ERROR Error = BT_ERR_NONE;

	BT_i8 *absolute_path = (BT_i8 *) szpPath;

#ifdef BT_CONFIG_PROCESS_CWD
	absolute_path = BT_kMalloc(BT_PATH_MAX);
	if(!absolute_path) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	Error = to_absolute_path(absolute_path, BT_PATH_MAX, szpPath, BT_FALSE);
	if(Error) {
		goto err_free_out;
	}
#endif

	BT_MOUNTPOINT *pMount = BT_GetMountPoint(absolute_path);
	if(!pMount) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	const BT_i8 *path = get_relative_path(pMount, absolute_path);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	Error = pFS->pfnRmDir(pMount->hMount, path);

err_free_out:
#ifdef BT_CONFIG_PROCESS_CWD
	BT_kFree(absolute_path);
err_out:
#endif

	return Error;
}
BT_EXPORT_SYMBOL(BT_RmDir);

BT_HANDLE BT_OpenDir(const BT_i8 *szpPath, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE h = NULL;

	BT_i8 *absolute_path = (BT_i8 *) szpPath;

#ifdef BT_CONFIG_PROCESS_CWD
	absolute_path = BT_kMalloc(BT_PATH_MAX);
	if(!absolute_path) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	Error = to_absolute_path(absolute_path, BT_PATH_MAX, szpPath, BT_TRUE);
	if(Error) {
		goto err_free_out;
	}
#endif

	BT_MOUNTPOINT *pMount = BT_GetMountPoint(absolute_path);
	if(!pMount) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	const BT_i8 *path = get_relative_path(pMount, absolute_path);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;

	h =  pFS->pfnOpenDir(pMount->hMount, path, pError);

err_free_out:
#ifdef BT_CONFIG_PROCESS_CWD
	BT_kFree(absolute_path);
err_out:
#endif

	if(pError) {
		*pError = Error;
	}

	return h;
}
BT_EXPORT_SYMBOL(BT_OpenDir);

BT_HANDLE BT_GetInode(const BT_i8 *szpPath, BT_ERROR *pError) {

	BT_ERROR 	Error = BT_ERR_NONE;
	BT_HANDLE 	h = NULL;

	BT_i8 *absolute_path = (BT_i8 *) szpPath;

#ifdef BT_CONFIG_PROCESS_CWD
	absolute_path = BT_kMalloc(BT_PATH_MAX);
	if(!absolute_path) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	Error = to_absolute_path(absolute_path, BT_PATH_MAX, szpPath, BT_FALSE);
	if(Error) {
		goto err_free_out;
	}
#endif

	BT_MOUNTPOINT *pMount = BT_GetMountPoint(absolute_path);
	if(!pMount) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	const BT_i8 *path = get_relative_path(pMount, absolute_path);

	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	if(pFS->pfnGetInode) {
		h = pFS->pfnGetInode(pMount->hMount, path, pError);
	}

err_free_out:
#ifdef BT_CONFIG_PROCESS_CWD
	BT_kFree(absolute_path);
err_out:
#endif

	if(pError) {
		*pError = Error;
	}

	return h;
}
BT_EXPORT_SYMBOL(BT_GetInode);

BT_ERROR BT_Remove(const BT_i8 *szpPath) {

	BT_HANDLE h;
	BT_ERROR Error;
	h = BT_GetInode(szpPath, &Error);
	if(!h) {
		return BT_ERR_GENERIC;
	}

	BT_INODE inode;
	Error = BT_ReadInode(h, &inode);
	BT_CloseHandle(h);

	if(inode.attr & BT_ATTR_DIR) {
		return BT_RmDir(szpPath);
	}

	return BT_Unlink(szpPath);
}
BT_EXPORT_SYMBOL(BT_Remove);

BT_ERROR BT_Unlink(const BT_i8 *szpPath) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_i8 *absolute_path = (BT_i8 *) szpPath;

#ifdef BT_CONFIG_PROCESS_CWD
	absolute_path = BT_kMalloc(BT_PATH_MAX);
	if(!absolute_path) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	Error = to_absolute_path(absolute_path, BT_PATH_MAX, szpPath, BT_FALSE);
	if(Error) {
		goto err_free_out;
	}
#endif

	BT_MOUNTPOINT *pMount = BT_GetMountPoint(absolute_path);
	if(!pMount) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	const BT_i8 *path = get_relative_path(pMount, absolute_path);
	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;

	Error = pFS->pfnUnlink(pMount->hMount, path);

err_free_out:
#ifdef BT_CONFIG_PROCESS_CWD
	BT_kFree(absolute_path);
err_out:
#endif

	return Error;
}
BT_EXPORT_SYMBOL(BT_Unlink);

BT_ERROR BT_Rename(const BT_i8 *szpPathA, const BT_i8 *szpPathB) {
	BT_MOUNTPOINT *pMountA = BT_GetMountPoint(szpPathA);
	if(!pMountA) {
		return BT_ERR_GENERIC;
	}

	BT_MOUNTPOINT *pMountB = BT_GetMountPoint(szpPathB);
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
BT_EXPORT_SYMBOL(BT_Rename);

#ifdef BT_CONFIG_PROCESS_CWD
BT_ERROR BT_GetCwd(BT_i8 *buf, BT_u32 len) {
	BT_u32 i = strlen(curtask->cwd);
	if(i >= len) {
		return BT_ERR_GENERIC;
	}

	strcpy(buf, curtask->cwd);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_GetCwd);

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
BT_EXPORT_SYMBOL(BT_ChDir);
#endif

BT_MOUNTPOINT *BT_GetNextMountPoint(BT_MOUNTPOINT *pMount) {
	BT_MOUNTPOINT *ret = NULL;

	if(!pMount) {
		ret = (BT_MOUNTPOINT *) g_mountpoints.next;
	} else {
		ret = (BT_MOUNTPOINT *) pMount->item.next;
	}

	if(ret == (BT_MOUNTPOINT *) &g_mountpoints) {
		ret = NULL;
	}

	return ret;
}
BT_EXPORT_SYMBOL(BT_GetNextMountPoint);

BT_ERROR BT_GetMountFSInfo(BT_MOUNTPOINT *pMount, struct bt_fsinfo *fsinfo) {
	const BT_IF_FS *pFS = pMount->pFS->hFS->h.pIf->oIfs.pFilesystemIF;
	if(pFS->pfnInfo) {
		return pFS->pfnInfo(pMount->hMount, fsinfo);
	}

	return BT_ERR_UNIMPLEMENTED;
}
BT_EXPORT_SYMBOL(BT_GetMountFSInfo);

BT_ERROR BT_GetFilesystemInfo(const BT_i8 *szpPath, struct bt_fsinfo *fsinfo) {
	BT_MOUNTPOINT *pMount = BT_GetMountPoint(szpPath);
	if(!pMount) {
		return BT_ERR_INVALID_VALUE;
	}

	return BT_GetMountFSInfo(pMount, fsinfo);
}
BT_EXPORT_SYMBOL(BT_GetFilesystemInfo);
