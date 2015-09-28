/**
 *  BitThunder Partition Table modification library.
 *
 **/

#include <bitthunder.h>
#include "ibm_mbr.h"

static BT_u32 _partitionCount(BT_u8 *pBuffer) {
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

BT_u32 BT_PartitionCount(BT_HANDLE hBlk) {

    BT_BLOCK_GEOMETRY oGeom;
    BT_GetBlockGeometry(hBlk, &oGeom);
    BT_u8 *pMBR = BT_kMalloc(oGeom.ulBlockSize);

    BT_BlockRead(hBlk, 0, 1, pMBR);

    BT_u32 count = _partitionCount(pMBR);

    BT_kFree(pMBR);

    return count;
}
BT_EXPORT_SYMBOL(BT_PartitionCount);

BT_ERROR BT_PartitionInfo(BT_HANDLE hBlk, BT_u32 ulPartitionNumber, struct bt_part_t *partition) {
    BT_BLOCK_GEOMETRY oGeom;
    BT_GetBlockGeometry(hBlk, &oGeom);
    if(!partition) {
        return BT_ERR_NULL_POINTER;
    }

    BT_u8 *pMBR = BT_kMalloc(oGeom.ulBlockSize);

    BT_BlockRead(hBlk, 0, 1, pMBR);

    BT_u32 partCount = _partitionCount(pMBR);

    BT_u32 ulPartitionOffset = OFS_PTABLE_PART_0 + (16 * ulPartitionNumber);

    partition->ulStartLBA = BT_Get32LE(pMBR, ulPartitionOffset + OFS_PART_STARTING_LBA_32);
    partition->ulSectorCount = BT_Get32LE(pMBR, ulPartitionOffset + OFS_PART_LENGTH_32);
    partition->ucActive = BT_Get8(pMBR, ulPartitionOffset + OFS_PART_ACTIVE_8);

    BT_kFree(pMBR);

    return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_PartitionInfo);


BT_ERROR BT_Partition(const BT_i8 *device, BT_PARTITION_PARAMETERS *pParams) {

	BT_ERROR Error = BT_ERR_NONE;

	if(!pParams) {
		Error = BT_ERR_NULL_POINTER;
		goto err_out;
	}

	BT_HANDLE hBlock = BT_Open(device, 0, &Error);
	if(!hBlock) {
		goto err_out;		///< Cannot open volume device.
	}

	if(BT_HANDLE_TYPE(hBlock) != BT_HANDLE_T_BLOCK) {
		Error = BT_ERR_INVALID_VALUE;
		goto err_volume_out;	///< Input device is not a volume!
	}

	BT_BLOCK_GEOMETRY oGeom;
	BT_GetBlockGeometry(hBlock, &oGeom);

	BT_u32 partNum;
	BT_u32 partCount = 0;
	BT_u32 ulAvailable;
	BT_u32 ulSummedSizes = 0;

	BT_u32 ulReservedSpace;
	BT_u32 ulHiddenSectors;

	const BT_u32 ulInterSpace = pParams->ulInterSpace ? pParams->ulInterSpace : 2048; // Hidden space between 2 extended partitions.

	/* Get the sum of partition sizes. */
	for(partNum = 0; partNum < BT_PARTITION_MAX; partNum++) {
		if(pParams->ulSizes[partNum] > 0) {
			partCount++;
			ulSummedSizes += pParams->ulSizes[partNum];
		}
	}

	if(!partCount) {
		partCount = 1;
		if(pParams->eSizeType == BT_PARTITION_SIZE_SECTORS) {
			pParams->ulSizes[0] = oGeom.ulTotalBlocks;
		} else {
			pParams->ulSizes[0] = 100;
		}

		ulSummedSizes = pParams->ulSizes[0];
	}

	/* Normalise the primary parition count */
	if(pParams->ulPrimaryCount > (( partCount > 4) ? 3 : partCount)) {
		pParams->ulPrimaryCount = ( partCount > 4) ? 3 : partCount;
	}

	/* Are extended partitions requested */
	BT_u32 ulExtended = (partCount > pParams->ulPrimaryCount);
	if(ulExtended) {
		if(pParams->ulHiddenSectors < 4096) {
			pParams->ulHiddenSectors = 4096;
		}

		ulReservedSpace = ulInterSpace * (partCount - pParams->ulPrimaryCount);
	} else {
		ulReservedSpace = 0;
		if(pParams->ulHiddenSectors < 4096) {
			pParams->ulHiddenSectors = 4096;
		}
	}

	ulAvailable = oGeom.ulTotalBlocks - pParams->ulHiddenSectors - ulReservedSpace;

	/*
	 *	Check the sizes are valid.
	 */
	switch(pParams->eSizeType) {
		case BT_PARTITION_SIZE_QUOTA:
			break;
		case BT_PARTITION_SIZE_PERCENT:
			if(ulSummedSizes > 100) {
				Error = BT_ERR_INVALID_VALUE;
				goto err_volume_out;
			}
			ulSummedSizes = 100;
			break;
		case BT_PARTITION_SIZE_SECTORS:
			if(ulSummedSizes > ulAvailable) {
				Error = BT_ERR_INVALID_VALUE;
				goto err_volume_out;
			}
			break;

		default:
			Error = BT_ERR_INVALID_VALUE;
			goto err_volume_out;
	}

	BT_u32 ulPartitionNumber;
	BT_u32 ulRemaining = ulAvailable;
	BT_u32 ulLBA = pParams->ulHiddenSectors;

	struct bt_part_t oParts[BT_PARTITION_MAX];
	memset(oParts, 0, sizeof(oParts));

	for(ulPartitionNumber = 0; ulPartitionNumber < partCount; ulPartitionNumber++) {
		if(pParams->ulSizes[ulPartitionNumber] > 0) {
			BT_u32 ulSize;
			switch(pParams->eSizeType) {
				case BT_PARTITION_SIZE_QUOTA:
				case BT_PARTITION_SIZE_PERCENT:
					ulSize = ( BT_u32 ) ( ( ( BT_u64 ) pParams->ulSizes[ ulPartitionNumber ] * ulAvailable) / ulSummedSizes );
					break;
				case BT_PARTITION_SIZE_SECTORS:
					ulSize = pParams->ulSizes[ulPartitionNumber];
					break;
			}

			if(ulSize > ulRemaining) {
				ulSize = ulRemaining;
			}

			ulRemaining -= ulSize;
			oParts[ulPartitionNumber].ulSectorCount = ulSize;
			oParts[ulPartitionNumber].ucActive = 0x80;
			oParts[ulPartitionNumber].ulStartLBA = ulLBA;
			oParts[ulPartitionNumber].ucPartitionID = 0x0B;
			ulLBA += ulSize;
		}

	}

	if(ulExtended) {
		BT_u32 ulNextLBA;

	} else {
		void *pvBlockBuffer = BT_kMalloc(oGeom.ulBlockSize);
		BT_BlockRead(hBlock, 0, 1, pvBlockBuffer);

		memset(pvBlockBuffer, 0 , oGeom.ulBlockSize);
		memcpy(pvBlockBuffer + OFS_BPB_jmpBoot_24, "\xEB\x00\x90" "BitThunder", 13);
		BT_u32 ulPartitionOffset = OFS_PTABLE_PART_0;

		for(ulPartitionNumber = 0; ulPartitionNumber < 4; ulPartitionNumber++, ulPartitionOffset += 16) {
			BT_Put8(pvBlockBuffer, ulPartitionOffset + OFS_PART_ACTIVE_8, 	oParts[ulPartitionNumber].ucActive);
			BT_Put8(pvBlockBuffer, ulPartitionOffset + OFS_PART_START_HEAD_8, 1);
			BT_Put8(pvBlockBuffer, ulPartitionOffset + OFS_PART_START_SEC_TRACK_16, 1);
			BT_Put8(pvBlockBuffer, ulPartitionOffset + OFS_PART_ID_NUMBER_8, oParts[ulPartitionNumber].ucPartitionID);
			BT_Put8(pvBlockBuffer, ulPartitionOffset + OFS_PART_ENDING_HEAD_8, 0xFE);
			BT_Put16LE(pvBlockBuffer, ulPartitionOffset + OFS_PART_ENDING_SEC_TRACK_16, oParts[ulPartitionNumber].ulSectorCount);
			BT_Put32LE(pvBlockBuffer, ulPartitionOffset + OFS_PART_STARTING_LBA_32, oParts[ulPartitionNumber].ulStartLBA);
			BT_Put32LE(pvBlockBuffer, ulPartitionOffset + OFS_PART_LENGTH_32, oParts[ulPartitionNumber].ulSectorCount);
		}

		BT_Put8(pvBlockBuffer, 510, 0x55);
		BT_Put8(pvBlockBuffer, 511, 0xAA);

		BT_BlockWrite(hBlock, 0, 1, pvBlockBuffer);

		BT_kFree(pvBlockBuffer);
	}

    BT_EnumerateVolumes(hBlock);    // Re-enumerate the volumes!

err_volume_out:
	BT_CloseHandle(hBlock);

err_out:

	return Error;
}
BT_EXPORT_SYMBOL(BT_Partition);
