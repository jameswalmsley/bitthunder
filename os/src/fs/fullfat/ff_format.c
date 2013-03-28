/*****************************************************************************
 *     FullFAT - High Performance, Thread-Safe Embedded FAT File-System      *
 *                                                                           *
 *        Copyright(C) 2009  James Walmsley  <james@fullfat-fs.co.uk>        *
 *        Copyright(C) 2011  Hein Tibosch    <hein_tibosch@yahoo.es>         *
 *                                                                           *
 *    See RESTRICTIONS.TXT for extra restrictions on the use of FullFAT.     *
 *                                                                           *
 *    WARNING : COMMERCIAL PROJECTS MUST COMPLY WITH THE GNU GPL LICENSE.    *
 *                                                                           *
 *  Projects that cannot comply with the GNU GPL terms are legally obliged   *
 *    to seek alternative licensing. Contact James Walmsley for details.     *
 *                                                                           *
 *****************************************************************************
 *           See http://www.fullfat-fs.co.uk/ for more information.          *
 *****************************************************************************
 *  This program is free software: you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation, either version 3 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.    *
 *                                                                           *
 *  The Copyright of Hein Tibosch on this project recognises his efforts in  *
 *  contributing to this project. The right to license the project under     *
 *  any other terms (other than the GNU GPL license) remains with the        *
 *  original copyright holder (James Walmsley) only.                         *
 *                                                                           *
 *****************************************************************************
 *  Modification/Extensions/Bugfixes/Improvements to FullFAT must be sent to *
 *  James Walmsley for integration into the main development branch.         *
 *****************************************************************************/

/**
 *	@file		ff_format.c
 *	@author		James Walmsley
 *	@ingroup	FORMAT
 *
 *	@defgroup	FORMAT Independent FAT Formatter
 *	@brief		Provides an interface to format a partition with FAT.
 *
 *
 *
 **/


#include "ff_format.h"
#include "ff_types.h"
#include "ff_ioman.h"
#include "ff_fatdef.h"

static FF_T_SINT8 FF_PartitionCount (FF_T_UINT8 *pBuffer)
{
	FF_T_SINT8 count = 0;
	FF_T_SINT8 part;
	// Check PBR or MBR signature
	if (FF_getChar(pBuffer, FF_FAT_MBR_SIGNATURE) != 0x55 &&
		FF_getChar(pBuffer, FF_FAT_MBR_SIGNATURE) != 0xAA ) {
		// No MBR, but is it a PBR ?
		if (FF_getChar(pBuffer, 0) == 0xEB &&          // PBR Byte 0
		    FF_getChar(pBuffer, 2) == 0x90 &&          // PBR Byte 2
		    (FF_getChar(pBuffer, 21) & 0xF0) == 0xF0) {// PBR Byte 21 : Media byte
			return 1;	// No MBR but PBR exist then only one partition
		}
		return 0;   // No MBR and no PBR then no partition found
	}
	for (part = 0; part < 4; part++)  {
		FF_T_UINT8 active = FF_getChar(pBuffer, FF_FAT_PTBL + FF_FAT_PTBL_ACTIVE + (16 * part));
		FF_T_UINT8 part_id = FF_getChar(pBuffer, FF_FAT_PTBL + FF_FAT_PTBL_ID + (16 * part));
		// The first sector must be a MBR, then check the partition entry in the MBR
		if (active != 0x80 && (active != 0 || part_id == 0)) {
			break;
		}
		count++;
	}
	return count;
}


FF_ERROR FF_CreatePartitionTable(FF_IOMAN *pIoman, FF_T_UINT32 ulTotalDeviceBlocks, FF_PARTITION_TABLE *pPTable) {
	FF_BUFFER *pBuffer = FF_GetBuffer(pIoman, 0, FF_MODE_WRITE);
	{
		if(!pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED;
		}


	}
	FF_ReleaseBuffer(pIoman, pBuffer);

	return FF_ERR_NONE;
}


