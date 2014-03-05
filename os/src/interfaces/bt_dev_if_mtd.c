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
#include <of/bt_of.h>
#include <stdio.h>
#include <string.h>

BT_DEF_MODULE_NAME			("MTD device manager")
BT_DEF_MODULE_DESCRIPTION	("Manages MTD devices")
BT_DEF_MODULE_AUTHOR		("Micheal Daniel")
BT_DEF_MODULE_EMAIL			("mdaniel@riegl.com")


static BT_LIST_HEAD(g_mtd_devices);
static BT_u32 num_mtd_devices = 0;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
};

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {
	BT_MTD_INFO *pInfo = (BT_MTD_INFO *) bt_container_of(node, BT_MTD_INFO, node);
	if(!pInfo->ulReferenceCount) {
		pInfo->offset = 0;
		pInfo->ulReferenceCount += 1;
		BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &pInfo->h);
		return (BT_HANDLE) &pInfo->h;
	}

	return NULL;
}

static const BT_DEVFS_OPS mtd_devfs_ops = {
	.pfnOpen = devfs_open,
};

/********************************************************************************************
 *
 * MTD Block Device functions
 *
 ********************************************************************************************/

static BT_u32 mtdblock_blockread(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer, BT_ERROR *pError) {
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) bt_container_of((BT_HANDLE_HEADER *) hBlock, BT_MTD_INFO, hBlockdev);
	BT_ERROR ret;
	BT_u32 retlen = 0;

	if((ret = BT_MTD_Read((BT_HANDLE)mtd,
							mtd->oBlock.oGeometry.ulBlockSize * ulBlock,
							mtd->oBlock.oGeometry.ulBlockSize * ulCount,
							&retlen, pBuffer )) != BT_ERR_NONE) {
		if(pError)
			*pError = ret;
		return 0;
	}

	if(pError)
		*pError = BT_ERR_NONE;

	return ulCount;
}

static BT_u32 mtdblock_blockwrite(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *buf, BT_ERROR *pError) {
	BT_u8 *pBuffer = (BT_u8 *) buf;
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) bt_container_of((BT_HANDLE_HEADER *) hBlock, BT_MTD_INFO, hBlockdev);
	BT_u32 retlen_total = 0;
	BT_u32 retlen = 0;
	BT_ERROR ret;
	BT_u32 pos = ulBlock * mtd->oBlock.oGeometry.ulBlockSize;
	BT_u32 len = ulCount * mtd->oBlock.oGeometry.ulBlockSize;
	BT_u8 * write_cache = BT_kMalloc(mtd->erasesize);

	while(len > 0) {
		BT_u32 sect_start = (pos/mtd->erasesize)*mtd->erasesize;
		BT_u32 offset = pos - sect_start;
		BT_u32 size = mtd->erasesize - offset;

		if(size > len)
			size = len;

		if(size == mtd->erasesize) {
			// do erase and write of whole sector
			BT_MTD_ERASE_INFO erase;
			erase.addr = sect_start;
			erase.len = mtd->erasesize;

			if((ret = BT_MTD_Erase((BT_HANDLE)mtd, &erase)) != BT_ERR_NONE) {
				goto mtd_blockwrite_fail;
			}

			if((ret = BT_MTD_Write((BT_HANDLE)mtd, sect_start, size, &retlen, pBuffer)) != BT_ERR_NONE){
				goto mtd_blockwrite_fail;
			}
			retlen_total += retlen;
		}
		else
		{
			// read
			if((ret = BT_MTD_Read((BT_HANDLE)mtd, sect_start, mtd->erasesize, &retlen, write_cache)) != BT_ERR_NONE) {
				goto mtd_blockwrite_fail;
			}

			// modify buffer
			memcpy(write_cache + offset, pBuffer, size);

			// erase flash sector
			BT_MTD_ERASE_INFO erase;
			erase.addr = sect_start;
			erase.len = mtd->erasesize;

			if((ret = BT_MTD_Erase((BT_HANDLE)mtd, &erase)) != BT_ERR_NONE) {
				goto mtd_blockwrite_fail;
			}

			// write modified buffer to flash
			if((ret = BT_MTD_Write((BT_HANDLE)mtd, sect_start, mtd->erasesize, &retlen, write_cache)) != BT_ERR_NONE) {
				goto mtd_blockwrite_fail;
			}
			retlen_total += size;
		}

		pBuffer += size;
		pos += size;
		len -= size;
	}

	if(pError)
		*pError = BT_ERR_NONE;
	BT_kFree(write_cache);
	return ulCount;

mtd_blockwrite_fail:
	if(pError)
		*pError = ret;
	BT_kFree(write_cache);
	return retlen_total / mtd->oBlock.oGeometry.ulBlockSize;
}

static const BT_IF_BLOCK mtdblock_blockdev_interface = {
		mtdblock_blockread,
		mtdblock_blockwrite,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.pBlockIF = &mtdblock_blockdev_interface,
};

static const BT_IF_HANDLE oBlockHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_DEVICE,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};


/********************************************************************************************
 *
 * MTD Char Device functions
 *
 ********************************************************************************************/


