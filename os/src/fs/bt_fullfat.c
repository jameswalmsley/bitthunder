/**
 *	FullFAT Filesystem - Filesystem plugin for BitThunder.
 *
 **/

#include <bitthunder.h>
#include <fs/bt_fs.h>
#include <volumes/bt_volume.h>
#include "ff_headers.h"

BT_DEF_MODULE_NAME			("FullFAT Filesystem")
BT_DEF_MODULE_DESCRIPTION	("BitThunder FS plugin for FullFAT")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

typedef struct _BT_FF_MOUNT {
	BT_HANDLE_HEADER 	h;
	FF_Disk_t			oFFDisk;
	FF_IOManager_t	   *pIoman;
	BT_HANDLE 			hVolume;
	void 			   *pBlockCache;
	BT_HANDLE 			hSem;
} BT_FF_MOUNT;

typedef struct _BT_FF_FILE {
	BT_HANDLE_HEADER h;
	BT_FF_MOUNT		*pMount;
	FF_FILE			*pFile;
} BT_FF_FILE;

typedef struct _BT_FF_DIR {
	BT_HANDLE_HEADER 	h;
	BT_FF_MOUNT		   *pMount;
	FF_DirEnt_t		    oDirent;
	BT_u32				ulCurrentEntry;
} BT_FF_DIR;

typedef struct _BT_FF_INODE {
	BT_HANDLE_HEADER	h;
	BT_FF_MOUNT		   *pMount;
	FF_DirEnt_t			oDirent;
} BT_FF_INODE;

static const BT_IF_HANDLE oHandleInterface;
static const BT_IF_HANDLE oFileHandleInterface;
static const BT_IF_HANDLE oDirHandleInterface;
static const BT_IF_HANDLE oInodeHandleInterface;

static int32_t fullfat_readblocks(uint8_t *pBuffer, uint32_t Address, uint32_t Count, FF_Disk_t *pDisk) {
	BT_FF_MOUNT *pMount = bt_container_of(pDisk, BT_FF_MOUNT, oFFDisk);
	BT_s32 retval = BT_VolumeRead(pMount->hVolume, Address, Count, pBuffer);
	if(retval < 0) {
		return FF_ERR_DRIVER_FATAL_ERROR;
	}

	return retval;
}

static int32_t fullfat_writeblocks(uint8_t *pBuffer, uint32_t Address, uint32_t Count, FF_Disk_t *pDisk) {
	BT_FF_MOUNT *pMount = bt_container_of(pDisk, BT_FF_MOUNT, oFFDisk);
	BT_s32 retval = BT_VolumeWrite(pMount->hVolume, Address, Count, pBuffer);
	if(retval < 0) {
		return FF_ERR_DRIVER_FATAL_ERROR;
	}

	return retval;
}

static BT_ERROR fullfat_format(BT_HANDLE hFS, BT_HANDLE hVolume) {

	BT_ERROR Error;
	FF_Error_t ffError;

	BT_BLOCK_GEOMETRY oGeom;
	BT_GetVolumeGeometry(hVolume, &oGeom);

	BT_FF_MOUNT oContext;
	oContext.pBlockCache = BT_kMalloc(oGeom.ulBlockSize*2);

	oContext.hSem = BT_kMutexCreate();
	oContext.hVolume = hVolume;

	FF_CreationParameters_t oFFParams = {
		.pucCacheMemory 	= oContext.pBlockCache,
		.ulMemorySize 		= oGeom.ulBlockSize*2,
		.ulSectorSize		= oGeom.ulBlockSize,
		.fnWriteBlocks		= fullfat_writeblocks,
		.fnReadBlocks		= fullfat_readblocks,
		.pxDisk				= &oContext.oFFDisk,
		.pvSemaphore		= oContext.hSem,
		.xBlockDeviceIsReentrant = 1,
	};

	oContext.pIoman = FF_CreateIOManager(&oFFParams, &ffError);
	if(!oContext.pIoman) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	oContext.oFFDisk.pxIOManager = oContext.pIoman;

	FF_FormatRegion(&oContext.oFFDisk, 0, 0, 0, oGeom.ulTotalBlocks);

err_free_out:
	BT_CloseHandle(oContext.hSem);

	return BT_ERR_NONE;
}

