/**
 *	Ext2 Filesystem - Filesystem plugin for BitThunder.
 *
 **/

#include <bitthunder.h>
#include <fs/bt_fs.h>
#include <volumes/bt_volume.h>
#include "ext2/ext2fs.h"

BT_DEF_MODULE_NAME			("Ext2 Filesystem (Read Only)")
BT_DEF_MODULE_DESCRIPTION		("BitThunder FS plugin for Ext2")
BT_DEF_MODULE_AUTHOR			("Andreas Friedl")
BT_DEF_MODULE_EMAIL			("afriedl@riegl.com")

#define DEBUG 0

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

typedef struct _BT_EXT2_MOUNT {
	BT_HANDLE_HEADER 	h;
	BT_HANDLE 		hVolume;
} BT_EXT2_MOUNT;

typedef struct _BT_EXT2_FILE {
	BT_HANDLE_HEADER h;
	BT_EXT2_MOUNT		*pMount;
} BT_EXT2_FILE;

typedef struct _BT_EXT2_INODE {
	BT_HANDLE_HEADER h;
	BT_EXT2_MOUNT		*pMount;
	const BT_i8 		*szpPath;
} BT_EXT2_INODE;

typedef struct _BT_EXT2_DIR {
	BT_HANDLE_HEADER 	h;
	BT_EXT2_MOUNT		*pMount;
	#define BT_EXT2_FNAME_MAX_STRLEN	256
	char 			*szpFname;
} BT_EXT2_DIR;

static const BT_IF_HANDLE oHandleInterface;
static const BT_IF_HANDLE oFileHandleInterface;
static const BT_IF_HANDLE oDirHandleInterface;
static const BT_IF_HANDLE oInodeHandleInterface;

static BT_ERROR ext2_cleanup(BT_HANDLE h) {
	return BT_ERR_NONE;
}

static int ext2_readblocks(unsigned char *pBuffer, unsigned int SectorAddress, unsigned int Count, void *pParam) {
	BT_HANDLE hVolume = (BT_HANDLE)pParam;
	BT_ERROR Error;
	BT_u32 ulRead = BT_VolumeRead(hVolume, SectorAddress, Count, pBuffer, &Error);
#if DEBUG
	int i, j;
	for(i=0;i<Count;i++) {
		bt_printf("%02d+%02d: ", SectorAddress, i);
		for(j=0;j<SECTOR_SIZE;j++) {
			if((j%16) == 0) bt_printf("\n  ");
			bt_printf("%02x ", pBuffer[j]);
		}
		bt_printf("\n");
	}
#endif
	return (int)ulRead;
}

static BT_HANDLE ext2_mount(BT_HANDLE hFS, BT_HANDLE hVolume, BT_ERROR *pError) {
	if(!hFS || !hVolume) {
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_out;
	}
	BT_EXT2_MOUNT *pMount = (BT_EXT2_MOUNT *) BT_CreateHandle(&oHandleInterface, sizeof(BT_EXT2_MOUNT), pError);
	if(!pMount) {
		*pError = BT_ERR_NO_MEMORY;
		goto err_out;
	}
	pMount->hVolume = hVolume;
	ext2fs_set_readblocks(ext2_readblocks, hVolume);	
	if(ext2fs_mount(0) < 0) {
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_free_out;
	}
	return (BT_HANDLE) pMount;
err_free_out:
	BT_DestroyHandle((BT_HANDLE)pMount);
err_out:
	return NULL;
}

static BT_ERROR ext2_unmount(BT_HANDLE hMount) {
	return BT_ERR_NONE;
}

