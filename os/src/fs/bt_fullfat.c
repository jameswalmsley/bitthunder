/**
 *	FullFAT Filesystem - Filesystem plugin for BitThunder.
 *
 **/

#include <bitthunder.h>
#include <fs/bt_fs.h>
#include <volumes/bt_volume.h>
#include "fullfat/fullfat.h"

BT_DEF_MODULE_NAME			("FullFAT Filesystem")
BT_DEF_MODULE_DESCRIPTION	("BitThunder FS plugin for FullFAT")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

typedef struct _BT_FF_MOUNT {
	BT_HANDLE_HEADER 	h;
	FF_IOMAN 		   *pIoman;
	BT_HANDLE 			hVolume;
	void 			   *pBlockCache;
} BT_FF_MOUNT;

typedef struct _BT_FF_FILE {
	BT_HANDLE_HEADER h;
	BT_FF_MOUNT		*pMount;
	FF_FILE			*pFile;
} BT_FF_FILE;

typedef struct _BT_FF_DIR {
	BT_HANDLE_HEADER 	h;
	BT_FF_MOUNT		   *pMount;
	FF_DIRENT		    oDirent;
	BT_u32				ulCurrentEntry;
} BT_FF_DIR;

typedef struct _BT_FF_INODE {
	BT_HANDLE_HEADER	h;
	BT_FF_MOUNT		   *pMount;
	FF_DIRENT			oDirent;
} BT_FF_INODE;

static const BT_IF_HANDLE oHandleInterface;
static const BT_IF_HANDLE oFileHandleInterface;
static const BT_IF_HANDLE oDirHandleInterface;
static const BT_IF_HANDLE oInodeHandleInterface;

static BT_ERROR fullfat_cleanup(BT_HANDLE h) {
	return BT_ERR_NONE;
}

static FF_T_SINT32 fullfat_readblocks(FF_T_UINT8 *pBuffer, FF_T_UINT32 Address, FF_T_UINT32 Count, void *pParam) {
	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) pParam;
	BT_ERROR Error;
	BT_u32 ulRead = BT_VolumeRead(pMount->hVolume, Address, Count, pBuffer, &Error);

	return (FF_T_SINT32) ulRead;
}

static FF_T_SINT32 fullfat_writeblocks(FF_T_UINT8 *pBuffer, FF_T_UINT32 Address, FF_T_UINT32 Count, void *pParam) {
	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) pParam;
	BT_ERROR Error;
	BT_u32 ulWritten = BT_VolumeWrite(pMount->hVolume, Address, Count, pBuffer, &Error);

	return (FF_T_SINT32) ulWritten;
}

static BT_HANDLE fullfat_mount(BT_HANDLE hFS, BT_HANDLE hVolume, BT_ERROR *pError) {

	FF_ERROR ffError;
	BT_ERROR Error = BT_ERR_GENERIC;
	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) BT_CreateHandle(&oHandleInterface, sizeof(BT_FF_MOUNT), pError);
	if(!pMount) {
		return NULL;
	}

	pMount->pBlockCache = BT_kMalloc(8192);
	pMount->hVolume = hVolume;
	pMount->pIoman = FF_CreateIOMAN(pMount->pBlockCache, 8192, 512, &ffError);
	if(!pMount->pIoman) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	ffError = FF_RegisterBlkDevice(pMount->pIoman, 512, fullfat_writeblocks, fullfat_readblocks, pMount);

	ffError = FF_MountPartition(pMount->pIoman, 0);

return (BT_HANDLE) pMount;

err_free_out:
	BT_kFree(pMount->pBlockCache);
	BT_kFree(pMount);

	*pError = Error;
	return NULL;
}

static BT_ERROR fullfat_unmount(BT_HANDLE hMount) {
	return BT_ERR_NONE;
}

static BT_HANDLE fullfat_open(BT_HANDLE hMount, BT_i8 *szpPath, BT_u32 ulModeFlags, BT_ERROR *pError) {

	FF_ERROR ffError;

	BT_FF_FILE *pFile = (BT_FF_FILE *) BT_CreateHandle(&oFileHandleInterface, sizeof(BT_FF_FILE), pError);
	if(!pFile) {
		return NULL;
	}

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;

	pFile->pFile = FF_Open(pMount->pIoman, szpPath, FF_MODE_READ, &ffError);
	if(!pFile->pFile) {
		return NULL;
	}

	pFile->pMount = pMount;

	return (BT_HANDLE) pFile;
}