static BT_HANDLE fullfat_mount(BT_HANDLE hFS, BT_HANDLE hVolume, const void *data, BT_ERROR *pError) {

	FF_Error_t ffError;
	BT_ERROR Error = BT_ERR_GENERIC;
	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) BT_CreateHandle(&oHandleInterface, sizeof(BT_FF_MOUNT), pError);
	if(!pMount) {
		return NULL;
	}

	pMount->pBlockCache = BT_kMalloc(BT_CONFIG_FS_FULLFAT_CACHE_SIZE);
	if(!pMount->pBlockCache) {
		goto err_block_cache_free_out;
	}

	pMount->hVolume = hVolume;

	BT_BLOCK_GEOMETRY oGeom;

	BT_GetVolumeGeometry(hVolume, &oGeom);

	pMount->hSem = BT_kMutexCreate();

	FF_CreationParameters_t oFFParams = {
		.pucCacheMemory 	= pMount->pBlockCache,
		.ulMemorySize 		= BT_CONFIG_FS_FULLFAT_CACHE_SIZE,
		.ulSectorSize		= oGeom.ulBlockSize,
		.fnWriteBlocks		= fullfat_writeblocks,
		.fnReadBlocks		= fullfat_readblocks,
		.pxDisk				= &pMount->oFFDisk,
		.pvSemaphore		= pMount->hSem,
		.xBlockDeviceIsReentrant = 1,
	};

	pMount->pIoman = FF_CreateIOManager(&oFFParams, &ffError);
	if(!pMount->pIoman) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}
	
	pMount->oFFDisk.pxIOManager = pMount->pIoman;

	ffError = FF_Mount(&pMount->oFFDisk, 0);
	if(ffError) {
		BT_kPrint("fullfat_mount: %s", FF_GetErrMessage(ffError));
		goto err_mount_out;
	}

	return (BT_HANDLE) pMount;

err_mount_out:
	ffError = FF_DeleteIOManager(pMount->pIoman);

err_sem_free_out:
	BT_CloseHandle(pMount->hSem);

err_block_cache_free_out:
	BT_kFree(pMount->pBlockCache);

err_free_out:
	BT_DestroyHandle((BT_HANDLE)pMount);

	*pError = Error;
	return NULL;
}

static BT_ERROR fullfat_unmount(BT_HANDLE hMount) {
	return BT_ERR_NONE;
}

static BT_HANDLE fullfat_open(BT_HANDLE hMount, const BT_i8 *szpPath, BT_u32 ulModeFlags, BT_ERROR *pError) {

	FF_Error_t ffError;

	BT_FF_FILE *pFile = (BT_FF_FILE *) BT_CreateHandle(&oFileHandleInterface, sizeof(BT_FF_FILE), pError);
	if(!pFile) {
		return NULL;
	}

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;

	pFile->pFile = FF_Open(pMount->pIoman, szpPath, (uint8_t) ulModeFlags, &ffError);
	if(!pFile->pFile) {
		BT_DestroyHandle((BT_HANDLE)pFile);
		return NULL;
	}

	pFile->pMount = pMount;

	return (BT_HANDLE) pFile;
}

static BT_ERROR fullfat_unlink(BT_HANDLE hMount, const BT_i8 *szpPath) {

	FF_Error_t ffError;

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;

	ffError = FF_RmFile(pMount->pIoman, szpPath);

	return (BT_ERROR) ffError;
}

static BT_ERROR fullfat_rename(BT_HANDLE hMount, const BT_i8 *szpPathA, const BT_i8 *szpPathB) {

	FF_Error_t ffError;

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;

	ffError = FF_Move(pMount->pIoman, szpPathA, szpPathB, 1);

	return (BT_ERROR) ffError;
}