static BT_HANDLE ext2_open(BT_HANDLE hMount, const BT_i8 *szpPath, BT_u32 ulModeFlags, BT_ERROR *pError) {
	if(!hMount) {
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_out;
	}
	BT_EXT2_FILE *pFile = (BT_EXT2_FILE *) BT_CreateHandle(&oFileHandleInterface, sizeof(BT_EXT2_FILE), pError);
	if(!pFile) {
		*pError = BT_ERR_NO_MEMORY;
		goto err_out;
	}
	pFile->pMount = (BT_EXT2_MOUNT *)hMount;
	if(ext2fs_open((char *)szpPath) < 0) 
	{
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_free_out;
	}
	return (BT_HANDLE) pFile;
err_free_out:
	BT_DestroyHandle((BT_HANDLE)pFile);
err_out:
	return NULL;
}

static BT_ERROR ext2_file_cleanup(BT_HANDLE hFile) {
	ext2fs_close();
	return BT_ERR_NONE;
}

static BT_u32 ext2_read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	if(!hFile) {
		if(pError) *pError = BT_ERR_GENERIC;
		return 0;
	}
	if(pError) *pError = BT_ERR_NONE;
	BT_u32 rc = 0;
	rc = ext2fs_read (pBuffer, ulSize);

	return rc;
}

static BT_u32 ext2_write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	if(pError) *pError = BT_ERR_GENERIC;
	return 0;
}

static BT_s32 ext2_getc(BT_HANDLE hFile, BT_u32 ulFlags, BT_ERROR *pError) {
	if(!hFile) {
		if(pError) *pError = BT_ERR_GENERIC;
		return 0;
	}
	if(pError) *pError = BT_ERR_NONE;
	char c = 0;
	BT_s32 rc = ext2fs_read (&c, 1);
	if(rc == 0) rc = -1;
	else rc = c;
	
	return rc;
}

static BT_ERROR ext2_putc(BT_HANDLE hFile, BT_u32 ulFlags, BT_i8 cData) {
	return BT_ERR_GENERIC;
}

static BT_ERROR ext2_seek(BT_HANDLE hFile, BT_s64 ulOffset, BT_u32 whence) {
	return BT_ERR_NONE;
}

static BT_u64 ext2_tell(BT_HANDLE hFile) {
	return 0;
}

static BT_ERROR ext2_mkdir(BT_HANDLE hMount, const BT_i8 *szpPath) {
	return BT_ERR_GENERIC;
}

static BT_HANDLE ext2_opendir(BT_HANDLE hMount, const BT_i8 *szpPath, BT_ERROR *pError) {
	if(!hMount) {
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_out;
	}
	BT_EXT2_DIR *pDir = (BT_EXT2_DIR *) BT_CreateHandle(&oDirHandleInterface, sizeof(BT_EXT2_DIR), pError);
	if(!pDir) {
		*pError = BT_ERR_NO_MEMORY;
		goto err_out;
	}
	pDir->pMount = (BT_EXT2_MOUNT *)hMount;
	pDir->szpFname = BT_kMalloc(BT_EXT2_FNAME_MAX_STRLEN);
	if(!pDir->szpFname) {
		*pError = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}	
	if(ext2fs_open_dir((char *)szpPath) < 0) 
	{
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_free_out;
	}
#if 0
	if(ext2fs_ls((char *)szpPath) < 0) 
	{
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_free_out;
	}
#endif
	return (BT_HANDLE) pDir;
err_free_out:
	if(pDir->szpFname) BT_kFree(pDir->szpFname);
	BT_DestroyHandle((BT_HANDLE)pDir);
err_out:
	return NULL;
}

static BT_ERROR ext2_read_dir(BT_HANDLE hDir, BT_DIRENT *pDirent) {
	if(!hDir || !pDirent) {
		return BT_ERR_GENERIC;
	}
	BT_EXT2_DIR *pDir = (BT_EXT2_DIR *)hDir;
	unsigned long ulFsize;
	int nlFtype;
	if(ext2fs_read_dir (pDir->szpFname, BT_EXT2_FNAME_MAX_STRLEN, &ulFsize, &nlFtype) < 0) {
		return BT_ERR_GENERIC;
	}
	pDirent->szpName = pDir->szpFname;
	pDirent->ullFileSize = (unsigned long long)ulFsize;
	pDirent->attr = 0;
	if(nlFtype & EXT2_FILETYPE_DIRECTORY) {
		pDirent->attr |= BT_ATTR_DIR;
	}
	
	return BT_ERR_NONE;
}

