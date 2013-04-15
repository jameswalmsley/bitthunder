/**
 *	BitThunder Volume and Partition Manager
 *
 *
 **/

#include <bitthunder.h>
#include <lib/getmem.h>
#include "ibm_mbr.h"
#include <string.h>
#include <stdio.h>

BT_DEF_MODULE_NAME			("Volume and Partition Manager")
BT_DEF_MODULE_DESCRIPTION	("Volume manager for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

typedef enum _BT_VOLUME_TYPE {
	BT_VOLUME_NORMAL,
	BT_VOLUME_PARTITION,
} BT_VOLUME_TYPE;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	BT_LIST_ITEM		oItem;
	BT_VOLUME_TYPE 		eType;
	BT_HANDLE			hBlock;
	BT_u32				ulTotalBlocks;
	BT_HANDLE			hInode;
	BT_u32				ulReferenceCount;
};

typedef struct _BT_PARTITION {
	struct _BT_OPAQUE_HANDLE oVolume;
	BT_u32	ulBaseAddress;
	BT_u32	ulPartitionNumber;
} BT_PARTITION;

static BT_LIST g_oVolumes 		= {0};
//static BT_LIST g_oPartitions 	= {0};

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(BT_HANDLE hVolume, BT_ERROR *pError) {
	if(!hVolume->ulReferenceCount) {
		hVolume->ulReferenceCount += 1;
		return hVolume;
	}

	return NULL;
}

static const BT_DEVFS_OPS oDevfsOps = {
	.pfnOpen = devfs_open,
};

static BT_u32 BT_PartitionCount(BT_u8 *pBuffer)
{
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

BT_ERROR BT_EnumerateVolumes(BT_HANDLE hBlockDevice) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_BLOCK_GEOMETRY oGeometry;

	BT_i8 *szpName = BT_GetInodeName(BT_BlockGetInode(hBlockDevice), &Error);
	if(!szpName) {
		BT_kPrint("Block device has no named inode entry");
		return BT_ERR_GENERIC;
	}

	if(hBlockDevice->h.pIf->eType != BT_HANDLE_T_INODE) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	Error = BT_GetBlockGeometry(hBlockDevice, &oGeometry);
	if(Error) {
		goto err_out;
	}

	BT_u8 *pMBR = BT_kMalloc(oGeometry.ulBlockSize);
	if(!pMBR) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	if(BT_BlockRead(hBlockDevice, 0, 1, pMBR, &Error) != 1) {
		goto err_free_out;
	}
	
	// Create the volume handle.
	BT_u32 partCount = BT_PartitionCount(pMBR);
	if(!partCount) {
		BT_HANDLE hVolume = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
		if(!hVolume) {
			return BT_ERR_NO_MEMORY;
		}

		hVolume->eType 			= BT_VOLUME_NORMAL;
		hVolume->ulTotalBlocks 	= oGeometry.ulTotalBlocks;
		hVolume->hBlock 		= hBlockDevice;

		BT_ListAddItem(&g_oVolumes, &hVolume->oItem);

		BT_i8 *iname = BT_kMalloc(strlen(szpName) + 10);;
		sprintf(iname, "%s%d", szpName, 0);
		
		hVolume->hInode = BT_DeviceRegister(hVolume, iname, &oDevfsOps, &Error);
		BT_kFree(iname);

	} else {
		BT_u32 i;
		for(i = 0; i < partCount; i++) {
			BT_HANDLE hVolume = BT_CreateHandle(&oHandleInterface, sizeof(BT_PARTITION), &Error);
			if(!hVolume) {
				return BT_ERR_NO_MEMORY;
			}

			BT_PARTITION *pPart = (BT_PARTITION *) hVolume;

			hVolume->eType 				= BT_VOLUME_PARTITION;
			pPart->ulBaseAddress 		= BT_GetLongLE(pMBR, (IBM_MBR_PTBL + (16 * i) + IBM_MBR_PTBL_LBA));
			pPart->ulPartitionNumber 	= i;
			hVolume->ulTotalBlocks		= BT_GetLongLE(pMBR, (IBM_MBR_PTBL + (16 * i) + IBM_MBR_PTBL_SECTORS));
			hVolume->hBlock				= hBlockDevice;

			BT_ListAddItem(&g_oVolumes, &hVolume->oItem);

			BT_i8 *iname = BT_kMalloc(strlen(szpName) + 10);
			sprintf(iname, "%s%lu", szpName, i);
			hVolume->hInode = BT_DeviceRegister(hVolume, iname, &oDevfsOps, &Error);

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


BT_u32 BT_VolumeRead(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	
	if(hVolume->eType == BT_VOLUME_NORMAL) {
		return BT_BlockRead(hVolume->hBlock, ulAddress, ulBlocks, pBuffer, pError);
	}

	BT_PARTITION *pPart = (BT_PARTITION *)  hVolume;

	return BT_BlockRead(hVolume->hBlock, ulAddress + pPart->ulBaseAddress, ulBlocks, pBuffer, pError);
}

BT_u32 BT_VolumeWrite(BT_HANDLE hVolume, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	if(hVolume->eType == BT_VOLUME_NORMAL) {
		return BT_BlockWrite(hVolume->hBlock, ulAddress, ulBlocks, pBuffer, pError);
	}

	BT_PARTITION *pPart = (BT_PARTITION *)  hVolume;

	return BT_BlockWrite(hVolume->hBlock, ulAddress + pPart->ulBaseAddress, ulBlocks, pBuffer, pError);
}

static BT_ERROR bt_volume_inode_cleanup(BT_HANDLE hVolume) {

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	{NULL},
	BT_HANDLE_T_INODE,
	bt_volume_inode_cleanup,
};

static BT_ERROR bt_volume_manager_init() {
	BT_ListInit(&g_oVolumes);
	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_volume_manager_init,
};