static BT_ERROR fullfat_info(BT_HANDLE hMount, struct bt_fsinfo *fsinfo) {

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;
	FF_Error_t ffError;

	fsinfo->total 		= FF_GetVolumeSize(pMount->pIoman);
	fsinfo->available 	= FF_GetFreeSize(pMount->pIoman, &ffError);

	return BT_ERR_NONE;
}

static BT_ERROR fullfat_file_cleanup(BT_HANDLE hFile) {

	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;
	FF_Close(pFile->pFile);

	return BT_ERR_NONE;
}

static BT_s32 fullfat_read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer) {
	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;
	int32_t sRead = FF_Read(pFile->pFile, 1, ulSize, (uint8_t *) pBuffer);
	return sRead;
}

static BT_s32 fullfat_write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer) {
	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;
	int32_t sWritten = FF_Write(pFile->pFile, 1, ulSize, (uint8_t *) pBuffer);
	return sWritten;
}

static BT_s32 fullfat_getc(BT_HANDLE hFile, BT_u32 ulFlags) {

	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;
	int32_t ret = FF_GetC(pFile->pFile);
	return ret;
}

static BT_ERROR fullfat_putc(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData) {

	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;

	int32_t ret = FF_PutC(pFile->pFile, (uint8_t) cData);

	return (BT_ERROR) ret;
}

static BT_ERROR fullfat_seek(BT_HANDLE hFile, BT_s64 ulOffset, BT_u32 whence) {
	int8_t Origin;
	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;

	if (whence==BT_SEEK_SET) {
		Origin = FF_SEEK_SET;
	}
	else if (whence==BT_SEEK_CUR) {
		Origin = FF_SEEK_CUR;
	}
	else {
		Origin = FF_SEEK_END;
	}

	FF_Error_t ret = FF_Seek(pFile->pFile, (int32_t) ulOffset, Origin);

	return (BT_ERROR) ret;
}

static BT_u64 fullfat_tell(BT_HANDLE hFile, BT_ERROR *pError) {
	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;
	if(pError) {
		*pError = BT_ERR_NONE;
	}
	return (BT_u64) FF_Tell(pFile->pFile);
}

static BT_BOOL fullfat_eof(BT_HANDLE hFile) {
	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;
	BaseType_t eof;

	eof=FF_isEOF(pFile->pFile);

	if (!eof) {
		return BT_FALSE;
	}

	return BT_TRUE;
}

static BT_ERROR fullfat_mkdir(BT_HANDLE hMount, const BT_i8 *szpPath) {

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;
	FF_Error_t ffError = FF_MkDir(pMount->pIoman, szpPath);
	if(ffError) {
		return BT_ERR_GENERIC;
	}

	return BT_ERR_NONE;
}

static BT_ERROR fullfat_rmdir(BT_HANDLE hMount, const BT_i8 *szpPath) {

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;
	FF_Error_t ffError = FF_RmDir(pMount->pIoman, szpPath);
	if(ffError) {
		return BT_ERR_GENERIC;
	}

	return BT_ERR_NONE;
}

