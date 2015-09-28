/**
 *	BitThunder Volume and Partition Manager
 *
 *
 **/

#include <bitthunder.h>
#include <collections/bt_list.h>
#include <lib/getmem.h>
#include "ibm_mbr.h"
#include <string.h>
#include <stdio.h>

BT_DEF_MODULE_NAME			("Volume and Partition Manager")
BT_DEF_MODULE_DESCRIPTION	("Volume manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_VOLUME_DESCRIPTOR 	v;
};

typedef struct _BT_PARTITION {
	struct _BT_OPAQUE_HANDLE oVolume;
	BT_u32	ulBaseAddress;
	BT_u32	ulPartitionNumber;
} BT_PARTITION;

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *dev_node, BT_ERROR *pError) {

	struct _BT_OPAQUE_HANDLE *h = bt_container_of(dev_node, struct _BT_OPAQUE_HANDLE, v.node);
	if(!h->v.ulReferenceCount) {
		h->v.ulReferenceCount += 1;

		BT_AttachHandle(NULL, &oHandleInterface, h);

		return h;
	}

	return NULL;
}

static const BT_DEVFS_OPS oDevfsOps = {
	.pfnOpen = devfs_open,
};

static BT_u32 BT_PartitionCount(BT_u8 *pBuffer) {
	BT_u32 count = 0;
	BT_u8 part;
	// Check PBR or MBR signature
	if (pBuffer[IBM_MBR_SIGNATURE] != 0x55 &&
		pBuffer[IBM_MBR_SIGNATURE+1] != 0xAA ) {
		// No MBR, but is it a PBR ?
		if (pBuffer[0] == 0xEB &&          // PBR Byte 0
		    pBuffer[2] == 0x90 &&          // PBR Byte 2
		    (pBuffer[21] & 0xF0) == 0xF0) {// PBR Byte 21 : Media byte
			return 1;	// No MBR but PBR exist then only one partition
		}
		return 0;   // No MBR and no PBR then no partition found
	}
	for (part = 0; part < 4; part++)  {
		BT_u8 active = pBuffer[IBM_MBR_PTBL + IBM_MBR_PTBL_ACTIVE + (16 * part)];
		BT_u8 part_id = pBuffer[IBM_MBR_PTBL + IBM_MBR_PTBL_ID + (16 * part)];
		// The first sector must be a MBR, then check the partition entry in the MBR
		if (active != 0x80 && (active != 0 || part_id == 0)) {
			break;
		}
		count++;
	}
	return count;
}

static void init_devfs_node(BT_HANDLE hVolume) {
	hVolume->v.node.pOps = &oDevfsOps;
}

BT_ERROR BT_EnumerateVolumes(BT_HANDLE hBlock) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_s32 ret;

	BT_BLKDEV_DESCRIPTOR *blk = (BT_BLKDEV_DESCRIPTOR *) hBlock;

	struct bt_list_head *pos, *next;
	bt_list_for_each_safe(pos, next, &blk->volumes) {

		BT_VOLUME_DESCRIPTOR *v = bt_container_of(pos, BT_VOLUME_DESCRIPTOR, item);
		BT_UnregisterVolume((BT_HANDLE) v);
	}


	BT_u8 *pMBR = BT_kMalloc(blk->oGeometry.ulBlockSize);
	if(!pMBR) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	ret = BT_BlockRead((BT_HANDLE) &blk->h, 0, 1, pMBR);
	if(ret != 1) {
		if(ret < 0) {
			Error = ret;
		} else {
			Error = BT_ERR_GENERIC;
		}
		goto err_free_out;
	}

	// Create the volume handle.
	BT_u32 partCount = BT_PartitionCount(pMBR);
	if(!partCount) {
		BT_HANDLE hVolume = BT_kMalloc(sizeof(struct _BT_OPAQUE_HANDLE));
		if(!hVolume) {
			return BT_ERR_NO_MEMORY;
		}

		hVolume->v.eType 			= BT_VOLUME_NORMAL;
		hVolume->v.ulTotalBlocks 	= blk->oGeometry.ulTotalBlocks;
		hVolume->v.blkdev         	= blk;
		hVolume->v.ulReferenceCount = 0;

		bt_list_add(&hVolume->v.item, &blk->volumes);

		BT_i8 *iname = BT_kMalloc(strlen(blk->node.szpName) + 10);;
		bt_sprintf(iname, "%s%d", blk->node.szpName, 0);

		init_devfs_node(hVolume);

		BT_DeviceRegister(&hVolume->v.node, iname);
		BT_kDebug("Adding a volume: %s", iname);

		BT_kFree(iname);

	} else {
		BT_u32 i;
		for(i = 0; i < partCount; i++) {
			BT_HANDLE hVolume = BT_kMalloc(sizeof(BT_PARTITION));
			if(!hVolume) {
				return BT_ERR_NO_MEMORY;
			}

			BT_PARTITION *pPart = (BT_PARTITION *) hVolume;

			hVolume->v.eType 				= BT_VOLUME_PARTITION;
			pPart->ulBaseAddress 		= BT_Get32LE(pMBR, (IBM_MBR_PTBL + (16 * i) + IBM_MBR_PTBL_LBA));
			pPart->ulPartitionNumber 	= i;
			hVolume->v.ulTotalBlocks		= BT_Get32LE(pMBR, (IBM_MBR_PTBL + (16 * i) + IBM_MBR_PTBL_SECTORS));
			hVolume->v.blkdev     		= blk;
			hVolume->v.ulReferenceCount 	= 0;

			bt_list_add(&hVolume->v.item, &blk->volumes);

			BT_i8 *iname = BT_kMalloc(strlen(blk->node.szpName) + 10);
			bt_sprintf(iname, "%s%lu", blk->node.szpName, i);

			init_devfs_node(hVolume);

			BT_DeviceRegister(&hVolume->v.node, iname);
			BT_kDebug("Adding a partition: %s", iname);

			BT_kFree(iname);
		}
	}

	BT_kFree(pMBR);

	return BT_ERR_NONE;

