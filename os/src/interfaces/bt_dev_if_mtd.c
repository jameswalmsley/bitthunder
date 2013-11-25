/*
 * bt_dev_if_mtd.c
 *
 *  Created on: 30.10.2013
 *      Author: dq
 */
#include <bitthunder.h>
#include <collections/bt_list.h>
#include <interrupts/bt_tasklets.h>
#include <devman/bt_mtd.h>
#include <string.h>

BT_DEF_MODULE_NAME			("MTD device manager")
BT_DEF_MODULE_DESCRIPTION	("Manages MTD devices")
BT_DEF_MODULE_AUTHOR		("Micheal Daniel")
BT_DEF_MODULE_EMAIL			("mdaniel@riegl.com")


static BT_LIST_HEAD(g_mtd_devices);

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
};

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {
	BT_MTD_INFO *pInfo = (BT_MTD_INFO *) bt_container_of(node, BT_MTD_INFO, node);
	if(!pInfo->ulReferenceCount) {
		pInfo->ulReferenceCount += 1;
		BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &pInfo->h);
		return (BT_HANDLE) &pInfo->h;
	}

	return NULL;
}

static const BT_DEVFS_OPS mtd_devfs_ops = {
	.pfnOpen = devfs_open,
};

BT_ERROR BT_MTD_RegisterDevice(BT_HANDLE hDevice, const BT_i8 *szpName, BT_MTD_INFO *mtd) {

	BT_ERROR Error;

	bt_list_add(&mtd->item, &g_mtd_devices);
	mtd->node.pOps = &mtd_devfs_ops;
	mtd->hMtd = hDevice;

	Error = BT_DeviceRegister(&mtd->node, szpName);

	return Error;
}

BT_ERROR BT_MTD_GetUserInfo(BT_HANDLE hMTD, BT_MTD_USER_INFO * info) {
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) hMTD;

	if(info == NULL || mtd == NULL) {
		return BT_ERR_GENERIC;
	}

	if(mtd->size >= 0 && mtd->erasesize >= 0) {
		info->size = mtd->size;
		info->erasesize = mtd->size;
		info->flags = mtd->flags;
		info->type = mtd->type;
	}
	else {
		return BT_ERR_GENERIC;
	}
	return BT_ERR_NONE;
}

void mtd_erase_callback(BT_HANDLE hFlash, BT_MTD_ERASE_INFO *instr)
{
	// TODO: check if part erase is present!!
	if(instr->callback)
		instr->callback(hFlash, instr);
}

/*
 * Erase is an asynchronous operation.  Device drivers are supposed
 * to call instr->callback() whenever the operation completes, even
 * if it completes with a failure.
 * Callers are supposed to pass a callback function and wait for it
 * to be called before writing to the block.
 */
BT_ERROR BT_MTD_Erase(BT_HANDLE hMTD, BT_MTD_ERASE_INFO *instr)
{
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) hMTD;

	if(instr->addr > mtd->size || instr->len > mtd->size - instr->addr)
		return BT_ERR_INVALID_VALUE;
	if(!(mtd->flags & BT_MTD_WRITEABLE))
		return BT_ERR_GENERIC;
	instr->fail_addr = BT_MTD_FAIL_ADDR_UNKNOWN;
	if(!instr->len) {
		instr->state = BT_MTD_ERASE_DONE;
		mtd_erase_callback(mtd->hMtd, instr);	// FIXME: changed hMtd -> mtd->hMtd
		return BT_ERR_NONE;
	}
	return BT_IF_MTD_OPS(mtd->hMtd)->pfnErase(mtd->hMtd, instr);
}