static BT_HANDLE fullfat_opendir(BT_HANDLE hMount, const BT_i8 *szpPath, BT_ERROR *pError) {

	BT_FF_DIR *pDir = (BT_FF_DIR *) BT_CreateHandle(&oDirHandleInterface, sizeof(BT_FF_DIR), pError);
	if(!pDir) {
		*pError = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;
	FF_Error_t ffError = FF_FindFirst(pMount->pIoman, &pDir->oDirent, szpPath);
	if(ffError && FF_GETERROR(ffError) != FF_ERR_DIR_END_OF_DIR) {
		*pError = BT_ERR_GENERIC;
		goto err_free_out;
	}

	pDir->pMount = pMount;

	return (BT_HANDLE) pDir;

err_free_out:
	BT_DestroyHandle((BT_HANDLE)pDir);

err_out:
	return NULL;
}

static void ff_time_to_bt_time(BT_DATETIME *bttime, FF_SystemTime_t *fftime) {
	bttime->year 	= fftime->Year;
	bttime->month 	= (BT_u8) fftime->Month;
	bttime->day 	= (BT_u8) fftime->Day;
	bttime->hour 	= (BT_u8) fftime->Hour;
	bttime->min		= (BT_u8) fftime->Minute;
	bttime->second 	= (BT_u8) fftime->Second;
}

static void bt_time_to_ff_time(FF_SystemTime_t *fftime, BT_DATETIME *bttime) {
	fftime->Year 	= (uint16_t) bttime->year;
	fftime->Month	= (uint16_t) bttime->month;
	fftime->Day		= (uint16_t) bttime->day;
	fftime->Hour	= (uint16_t) bttime->hour;
	fftime->Minute	= (uint16_t) bttime->min;
	fftime->Second	= (uint16_t) bttime->second;
}

static BT_ERROR fullfat_read_dir(BT_HANDLE hDir, BT_DIRENT *pDirent) {

	BT_FF_DIR *pDir = (BT_FF_DIR *) hDir;

	if(pDir->ulCurrentEntry) {
		FF_Error_t ffError = FF_FindNext(pDir->pMount->pIoman, &pDir->oDirent);
		if(ffError) {
			return BT_ERR_GENERIC;
		}
	}

	pDirent->szpName = pDir->oDirent.pcFileName;
	pDirent->ullFileSize = pDir->oDirent.ulFileSize;
	pDirent->attr = 0;
	if(pDir->oDirent.ucAttrib & FF_FAT_ATTR_DIR) {
		pDirent->attr |= BT_ATTR_DIR;
	}
	if(pDir->oDirent.ucAttrib & FF_FAT_ATTR_READONLY) {
		pDirent->attr |= BT_ATTR_READONLY;
	}
	if(pDir->oDirent.ucAttrib & FF_FAT_ATTR_HIDDEN) {
		pDirent->attr |= BT_ATTR_HIDDEN;
	}
	if(pDir->oDirent.ucAttrib & FF_FAT_ATTR_SYSTEM) {
		pDirent->attr |= BT_ATTR_SYSTEM;
	}
	if(pDir->oDirent.ucAttrib & FF_FAT_ATTR_ARCHIVE) {
		pDirent->attr |= BT_ATTR_ARCHIVE;
	}

	ff_time_to_bt_time(&pDirent->ctime, &pDir->oDirent.xCreateTime);
	ff_time_to_bt_time(&pDirent->atime, &pDir->oDirent.xAccessedTime);
	ff_time_to_bt_time(&pDirent->mtime, &pDir->oDirent.xModifiedTime);

	pDir->ulCurrentEntry += 1;

	return BT_ERR_NONE;
}

static BT_HANDLE fullfat_open_inode(BT_HANDLE hMount, const BT_i8 *szpPath, BT_ERROR *pError) {

	BT_FF_INODE *pInode = (BT_FF_INODE *) BT_CreateHandle(&oInodeHandleInterface, sizeof(BT_FF_INODE), pError);
	if(!pInode) {
		goto err_out;
	}

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;
	FF_Error_t ffError = FF_FindFirst(pMount->pIoman, &pInode->oDirent, szpPath);
	if(ffError) {
		goto err_free_out;
	}

	pInode->pMount = pMount;

	return (BT_HANDLE) pInode;

err_free_out:
	BT_DestroyHandle((BT_HANDLE)pInode);

err_out:
	return NULL;
}

static BT_ERROR fullfat_read_inode(BT_HANDLE hInode, BT_INODE *pInode) {

	BT_FF_INODE *phInode = (BT_FF_INODE *) hInode;
	pInode->ullFileSize = phInode->oDirent.ulFileSize;
	pInode->attr = 0;
	if(phInode->oDirent.ucAttrib & FF_FAT_ATTR_DIR) {
		pInode->attr |= BT_ATTR_DIR;
	}

	ff_time_to_bt_time(&pInode->ctime, &phInode->oDirent.xCreateTime);
	ff_time_to_bt_time(&pInode->atime, &phInode->oDirent.xAccessedTime);
	ff_time_to_bt_time(&pInode->mtime, &phInode->oDirent.xModifiedTime);

	return BT_ERR_NONE;
}

static BT_ERROR fullfat_utime(BT_HANDLE hMount, const BT_i8 *szpPath, BT_DATETIME *mtime, BT_DATETIME *atime) {

	BT_ERROR Error = BT_ERR_NONE;
	FF_Error_t ffError;
	BT_FF_FILE *pFile = (BT_FF_FILE *) BT_CreateHandle(&oFileHandleInterface, sizeof(BT_FF_FILE), &Error);
	if(!pFile) {
		goto err_out;
	}

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;

	pFile->pFile = FF_Open(pMount->pIoman, szpPath, BT_GetModeFlags("a"), &ffError);
	if(!pFile->pFile) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	FF_SystemTime_t Time;

	if (mtime) {
		bt_time_to_ff_time(&Time, mtime);
		ffError = FF_SetFileTime(pFile->pFile, &Time, ETimeMod);
		if (ffError) {
			FF_Close(pFile->pFile);
			Error = BT_ERR_GENERIC;
			goto err_free_out;
		}
	}

	if (atime) {
		bt_time_to_ff_time(&Time, atime);
		ffError = FF_SetFileTime(pFile->pFile, &Time, ETimeAccess);
		if (ffError) {
			FF_Close(pFile->pFile);
			Error = BT_ERR_GENERIC;
			goto err_free_out;
		}
	}

	FF_Close(pFile->pFile);

err_free_out:
	BT_DestroyHandle((BT_HANDLE)pFile);

err_out:
	return Error;
}

static const BT_IF_DIR oDirOperations = {
	.pfnReadDir = fullfat_read_dir,
};

static const BT_IF_INODE oInodeOperations = {
	.pfnReadInode = fullfat_read_inode,
};

static const BT_IF_FILE oFileOperations = {
	.pfnRead 	= fullfat_read,
	.pfnWrite 	= fullfat_write,
	.pfnGetC	= fullfat_getc,
	.pfnPutC	= fullfat_putc,
	.pfnSeek	= fullfat_seek,
	.pfnTell	= fullfat_tell,
	.pfnEOF		= fullfat_eof,
};

static const BT_IF_FS oFilesystemInterface = {
	.name 			= "vfat",
	.pfnFormat 		= fullfat_format,
	.pfnMount 		= fullfat_mount,
	.pfnUnmount 	= fullfat_unmount,
	.pfnOpen		= fullfat_open,
	.pfnMkDir		= fullfat_mkdir,
	.pfnRmDir		= fullfat_rmdir,
	.pfnOpenDir 	= fullfat_opendir,
	.pfnGetInode 	= fullfat_open_inode,
	.pfnUnlink 		= fullfat_unlink,
	.pfnRename 		= fullfat_rename,
	.pfnInfo		= fullfat_info,
	.pfnUTime		= fullfat_utime,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pFilesystemIF = &oFilesystemInterface,
	},
	.eType = BT_HANDLE_T_FILESYSTEM,
};

static const BT_IF_HANDLE oFileHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = fullfat_file_cleanup,
	.pFileIF = &oFileOperations,
	.eType = BT_HANDLE_T_FILE,
};

static const BT_IF_HANDLE oDirHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDirIF = &oDirOperations,
	},
	.eType = BT_HANDLE_T_DIRECTORY,
};

static const BT_IF_HANDLE oInodeHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pInodeIF = &oInodeOperations,
	},
	.eType = BT_HANDLE_T_INODE,
};

static BT_ERROR fullfat_init() {
	BT_ERROR Error;
	BT_HANDLE hFullFAT = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hFullFAT) {
		return Error;
	}

	return BT_RegisterFilesystem(hFullFAT);
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	fullfat_init,
};