BT_ERROR BT_MTD_RegisterDevice(BT_HANDLE hDevice, const BT_i8 *szpName, BT_MTD_INFO *mtd) {

	BT_ERROR Error;
	char szpCharname[64];
	char szpBlockname[64];
	int i = 0;

	// mtd char device registration
	bt_list_add(&mtd->item, &g_mtd_devices);
	mtd->node.pOps = &mtd_devfs_ops;
	mtd->hMtd = hDevice;
	mtd->isPartition = BT_FALSE;

	sprintf(szpCharname, "%s%lu", (char*)szpName, num_mtd_devices);
	Error = BT_DeviceRegister(&mtd->node, szpCharname);

	// mtd block device registration
	BT_AttachHandle(NULL, &oBlockHandleInterface, (BT_HANDLE)&mtd->hBlockdev);
	mtd->oBlock.oGeometry.ulBlockSize 	= 512;
	mtd->oBlock.oGeometry.ulTotalBlocks	= mtd->size / 512;
	BT_LIST_INIT_HEAD(&mtd->partitions);

	sprintf(szpBlockname, "%sblock%lu", (char*)szpName, num_mtd_devices);
	BT_RegisterBlockDevice((BT_HANDLE)&mtd->hBlockdev, szpBlockname, &mtd->oBlock);

	// parse information from device tree
#ifdef BT_CONFIG_OF
	struct bt_device_node *node = bt_of_integrated_get_node(mtd->pDevice);
	struct bt_list_head *pos;
	i=0;
	bt_list_for_each(pos, &node->children) {
		struct bt_device_node *part_node = (struct bt_device_node *) pos;
		const char *label = bt_of_get_property(part_node, "label", NULL);
		BT_u64 size = 0;
		const BT_be32 *val = bt_of_get_address(part_node, 0, &size, 0);
		BT_u32 start_address = bt_be32_to_cpu(*val);

		BT_MTD_PART * partition = BT_kMalloc(sizeof(BT_MTD_PART));
		partition->master = mtd;
		partition->offset = start_address;

		partition->mtd.node.pOps = &mtd_devfs_ops;	// same ops structure as parent device
		partition->mtd.hMtd = hDevice;
		partition->mtd.ulReferenceCount = 0; 		// asume device is not open!

		partition->mtd.size = size;
		partition->mtd.erasesize = partition->master->erasesize;
		partition->mtd.flags = partition->master->flags;
		partition->mtd.isPartition = BT_TRUE;
		partition->mtd.type = partition->master->type;
		partition->mtd.numeraseregions = partition->master->numeraseregions;

		bt_list_add(&partition->mtd.item, &partition->master->partitions);
		//sprintf(szpCharname, "%s%lu%d", (char*)szpName, num_mtd_devices, i);
		sprintf(szpCharname, "%s", label);
		BT_DeviceRegister(&partition->mtd.node, szpCharname);

		// do also register partition as block device
		BT_AttachHandle(NULL, &oBlockHandleInterface, (BT_HANDLE)&partition->mtd.hBlockdev);
		partition->mtd.oBlock.oGeometry.ulBlockSize = 512;
		partition->mtd.oBlock.oGeometry.ulTotalBlocks = partition->mtd.size / partition->mtd.oBlock.oGeometry.ulBlockSize;

		//sprintf(szpBlockname, "%sblock%lu%d", (char*)szpName, num_mtd_devices, i);
		sprintf(szpBlockname, "%s-block", label);
		BT_RegisterBlockDevice((BT_HANDLE)&partition->mtd.hBlockdev, szpBlockname, &partition->mtd.oBlock);
		i++;
	}
#endif
	num_mtd_devices++;
	return Error;
}

BT_ERROR BT_MTD_GetUserInfo(BT_HANDLE hMTD, BT_MTD_USER_INFO * info) {
	BT_MTD_INFO *mtd = (BT_MTD_INFO *) hMTD;

	if(info == NULL || mtd == NULL) {
		return BT_ERR_GENERIC;
	}

	if(mtd->size >= 0 && mtd->erasesize >= 0) {
		info->size = mtd->size;
		info->erasesize = mtd->erasesize;
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
		//mtd_erase_callback(mtd->hMtd, instr);	// FIXME: changed hMtd -> mtd->hMtd
		return BT_ERR_NONE;
	}

	if(mtd->isPartition) {
		BT_MTD_PART * pPart = (BT_MTD_PART *) mtd;
		BT_MTD_ERASE_INFO intern;
		intern.addr = instr->addr + pPart->offset;
		intern.len = instr->len;
		intern.state = instr->state;
		return BT_IF_MTD_OPS(pPart->master->hMtd)->pfnErase(pPart->master->hMtd, &intern);
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

	if(mtd->isPartition) {
		BT_MTD_PART * pPart = (BT_MTD_PART *) mtd;
		return BT_IF_MTD_OPS(pPart->master->hMtd)->pfnRead(pPart->master->hMtd, from + pPart->offset, len, retlen, buf );
	}


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

	if(mtd->isPartition) {
		BT_MTD_PART * pPart = (BT_MTD_PART *) mtd;
		return BT_IF_MTD_OPS(pPart->master->hMtd)->pfnWrite(pPart->master->hMtd, to + pPart->offset, len, retlen, buf );
	}

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