FF_ERROR FF_FormatPartition(FF_IOMAN *pIoman, FF_T_UINT32 ulPartitionNumber, FF_T_UINT32 ulClusterSize) {

	FF_BUFFER *pBuffer;
  	FF_T_SINT8	scPartitionCount;
	FF_T_UINT32 maxClusters, f16MaxClusters, f32MaxClusters;
	FF_T_UINT32 fatSize = 32; // Default to a fat32 format.

	FF_PARTITION_ENTRY partitionGeom;

	FF_T_UINT32 ulBPRLba; ///< The LBA of the boot partition record.

	FF_T_UINT32 fat32Size, fat16Size, newFat32Size, newFat16Size, finalFatSize;
	FF_T_UINT32 sectorsPerCluster = ulClusterSize / pIoman->BlkSize;

	FF_T_UINT32 ulReservedSectors, ulTotalSectors;

	FF_T_UINT32 ul32DataSectors, ul16DataSectors;
	FF_T_UINT32 i;

	FF_T_UINT32 ulClusterBeginLBA;

	FF_ERROR	Error = FF_ERR_NONE;

	// Get Partition Metrics, and pass on to FF_Format() function

	pBuffer = FF_GetBuffer(pIoman, 0, FF_MODE_READ);
	{
		if(!pBuffer) {
			return FF_ERR_DEVICE_DRIVER_FAILED | FF_FORMATPARTITION;
		}


		scPartitionCount = FF_PartitionCount(pBuffer->pBuffer);

		if(!scPartitionCount) {
			// Get Partition Geom from volume boot record.
			ulBPRLba = 0;
			partitionGeom.ulStartLBA = FF_getShort(pBuffer->pBuffer, FF_FAT_RESERVED_SECTORS); // Get offset to start of where we can actually put the FAT table.
			ulReservedSectors = partitionGeom.ulStartLBA;
			partitionGeom.ulLength = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, FF_FAT_16_TOTAL_SECTORS);

			if(partitionGeom.ulLength == 0) { // 32-bit entry was used.
				partitionGeom.ulLength = FF_getLong(pBuffer->pBuffer, FF_FAT_32_TOTAL_SECTORS);
			}

			ulTotalSectors = partitionGeom.ulLength;

			partitionGeom.ulLength -= partitionGeom.ulStartLBA; // Remove the reserved sectors from the count.

		} else {
			// Get partition Geom from the partition table entry.

		}

		// Calculate the max possiblenumber of clusters based on clustersize.
		maxClusters = partitionGeom.ulLength / sectorsPerCluster;

		// Determine the size of a FAT table required to support this.
		fat32Size = (maxClusters * 4) / pIoman->BlkSize; // Potential size in sectors of a fat32 table.
		if((maxClusters * 4) % pIoman->BlkSize) {
			fat32Size++;
		}
		fat32Size *= 2;	// Officially there are 2 copies of the FAT.

		fat16Size = (maxClusters * 2) / pIoman->BlkSize; // Potential size in bytes of a fat16 table.
		if((maxClusters * 2) % pIoman->BlkSize) {
			fat16Size++;
		}
		fat16Size *= 2;

		// A real number of sectors to be available is therefore ~~
		ul16DataSectors = partitionGeom.ulLength - fat16Size;
		ul32DataSectors = partitionGeom.ulLength - fat32Size;

		f16MaxClusters = ul16DataSectors / sectorsPerCluster;
		f32MaxClusters = ul32DataSectors / sectorsPerCluster;

		newFat16Size = (f16MaxClusters * 2) / pIoman->BlkSize;
		if((f16MaxClusters * 2) % pIoman->BlkSize) {
			newFat16Size++;
		}

		newFat32Size = (f32MaxClusters * 4) / pIoman->BlkSize;
		if((f32MaxClusters * 4) % pIoman->BlkSize) {
			newFat32Size++;
		}

		// Now determine if this should be fat16/32 format?

		if(f16MaxClusters < 65525) {
			fatSize = 16;
			finalFatSize = newFat16Size;
		} else {
			fatSize = 32;
			finalFatSize = newFat32Size;
		}

		FF_ReleaseBuffer(pIoman, pBuffer);
		for(i = 0; i < finalFatSize*2; i++) { // Ensure the FAT table is clear.
			if(i == 0) {
				pBuffer = FF_GetBuffer(pIoman, partitionGeom.ulStartLBA, FF_MODE_WR_ONLY);
				if(!pBuffer) {
					return FF_ERR_DEVICE_DRIVER_FAILED;
				}

				memset(pBuffer->pBuffer, 0, pIoman->BlkSize);
			} else {
				FF_BlockWrite(pIoman, partitionGeom.ulStartLBA+i, 1, pBuffer->pBuffer, FF_FALSE);
			}
		}

		switch(fatSize) {
		case 16: {
			FF_putShort(pBuffer->pBuffer, 0, 0xFFF8); // First FAT entry.
			FF_putShort(pBuffer->pBuffer, 2, 0xFFFF); // RESERVED alloc.
			break;
		}

		case 32: {
			FF_putLong(pBuffer->pBuffer, 0, 0x0FFFFFF8); // FAT32 FAT sig.
			FF_putLong(pBuffer->pBuffer, 4, 0xFFFFFFFF); // RESERVED alloc.
			FF_putLong(pBuffer->pBuffer, 8, 0x0FFFFFFF); // Root dir allocation.
			break;
		}

		default:
			break;
		}

		FF_ReleaseBuffer(pIoman, pBuffer);


		// Clear and initialise the root dir.
		ulClusterBeginLBA = partitionGeom.ulStartLBA + (finalFatSize*2);

		for(i = 0; i < sectorsPerCluster; i++) {
			if(i == 0) {
				pBuffer = FF_GetBuffer(pIoman, ulClusterBeginLBA, FF_MODE_WR_ONLY);
				memset(pBuffer->pBuffer, 0, pIoman->BlkSize);
			} else  {
				FF_BlockWrite(pIoman, ulClusterBeginLBA+i, 1, pBuffer->pBuffer, FF_FALSE);
			}

		}

		FF_ReleaseBuffer(pIoman, pBuffer);

		// Correctly modify the second FAT item again.
		pBuffer = FF_GetBuffer(pIoman, partitionGeom.ulStartLBA + finalFatSize, FF_MODE_WRITE);
		{
			switch(fatSize) {
			case 16: {
				FF_putShort(pBuffer->pBuffer, 0, 0xFFF8);
				FF_putShort(pBuffer->pBuffer, 2, 0xFFFF);
				break;
			}
			   
			case 32:
				FF_putLong(pBuffer->pBuffer, 0, 0x0FFFFFF8);
				FF_putLong(pBuffer->pBuffer, 4, 0xFFFFFFFF);
				FF_putLong(pBuffer->pBuffer, 8, 0x0FFFFFFF); // Root dir allocation.
			}
		}
		FF_ReleaseBuffer(pIoman, pBuffer);


		// Modify the fields in the VBR/PBR for correct mounting.
		pBuffer = FF_GetBuffer(pIoman, ulBPRLba, FF_MODE_WRITE); 		// Modify the FAT descriptions.
		{

			// -- First section Common vars to Fat12/16 and 32.
			memset(pBuffer->pBuffer, 0, pIoman->BlkSize); 				// Clear the boot record.

			FF_putChar(pBuffer->pBuffer, 0, 0xEB);						// Place the Jump to bootstrap x86 instruction.
			FF_putChar(pBuffer->pBuffer, 1, 0x3C);						// Even though we won't populate the bootstrap code.
			FF_putChar(pBuffer->pBuffer, 2, 0x90);						// Some devices look for this as a signature.

			memcpy(((FF_T_UINT8 *)pBuffer->pBuffer+3), "FULLFAT2", 8); // Place the FullFAT OEM code.

			FF_putShort(pBuffer->pBuffer, 11, pIoman->BlkSize);
			FF_putChar(pBuffer->pBuffer, 13, (FF_T_UINT8) sectorsPerCluster);

			FF_putShort(pBuffer->pBuffer, FF_FAT_RESERVED_SECTORS, (FF_T_UINT16)partitionGeom.ulStartLBA); 	// Number of reserved sectors. (1 for fat12/16, 32 for f32).
			FF_putShort(pBuffer->pBuffer, FF_FAT_NUMBER_OF_FATS, 2); 	// Always 2 copies.


			//FF_putShort(pBuffer->pBuffer, 19, 0);						// Number of sectors in partition if size < 32mb.

			FF_putChar(pBuffer->pBuffer, 21, 0xF8);             		// Media type -- HDD.
		   

			FF_putShort(pBuffer->pBuffer, 510, 0xAA55);					// MBR sig.
			FF_putLong(pBuffer->pBuffer, 32, partitionGeom.ulLength+partitionGeom.ulStartLBA); // Total sectors of this partition.

			if(fatSize == 32) {
				FF_putShort(pBuffer->pBuffer, 36, (FF_T_UINT16)finalFatSize);		// Number of sectors per fat. 
				FF_putShort(pBuffer->pBuffer, 44, 2);					// Root dir cluster (2).
				FF_putShort(pBuffer->pBuffer, 48, 1);					// FSINFO sector at LBA1.
				FF_putShort(pBuffer->pBuffer, 50, 6);					// 0 for no backup boot sector.
				FF_putChar(pBuffer->pBuffer, 66, 0x29);					// Indicate extended signature is present.
				memcpy(((FF_T_UINT8 *)pBuffer->pBuffer+71), "FullFAT2-V", 10); // Volume name.
				memcpy(((FF_T_UINT8 *)pBuffer->pBuffer+81), "FAT32   ", 8);

				// Put backup boot sector.
				FF_BlockWrite(pIoman, 6, 1, pBuffer->pBuffer, FF_FALSE);
			} else {
				FF_putChar(pBuffer->pBuffer, 38, 0x28);					// Signal this contains an extended signature.
				memcpy(((FF_T_UINT8 *)pBuffer->pBuffer+43), "FullFAT2-V", 10); // Volume name.
				memcpy(((FF_T_UINT8 *)pBuffer->pBuffer+54), "FAT16   ", 8);
				FF_putShort(pBuffer->pBuffer, FF_FAT_16_SECTORS_PER_FAT, (FF_T_UINT16) finalFatSize);
				FF_putShort(pBuffer->pBuffer, 17, 512);				 		// Number of Dir entries. (FAT32 0).
				//FF_putShort(pBuffer->pBuffer, FF_FAT_ROOT_ENTRY_COUNT, 
			}
		}
	}
	FF_ReleaseBuffer(pIoman, pBuffer);

	if(fatSize == 32) {
		// Finally if FAT32, create an FSINFO sector.
		pBuffer = FF_GetBuffer(pIoman, 1, FF_MODE_WRITE);
		{
			memset(pBuffer->pBuffer, 0, pIoman->BlkSize);
			FF_putLong(pBuffer->pBuffer, 0, 0x41615252);	// FSINFO sect magic number.
			FF_putLong(pBuffer->pBuffer, 484, 0x61417272);	// FSINFO second sig.
			// Calculate total sectors, -1 for the root dir allocation. (Free sectors count).
			FF_putLong(pBuffer->pBuffer, 488, ((ulTotalSectors - (ulReservedSectors + (2*finalFatSize))) / sectorsPerCluster)-1);
			FF_putLong(pBuffer->pBuffer, 492, 2);			// Hint for next free cluster.
			FF_putShort(pBuffer->pBuffer, 510, 0xAA55);
		}
		FF_ReleaseBuffer(pIoman, pBuffer);
	}

	FF_FlushCache(pIoman);

	return Error;
}