static BT_ERROR fullfat_file_cleanup(BT_HANDLE hFile) {
	return BT_ERR_NONE;
}

static BT_u32 fullfat_read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {

	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;

	FF_T_SINT32 sRead = FF_Read(pFile->pFile, 1, ulSize, (FF_T_UINT8 *) pBuffer);
	return (BT_u32) sRead;
}

static BT_u32 fullfat_write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	return 0;
}

static BT_s32 fullfat_getc(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError) {
	return -1;
}

static BT_ERROR fullfat_putc(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData) {
	return BT_ERR_NONE;
}

static BT_ERROR fullfat_seek(BT_HANDLE hFile, BT_u64 ulOffset) {
	return BT_ERR_NONE;
}

static BT_u64 fullfat_tell(BT_HANDLE hFile) {
	return 0;
}

static BT_ERROR fullfat_mkdir(BT_HANDLE hMount, BT_i8 *szpPath) {
	return BT_ERR_GENERIC;
}

static BT_HANDLE fullfat_opendir(BT_HANDLE hMount, BT_i8 *szpPath, BT_ERROR *pError) {

	BT_FF_DIR *pDir = (BT_FF_DIR *) BT_CreateHandle(&oDirHandleInterface, sizeof(BT_FF_DIR), pError);
	if(!pDir) {
		*pError = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) hMount;
	FF_ERROR ffError = FF_FindFirst(pMount->pIoman, &pDir->oDirent, szpPath);
	if(ffError) {
		*pError = BT_ERR_GENERIC;
		goto err_free_out;
	}

	pDir->pMount = pMount;

	return (BT_HANDLE) pDir;

err_free_out:
	BT_kFree(pDir);

err_out:
	return NULL;
}

static BT_ERROR fullfat_read_dir(BT_HANDLE hDir, BT_DIRENT *pDirent) {

	BT_FF_DIR *pDir = (BT_FF_DIR *) hDir;

	if(pDir->ulCurrentEntry) {
		FF_ERROR ffError = FF_FindNext(pDir->pMount->pIoman, &pDir->oDirent);
	}

	pDirent->szpName = pDir->oDirent.FileName;
	pDirent->ullFileSize = pDir->oDirent.Filesize;

	pDir->ulCurrentEntry += 1;

	return BT_ERR_NONE;
}

static BT_ERROR fullfat_dir_cleanup(BT_HANDLE hDir) {
	return BT_ERR_NONE;
}

static BT_HANDLE fullfat_open_inode(BT_HANDLE hMount, BT_i8 *szpPath, BT_ERROR *pError) {
	return NULL;
}

static BT_ERROR fullfat_read_inode(BT_HANDLE hInode, BT_INODE *pInode) {
	return BT_ERR_GENERIC;
}

static BT_ERROR fullfat_inode_cleanup(BT_HANDLE hInode) {
	return BT_ERR_NONE;
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
};

static const BT_IF_FS oFilesystemInterface = {
	.pfnMount 		= fullfat_mount,
	.pfnUnmount 	= fullfat_unmount,
	.pfnOpen		= fullfat_open,
	.pfnMkDir		= fullfat_mkdir,
	.pfnOpenDir 	= fullfat_opendir,
	.pfnGetInode 	= fullfat_open_inode,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = fullfat_cleanup,
	.oIfs = {
		.pFilesystemIF = &oFilesystemInterface,
	},
	.eType = BT_HANDLE_T_FILESYSTEM,
};

static const BT_IF_HANDLE oFileHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = fullfat_file_cleanup,
	.oIfs = {
		.pFileIF = &oFileOperations,
	},
	.eType = BT_HANDLE_T_FILE,
};

static const BT_IF_HANDLE oDirHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = fullfat_dir_cleanup,
	.oIfs = {
		.pDirIF = &oDirOperations,
	},
	.eType = BT_HANDLE_T_DIRECTORY,
};

static const BT_IF_HANDLE oInodeHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = fullfat_inode_cleanup,
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