err_free_out:
	BT_kFree(pMBR);

err_out:
	return Error;
}
BT_EXPORT_SYMBOL(BT_EnumerateVolumes);

BT_BOOL BT_isVolumeBusy(BT_HANDLE hVolume) {
	return (hVolume->v.ulReferenceCount > 0);
}
BT_EXPORT_SYMBOL(BT_isVolumeBusy);

BT_RegisterVolume(BT_HANDLE hVolume) {

}
BT_EXPORT_SYMBOL(BT_RegisterVolume);

BT_UnregisterVolume(BT_HANDLE hVolume) {

	BT_DeviceUnregister(&hVolume->v.node);
}
BT_EXPORT_SYMBOL(BT_UnregisterVolume);

BT_s32 BT_VolumeRead(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer) {

	if(hVolume->v.eType == BT_VOLUME_NORMAL) {
		return BT_BlockRead((BT_HANDLE) hVolume->v.blkdev, ulAddress, ulBlocks, pBuffer);
	}

	BT_PARTITION *pPart = (BT_PARTITION *)  hVolume;

	return BT_BlockRead((BT_HANDLE) hVolume->v.blkdev, ulAddress + pPart->ulBaseAddress, ulBlocks, pBuffer);
}
BT_EXPORT_SYMBOL(BT_VolumeRead);

BT_s32 BT_VolumeWrite(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer) {

	if(hVolume->v.eType == BT_VOLUME_NORMAL) {
		return BT_BlockWrite((BT_HANDLE) hVolume->v.blkdev, ulAddress, ulBlocks, pBuffer);
	}

	BT_PARTITION *pPart = (BT_PARTITION *)  hVolume;

	return BT_BlockWrite((BT_HANDLE) hVolume->v.blkdev, ulAddress + pPart->ulBaseAddress, ulBlocks, pBuffer);
}
BT_EXPORT_SYMBOL(BT_VolumeWrite);

BT_ERROR BT_GetVolumeGeometry(BT_HANDLE hVolume, BT_BLOCK_GEOMETRY *pGeometry) {
	BT_GetBlockGeometry((BT_HANDLE) hVolume->v.blkdev, pGeometry);
	if(pGeometry) {
		pGeometry->ulTotalBlocks = hVolume->v.ulTotalBlocks;
	}
}
BT_EXPORT_SYMBOL(BT_GetVolumeGeometry);

static BT_ERROR bt_volume_inode_cleanup(BT_HANDLE hVolume) {
	hVolume->v.ulReferenceCount -= 1;
	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.ulFlags = BT_HANDLE_FLAGS_NO_DESTROY,
	.eType = BT_HANDLE_T_VOLUME,
	.pfnCleanup = bt_volume_inode_cleanup,
};