BT_ERROR BT_MTD_Read(BT_HANDLE hMTD, BT_u64 from, BT_u32 len, BT_u32 *retlen, BT_u8 *buf)
{
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) hMTD;

	BT_ERROR retcode;
	*retlen = 0;
	if(from < 0 || from > mtd->size || len > mtd->size - from)
		return BT_ERR_INVALID_VALUE;
	if(!len)
		return BT_ERR_NONE;

	/*
	 * In the absence of an error, drivers return a non-negative integer
	 * representing the maximum number of bitflips that were corrected on
	 * any one ecc region (if applicable; zero otherwise).
	 */
	retcode = BT_IF_MTD_OPS(mtd->hMtd)->pfnRead(mtd->hMtd, from, len, retlen, buf);
	if(retcode != BT_ERR_NONE)
		return retcode;

	//if(mtd->ecc_strength == 0)
	//	return BT_ERR_GENERIC;

	return retcode;
}


BT_ERROR BT_MTD_Write(BT_HANDLE hMTD, BT_u64 to, BT_u32 len, BT_u32 *retlen, const BT_u8 *buf)
{
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) hMTD;

	*retlen = 0;
	if(to < 0 || to > mtd->size || len > mtd->size - to)
		return BT_ERR_INVALID_VALUE;
	if(!(mtd->flags & BT_MTD_WRITEABLE))
		return BT_ERR_GENERIC;
	if(!len)
		return BT_ERR_NONE;
	return BT_IF_MTD_OPS(mtd->hMtd)->pfnWrite(mtd->hMtd, to, len, retlen, buf);
}

static BT_ERROR bt_mtd_cleanup(BT_HANDLE hMTD) {
	BT_MTD_INFO *pInfo = (BT_MTD_INFO *) hMTD;

	if(pInfo->ulReferenceCount) {
		pInfo->ulReferenceCount -= 1;
	}

	pInfo->offset = 0;

	return BT_ERR_NONE;
}

static BT_u32 mtd_file_write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer, BT_ERROR *pError) {
	BT_MTD_INFO *pInfo = (BT_MTD_INFO *) hFile;
	BT_u32 len = 0;
	BT_ERROR Error = BT_MTD_Write(hFile, pInfo->offset, ulSize, &len, pBuffer);
	pInfo->offset += len;
	*pError = Error;
	return len;
}

static BT_u32 mtd_file_read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {
	BT_MTD_INFO *pInfo = (BT_MTD_INFO *) hFile;
	BT_u32 len = 0;
	BT_ERROR Error = BT_MTD_Read(hFile, pInfo->offset, ulSize, &len, pBuffer);
	pInfo->offset += len;
	*pError = Error;
	return len;
}

static BT_u64 mtd_file_tell(BT_HANDLE hFile) {
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) hFile;
	return mtd->offset;
}

static BT_ERROR mtd_file_seek(BT_HANDLE hFile, BT_s64 ulOffset, BT_u32 whence) {
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) hFile;
	switch(whence) {
		case BT_SEEK_SET:
			if(ulOffset < 0)
				return BT_ERR_INVALID_VALUE;
			else if(ulOffset > mtd->size)
				return BT_ERR_INVALID_VALUE;
			else
				mtd->offset = ulOffset;
			break;
		case BT_SEEK_CUR:
			if(mtd->offset + ulOffset < 0)
				return BT_ERR_INVALID_VALUE;
			else if (mtd->offset + ulOffset > mtd->size)
				return BT_ERR_INVALID_VALUE;
			else
				mtd->offset += ulOffset;
			break;
		case BT_SEEK_END:
			if(ulOffset > 0)
				return BT_ERR_INVALID_VALUE;
			else if(mtd->offset + ulOffset < 0)
				return BT_ERR_INVALID_VALUE;
			else
				mtd->offset += ulOffset;
			break;
		default:
			return BT_ERR_UNSUPPORTED_FLAG;
	}
	return BT_ERR_NONE;
}

static BT_IF_FILE mtd_file_ops = {
	.pfnWrite	= mtd_file_write,
	.pfnRead	= mtd_file_read,
	.pfnSeek 	= mtd_file_seek,
	.pfnTell 	= mtd_file_tell,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.ulFlags = BT_HANDLE_FLAGS_NO_DESTROY,
	.eType = BT_HANDLE_T_MTD,
	.pFileIF = &mtd_file_ops,
	.pfnCleanup = bt_mtd_cleanup,
};