static BT_ERROR ext2_dir_cleanup(BT_HANDLE hDir) {
	ext2fs_close_dir();
	if(hDir) {
		BT_EXT2_DIR *pDir = (BT_EXT2_DIR *)hDir;
		if(pDir->szpFname) BT_kFree(pDir->szpFname);
	}
	return BT_ERR_NONE;
}

static BT_HANDLE ext2_open_inode(BT_HANDLE hMount, const BT_i8 *szpPath, BT_ERROR *pError) {
	if(!hMount) {
		if(pError) *pError = BT_ERR_GENERIC;
		goto err_out;
	}
	BT_EXT2_INODE *pInode = (BT_EXT2_INODE *) BT_CreateHandle(&oInodeHandleInterface, sizeof(BT_EXT2_INODE), pError);
	if(!pInode) {
		*pError = BT_ERR_NO_MEMORY;
		goto err_out;
	}
	pInode->pMount = (BT_EXT2_MOUNT *)hMount;
	pInode->szpPath = szpPath;

	return (BT_HANDLE) pInode;
err_out:
	return NULL;
}

static BT_ERROR ext2_read_inode(BT_HANDLE hInode, BT_INODE *pInode) {
	struct ext2_inode inode;
	BT_EXT2_INODE *phInode = (BT_EXT2_INODE *)hInode;

	if(!hInode || !pInode) {
		return BT_ERR_GENERIC;
	}
	if(ext2fs_get_inode((char *)phInode->szpPath, &inode) < 0) {
		return BT_ERR_GENERIC;
	}
	pInode->ullFilesize = inode.size;

	return BT_ERR_NONE;
}

static BT_ERROR ext2_inode_cleanup(BT_HANDLE hInode) {
	return BT_ERR_NONE;
}

static const BT_IF_DIR oDirOperations = {
	.pfnReadDir 	= ext2_read_dir,
};

static const BT_IF_INODE oInodeOperations = {
	.pfnReadInode 	= ext2_read_inode,
};

static const BT_IF_FILE oFileOperations = {
	.pfnRead 	= ext2_read,
	.pfnWrite 	= ext2_write,
	.pfnGetC	= ext2_getc,
	.pfnPutC	= ext2_putc,
	.pfnSeek	= ext2_seek,
	.pfnTell	= ext2_tell,
};

static const BT_IF_FS oFilesystemInterface = {
	.pfnMount 	= ext2_mount,
	.pfnUnmount 	= ext2_unmount,
	.pfnOpen	= ext2_open,
	.pfnMkDir	= ext2_mkdir,
	.pfnOpenDir 	= ext2_opendir,
	.pfnGetInode 	= ext2_open_inode,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup 	= ext2_cleanup,
	.oIfs = {
		.pFilesystemIF = &oFilesystemInterface,
	},
	.eType 		= BT_HANDLE_T_FILESYSTEM,
};

static const BT_IF_HANDLE oFileHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup 	= ext2_file_cleanup,
	.pFileIF 	= &oFileOperations,
	.eType 		= BT_HANDLE_T_FILE,
};

static const BT_IF_HANDLE oDirHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup 	= ext2_dir_cleanup,
	.oIfs = {
		.pDirIF = &oDirOperations,
	},
	.eType 		= BT_HANDLE_T_DIRECTORY,
};

static const BT_IF_HANDLE oInodeHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup 	= ext2_inode_cleanup,
	.oIfs = {
		.pInodeIF = &oInodeOperations,
	},
	.eType 		= BT_HANDLE_T_INODE,
};

static BT_ERROR ext2_init() {
	BT_ERROR Error;
	BT_HANDLE hFsExt2 = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hFsExt2) {
		return Error;
	}

	return BT_RegisterFilesystem(hFsExt2);
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	ext2_init,
};
