/**
 *	FullFAT Filesystem - Filesystem plugin for BitThunder.
 *
 **/

#include <bitthunder.h>
#include <fs/bt_fs.h>
#include "fullfat/fullfat.h"

BT_DEF_MODULE_NAME			("FullFAT Filesystem")
BT_DEF_MODULE_DESCRIPTION	("BitThunder FS plugin for FullFAT")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

typedef struct _BT_FF_MOUNT {
	BT_HANDLE_HEADER h;
	FF_IOMAN *pIoman;
	BT_HANDLE hVolume;
} BT_FF_MOUNT;

typedef struct _BT_FF_FILE {
	BT_HANDLE_HEADER h;
	BT_FF_MOUNT		*pMount;
	FF_FILE			*pFile;
} BT_FF_FILE;

static BT_ERROR fullfat_cleanup(BT_HANDLE h) {
	return BT_ERR_NONE;
}

static FF_T_SINT32 fullfat_readblocks(FF_T_UINT8 *pBuffer, FF_T_UINT32 Address, FF_T_UINT32 Count, void *pParam) {
	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) pParam;

	return 0;
}

static FF_T_SINT32 fullfat_writeblocks(FF_T_UINT8 *pBuffer, FF_T_UINT32 Address, FF_T_UINT32 Count, void *pParam) {
	BT_FF_MOUNT *pMount = (BT_FF_MOUNT *) pParam;


	return 0;
}

static BT_HANDLE fullfat_mount(BT_HANDLE hFS, BT_HANDLE hVolume, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_GENERIC;


	*pError = Error;
	return NULL;
}

static BT_ERROR fullfat_unmount(BT_HANDLE hMount) {

	return BT_ERR_NONE;
}

static BT_u32 fullfat_read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {

	BT_FF_FILE *pFile = (BT_FF_FILE *) hFile;

	FF_Read(pFile->pFile, 1, ulSize, (FF_T_UINT8 *) pBuffer);
	return 0;
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

static const BT_IF_FILE oFileOperations = {
	.pfnRead 	= fullfat_read,
	.pfnWrite 	= fullfat_write,
	.pfnGetC	= fullfat_getc,
	.pfnPutC	= fullfat_putc,
	.pfnSeek	= fullfat_seek,
	.pfnTell	= fullfat_tell,
};

static const BT_IF_FS oFilesystemInterface = {
	.pfnMount 	= fullfat_mount,
	.pfnUnmount = fullfat_unmount,
	.pFileOps = &oFileOperations,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = fullfat_cleanup,
	.oIfs = {
		.pFilesystemIF = &oFilesystemInterface,
	},
	.eType = BT_HANDLE_T_FILESYSTEM,
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
