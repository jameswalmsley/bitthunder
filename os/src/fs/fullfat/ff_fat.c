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
 *	@file		ff_fat.c
 *	@author		James Walmsley
 *	@ingroup	FAT
 *
 *	@defgroup	FAT Fat File-System
 *	@brief		Handles FAT access and traversal.
 *
 *	Provides file-system interfaces for the FAT file-system.
 **/

#include "ff_fat.h"
#include "ff_config.h"
#include <string.h>

struct SFatStat fatStat;

void FF_lockFAT(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);	// Use Semaphore to protect FAT modifications.
	{
		while((pIoman->Locks & FF_FAT_LOCK)) {
			FF_ReleaseSemaphore(pIoman->pSemaphore);
			FF_Yield();						// Keep Releasing and Yielding until we have the Fat protector.
			FF_PendSemaphore(pIoman->pSemaphore);
		}
		pIoman->Locks |= FF_FAT_LOCK;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

void FF_unlockFAT(FF_IOMAN *pIoman) {
	FF_PendSemaphore(pIoman->pSemaphore);
	{
		pIoman->Locks &= ~FF_FAT_LOCK;
	}
	FF_ReleaseSemaphore(pIoman->pSemaphore);
}

/**
 *	@private
 **/
FF_T_UINT32 FF_getRealLBA(FF_IOMAN *pIoman, FF_T_UINT32 LBA) {
	return LBA * pIoman->pPartition->BlkFactor;
}

/**
 *	@private
 **/
FF_T_UINT32 FF_Cluster2LBA(FF_IOMAN *pIoman, FF_T_UINT32 Cluster) {
	FF_T_UINT32 lba = 0;
	FF_PARTITION *pPart;
	if(pIoman) {
		pPart = pIoman->pPartition;

		if(Cluster > 1) {
			lba = ((Cluster - 2) * pPart->SectorsPerCluster) + pPart->FirstDataSector;
		} else {
			lba = pPart->ClusterBeginLBA;
		}
	}
	return lba;
}

/**
 *	@private
 **/
FF_T_UINT32 FF_LBA2Cluster(FF_IOMAN *pIoman, FF_T_UINT32 Address) {
	FF_T_UINT32 cluster = 0;
	FF_PARTITION *pPart;
	if(pIoman) {
		pPart = pIoman->pPartition;
		if(pPart->Type == FF_T_FAT32) {
			cluster = ((Address - pPart->ClusterBeginLBA) / pPart->SectorsPerCluster) + 2;
		} else {
			cluster = ((Address - pPart->ClusterBeginLBA) / pPart->SectorsPerCluster);
		}
	}
	return cluster;
}


FF_ERROR FF_ReleaseFatBuffer (FF_IOMAN *pIoman, FF_FatBuffers *pBuffer)
{
	FF_T_INT i;
	FF_ERROR Error = FF_ERR_NONE;
	for (i = 0; i < BUF_STORE_COUNT; i++) {
		if (pBuffer->pBuffers[i]) {
			Error = FF_ReleaseBuffer(pIoman, pBuffer->pBuffers[i]);
			if(FF_isERR(Error)) {
				break;
			}
			pBuffer->pBuffers[i] = NULL;
		}
	}
	fatStat.clearCount++;

	return Error;
}

/**
 *	@private
 **/
FF_T_UINT32 FF_getFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster, FF_ERROR *pError, FF_FatBuffers *pFatBuf) {

	FF_BUFFER 	*pBuffer = NULL;
	FF_T_UINT32 FatOffset;
	FF_T_UINT32 FatSector;
	FF_T_UINT32 FatSectorEntry;
	FF_T_UINT32 FatEntry;
	FF_T_UINT	LBAadjust;
	FF_T_UINT32 relClusterEntry;
	// preferred mode, user might want to update this entry
	FF_T_UINT8  Mode = pFatBuf ? pFatBuf->Mode : FF_MODE_READ;

#ifdef FF_FAT12_SUPPORT
	FF_T_UINT8	F12short[2];		// For FAT12 FAT Table Across sector boundary traversal.
#endif
	*pError = FF_ERR_NONE;

	if (nCluster >= pIoman->pPartition->NumClusters) {
		// HT: find a more specific error code
		*pError = FF_ERR_IOMAN_NOT_ENOUGH_FREE_SPACE | FF_GETFATENTRY;
		return 0;
	}
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		FatOffset = nCluster * 4;
	} else if(pIoman->pPartition->Type == FF_T_FAT16) {
		FatOffset = nCluster * 2;
	}else {
		FatOffset = nCluster + (nCluster / 2);
	}

	FatSector		= pIoman->pPartition->FatBeginLBA + (FatOffset / pIoman->pPartition->BlkSize);
	FatSectorEntry	= FatOffset % pIoman->pPartition->BlkSize;

	LBAadjust		= (FF_T_UINT)	(FatSectorEntry / pIoman->BlkSize);
	relClusterEntry = FatSectorEntry % pIoman->BlkSize;

	FatSector = FF_getRealLBA(pIoman, FatSector) + LBAadjust;

#ifdef FF_FAT12_SUPPORT
	if(pIoman->pPartition->Type == FF_T_FAT12) {
		if(relClusterEntry == (FF_T_UINT32)((pIoman->BlkSize - 1))) {
			// Fat Entry SPANS a Sector!
			// First Buffer get the last Byte in buffer (first byte of our address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector, Mode);
			{
				if(!pBuffer) {
					*pError = FF_ERR_DEVICE_DRIVER_FAILED | FF_GETFATENTRY;
					return 0;
				}
				F12short[0] = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16)(pIoman->BlkSize - 1));
			}
			*pError = FF_ReleaseBuffer(pIoman, pBuffer);
			if(FF_isERR(*pError)) {
				return 0;
			}
			// Second Buffer get the first Byte in buffer (second byte of out address)!
			pBuffer = FF_GetBuffer(pIoman, FatSector + 1, Mode);
			{
				if(!pBuffer) {
					*pError = FF_ERR_DEVICE_DRIVER_FAILED | FF_GETFATENTRY;
					return 0;
				}
				F12short[1] = FF_getChar(pBuffer->pBuffer, 0);
			}
			*pError = FF_ReleaseBuffer(pIoman, pBuffer);
			if(FF_isERR(*pError)) {
				return 0;
			}

			FatEntry = (FF_T_UINT32) FF_getShort((FF_T_UINT8*)&F12short, 0);	// Guarantee correct Endianess!

			if(nCluster & 0x0001) {
				FatEntry = FatEntry >> 4;
			}
			FatEntry &= 0x0FFF;
			return (FF_T_SINT32) FatEntry;
		}
	}
#endif
	if (pFatBuf) {
		FF_BUFFER *buf = pFatBuf->pBuffers[0];
		if (buf) {
			if (buf->Sector == FatSector) {
				pBuffer = buf;
				fatStat.reuseCount[0]++;
			} else {
				*pError = FF_ReleaseBuffer(pIoman, buf);
				if(FF_isERR(*pError)) {
					return 0;
				}
				pFatBuf->pBuffers[0] = NULL;
				fatStat.missCount[0]++;
			}
		} else {
			fatStat.getCount[0]++;
		}
	}
	if (!pBuffer)
		pBuffer = FF_GetBuffer(pIoman, FatSector, Mode);
	{
		if(!pBuffer) {
			*pError = FF_ERR_DEVICE_DRIVER_FAILED | FF_GETFATENTRY;
			return 0;
		}

		switch(pIoman->pPartition->Type) {
			case FF_T_FAT32:
				FatEntry = FF_getLong(pBuffer->pBuffer, relClusterEntry);
				FatEntry &= 0x0fffffff;	// Clear the top 4 bits.
				break;

			case FF_T_FAT16:
				FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
				break;

#ifdef FF_FAT12_SUPPORT
			case FF_T_FAT12:
				FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
				if(nCluster & 0x0001) {
					FatEntry = FatEntry >> 4;
				}
				FatEntry &= 0x0FFF;
				break;
#endif
			default:
				FatEntry = 0;
				break;
		}
	}
	if (pFatBuf) {
		pFatBuf->pBuffers[0] = pBuffer;
	} else {
		*pError = FF_ReleaseBuffer(pIoman, pBuffer);
		if(FF_isERR(*pError)) {
			return 0;
		}
	}

	return (FF_T_SINT32) FatEntry;
}

FF_ERROR FF_ClearCluster(FF_IOMAN *pIoman, FF_T_UINT32 nCluster) {
	FF_BUFFER *pBuffer = NULL;
	FF_T_INT i;
	FF_T_UINT32	BaseLBA;
	FF_ERROR slRetVal = FF_ERR_NONE;

	BaseLBA = FF_Cluster2LBA(pIoman, nCluster);
	BaseLBA = FF_getRealLBA(pIoman, BaseLBA);

	for(i = 0; i < pIoman->pPartition->SectorsPerCluster; i++) {
		if (i == 0) {
			pBuffer = FF_GetBuffer(pIoman, BaseLBA, FF_MODE_WR_ONLY);
			if(!pBuffer) {
				return FF_ERR_DEVICE_DRIVER_FAILED | FF_CLEARCLUSTER;
			}
			memset(pBuffer->pBuffer, 0x00, pIoman->BlkSize);
		}
		slRetVal = FF_BlockWrite(pIoman, BaseLBA+i, 1, pBuffer->pBuffer, FF_FALSE);
		if(slRetVal < 0) {
			break;
		}
	}
	pBuffer->Modified = FF_FALSE;

	if(FF_isERR(slRetVal)) {
		FF_ReleaseBuffer(pIoman, pBuffer);
		return slRetVal;
	}

	slRetVal = FF_ReleaseBuffer(pIoman, pBuffer);

	return slRetVal;
}

/**
 *	@private
 *	@brief	Returns the Cluster address of the Cluster number from the beginning of a chain.
 *
 *	@param	pIoman		FF_IOMAN Object
 *	@param	Start		Cluster address of the first cluster in the chain.
 *	@param	Count		Number of Cluster in the chain,
 *
 *
 *
 **/
FF_T_UINT32 FF_TraverseFAT(FF_IOMAN *pIoman, FF_T_UINT32 Start, FF_T_UINT32 Count, FF_ERROR *pError) {

	FF_T_UINT32 i;
	FF_T_UINT32 fatEntry = Start, currentCluster = Start;
	FF_FatBuffers FatBuf;
	FF_InitFatBuffer (&FatBuf, FF_MODE_READ);

	*pError = FF_ERR_NONE;

	for(i = 0; i < Count; i++) {
		fatEntry = FF_getFatEntry(pIoman, currentCluster, pError, &FatBuf);
		if(FF_isERR(*pError)) {
			fatEntry = 0;
			break;
		}

		if(FF_isEndOfChain(pIoman, fatEntry)) {
			fatEntry = currentCluster;
			break;
		}
		currentCluster = fatEntry;
	}
	*pError = FF_ReleaseFatBuffer(pIoman, &FatBuf);

	return fatEntry;
}

FF_T_UINT32 FF_FindEndOfChain(FF_IOMAN *pIoman, FF_T_UINT32 Start, FF_ERROR *pError) {

	FF_T_UINT32 fatEntry = Start, currentCluster = Start;
	FF_FatBuffers FatBuf;
	FF_InitFatBuffer (&FatBuf, FF_MODE_READ);
	*pError = FF_ERR_NONE;

	while(!FF_isEndOfChain(pIoman, fatEntry)) {
		fatEntry = FF_getFatEntry(pIoman, currentCluster, pError, &FatBuf);
		if(FF_isERR(*pError)) {
			fatEntry = 0;
			break;
		}

		if(FF_isEndOfChain(pIoman, fatEntry)) {
			fatEntry = currentCluster;
			break;
		}
		currentCluster = fatEntry;
	}
	*pError = FF_ReleaseFatBuffer(pIoman, &FatBuf);

	return fatEntry;
}


/**
 *	@private
 *	@brief	Tests if the fatEntry is an End of Chain Marker.
 *
 *	@param	pIoman		FF_IOMAN Object
 *	@param	fatEntry	The fat entry from the FAT table to be checked.
 *
 *	@return	FF_TRUE if it is an end of chain, otherwise FF_FALSE.
 *
 **/
FF_T_BOOL FF_isEndOfChain(FF_IOMAN *pIoman, FF_T_UINT32 fatEntry) {
	FF_T_BOOL result = FF_FALSE;
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		if((fatEntry & 0x0fffffff) >= 0x0ffffff8) {
			result = FF_TRUE;
		}
	} else if(pIoman->pPartition->Type == FF_T_FAT16) {
		if(fatEntry >= 0x0000fff8) {
			result = FF_TRUE;
		}
	} else {
		if(fatEntry >= 0x00000ff8) {
			result = FF_TRUE;
		}
	}
	if(fatEntry == 0x00000000) {
		result = FF_TRUE;	//Perhaps trying to read a deleted file!
	}
	return result;
}


/**
 *	@private
 *	@brief	Writes a new Entry to the FAT Tables.
 *
 *	@param	pIoman		IOMAN object.
 *	@param	nCluster	Cluster Number to be modified.
 *	@param	Value		The Value to store.
 **/
FF_ERROR FF_putFatEntry(FF_IOMAN *pIoman, FF_T_UINT32 nCluster, FF_T_UINT32 Value, FF_FatBuffers *pFatBuf) {

	FF_BUFFER 	*pBuffer = NULL;
	FF_T_UINT32 FatOffset;
	FF_T_UINT32 FatSector;
	FF_T_UINT32 FatSectorEntry;
	FF_T_UINT32 FatEntry;
	FF_T_UINT	LBAadjust;
	FF_T_UINT32 relClusterEntry;
	FF_ERROR Error;
#ifdef FF_FAT12_SUPPORT
	FF_T_UINT8	F12short[2];		// For FAT12 FAT Table Across sector boundary traversal.
#endif

	FF_T_INT i;

	// HT: avoid corrupting the disk
	if (!nCluster || nCluster >= pIoman->pPartition->NumClusters) {
		// find a more specific error code
		return FF_ERR_IOMAN_NOT_ENOUGH_FREE_SPACE | FF_PUTFATENTRY;
	}
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		FatOffset = nCluster * 4;
	} else if(pIoman->pPartition->Type == FF_T_FAT16) {
		FatOffset = nCluster * 2;
	}else {
		FatOffset = nCluster + (nCluster / 2);
	}

	FatSector = pIoman->pPartition->FatBeginLBA + (FatOffset / pIoman->pPartition->BlkSize);
	FatSectorEntry = FatOffset % pIoman->pPartition->BlkSize;

	LBAadjust = (FF_T_UINT) (FatSectorEntry / pIoman->BlkSize);
	relClusterEntry = FatSectorEntry % pIoman->BlkSize;

	FatSector = FF_getRealLBA(pIoman, FatSector); // LBA * pIoman->pPartition->BlkFactor;
	FatSector += LBAadjust;

#ifdef FF_FAT12_SUPPORT
	 if(pIoman->pPartition->Type == FF_T_FAT12) {
		  if(relClusterEntry == (FF_T_UINT32) (pIoman->BlkSize - 1)) {

				// Fat Entry SPANS a Sector!
				// First Buffer get the last Byte in buffer (first byte of our address)!
				pBuffer = FF_GetBuffer(pIoman, FatSector, FF_MODE_READ);
				{
					 if(!pBuffer) {
						  return FF_ERR_DEVICE_DRIVER_FAILED | FF_PUTFATENTRY;
					 }
					 F12short[0] = FF_getChar(pBuffer->pBuffer, pIoman->BlkSize - 1);
				}
				Error = FF_ReleaseBuffer(pIoman, pBuffer);
				if(FF_isERR(Error)) {
					return Error;
				}
				// Second Buffer get the first Byte in buffer (second byte of out address)!
				pBuffer = FF_GetBuffer(pIoman, FatSector + 1, FF_MODE_READ);
				{
					 if(!pBuffer) {
						  return FF_ERR_DEVICE_DRIVER_FAILED | FF_PUTFATENTRY;
					 }
					 F12short[1] = FF_getChar(pBuffer->pBuffer, (FF_T_UINT16) 0x0000);
				}
				Error = FF_ReleaseBuffer(pIoman, pBuffer);
				if(FF_isERR(Error)) {
					return Error;
				}

				FatEntry = FF_getShort((FF_T_UINT8*)&F12short, (FF_T_UINT16) 0x0000);	// Guarantee correct Endianess!
				if(nCluster & 0x0001) {
					 FatEntry   &= 0x000F;
					 Value		= (Value << 4);
					 Value	   &= 0xFFF0;
				}  else {
					 FatEntry	&= 0xF000;
					 Value		&= 0x0FFF;
				}

				FF_putShort((FF_T_UINT8 *)F12short, 0x0000, (FF_T_UINT16) (FatEntry | Value));

#ifdef FF_WRITE_BOTH_FATS
				for (i = 0; i < pIoman->pPartition->NumFATS; i++) {
					FatSector += (i * pIoman->pPartition->SectorsPerFAT);
#endif

					pBuffer = FF_GetBuffer(pIoman, FatSector, FF_MODE_WRITE);
					{
						if(!pBuffer) {
							return FF_ERR_DEVICE_DRIVER_FAILED | FF_PUTFATENTRY;
						}
						FF_putChar(pBuffer->pBuffer, (FF_T_UINT16)(pIoman->BlkSize - 1), F12short[0]);
					}
					Error = FF_ReleaseBuffer(pIoman, pBuffer);
					if(FF_isERR(Error)) {
						return Error;
					}

					 // Second Buffer get the first Byte in buffer (second byte of out address)!
					pBuffer = FF_GetBuffer(pIoman, FatSector + 1, FF_MODE_WRITE); // changed to MODE_WRITE -- BUG???
					{
						if(!pBuffer) {
							return FF_ERR_DEVICE_DRIVER_FAILED | FF_PUTFATENTRY;
						}
						FF_putChar(pBuffer->pBuffer, 0x0000, F12short[1]);
					}
					Error = FF_ReleaseBuffer(pIoman, pBuffer);
					if(FF_isERR(Error)) {
						return Error;
					}

#ifdef FF_WRITE_BOTH_FATS
				}
#endif

				return FF_ERR_NONE;
		  }
	 }
#endif

#ifdef FF_WRITE_BOTH_FATS
	for (i = 0; i < pIoman->pPartition->NumFATS;
		i++, FatSector += pIoman->pPartition->SectorsPerFAT)
#else
	// Will be optimized away by compiler
	for (i = 0; i < 1; i++)
#endif
	{

		if (i < BUF_STORE_COUNT && pFatBuf) {
			FF_BUFFER *buf = pFatBuf->pBuffers[i];
			if (buf) {
				if (buf->Sector == FatSector && (buf->Mode & FF_MODE_WRITE)) {
					// Same sector, correct mode: we can reuse it
					pBuffer = buf;
					fatStat.reuseCount[1]++;
				} else {
					Error = FF_ReleaseBuffer(pIoman, buf);
					if(FF_isERR(Error)) {
						return Error;
					}
					pFatBuf->pBuffers[i] = NULL;
					fatStat.missCount[1]++;
					pBuffer = NULL;
				}
			} else {
				fatStat.getCount[1]++;
			}
		}
		if (!pBuffer)
			pBuffer = FF_GetBuffer(pIoman, FatSector, FF_MODE_WRITE);
		{
			if(!pBuffer) {
				return FF_ERR_DEVICE_DRIVER_FAILED | FF_PUTFATENTRY;
			}
			if(pIoman->pPartition->Type == FF_T_FAT32) {
				Value &= 0x0fffffff;	// Clear the top 4 bits.
				FF_putLong(pBuffer->pBuffer, relClusterEntry, Value);
			} else if(pIoman->pPartition->Type == FF_T_FAT16) {
				FF_putShort(pBuffer->pBuffer, relClusterEntry, (FF_T_UINT16) Value);
			} else {
				FatEntry	= (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, relClusterEntry);
				if(nCluster & 0x0001) {
					FatEntry   &= 0x000F;
					Value		= (Value << 4);
					Value	   &= 0xFFF0;
				}  else {
					FatEntry	&= 0xF000;
					Value		&= 0x0FFF;
				}

				FF_putShort(pBuffer->pBuffer, relClusterEntry, (FF_T_UINT16) (FatEntry | Value));
			}
		}
		if (i < BUF_STORE_COUNT && pFatBuf) {
			// Store it for later use
			pFatBuf->pBuffers[i] = pBuffer;
			pFatBuf->Mode = FF_MODE_WRITE;
		} else {
			Error = FF_ReleaseBuffer(pIoman, pBuffer);
			if(FF_isERR(Error)) {
				return Error;
			}
		}
		pBuffer = NULL;
	}

	return FF_ERR_NONE;
}



/**
 *	@private
 *	@brief	Finds a Free Cluster and returns its number.
 *
 *	@param	pIoman	IOMAN Object.
 *
 *	@return	The number of the cluster found to be free.
 *	@return 0 on error.
 **/
#ifdef FF_FAT12_SUPPORT
FF_T_UINT32 FF_FindFreeClusterOLD(FF_IOMAN *pIoman, FF_ERROR *pError) {
	FF_T_UINT32 nCluster = 0;
	FF_T_UINT32 fatEntry;
	FF_FatBuffers FatBuf;
	FF_InitFatBuffer (&FatBuf, FF_MODE_READ);

	*pError = FF_ERR_NONE;

	for(nCluster = pIoman->pPartition->LastFreeCluster; nCluster < pIoman->pPartition->NumClusters; nCluster++) {
		fatEntry = FF_getFatEntry(pIoman, nCluster, pError, &FatBuf);
		if(FF_isERR(*pError)) {
			nCluster = 0;
			break;
		}
		if(fatEntry == 0x00000000) {
			pIoman->pPartition->LastFreeCluster = nCluster;
			break;

		}
	}
	*pError = FF_ReleaseFatBuffer(pIoman, &FatBuf);
	return nCluster;
}
#endif

FF_T_UINT32 FF_FindFreeCluster(FF_IOMAN *pIoman, FF_ERROR *pError) {
	FF_BUFFER	*pBuffer;
	FF_T_UINT32	x, nCluster = pIoman->pPartition->LastFreeCluster;
	FF_T_UINT32	FatOffset;
	FF_T_UINT32	FatSector;
	FF_T_UINT32	FatSectorEntry;
	FF_T_UINT32	EntriesPerSector;
	FF_T_UINT32 FatEntry = 1;
	FF_ERROR Error;
	const FF_T_INT EntrySize = (pIoman->pPartition->Type == FF_T_FAT32) ? 4 : 2;
	const FF_T_UINT32 uNumClusters = pIoman->pPartition->NumClusters;

	Error = FF_ERR_NONE;

#ifdef FF_FAT12_SUPPORT
	if(pIoman->pPartition->Type == FF_T_FAT12) {	// FAT12 tables are too small to optimise, and would make it very complicated!
		return FF_FindFreeClusterOLD(pIoman, pError);
	}
#endif

#ifdef FF_FSINFO_TRUSTED
	if(pIoman->pPartition->Type == FF_T_FAT32 && !pIoman->pPartition->LastFreeCluster) {
		pBuffer = FF_GetBuffer(pIoman, pIoman->pPartition->FSInfoLBA, FF_MODE_READ);
		{
			if(!pBuffer) {
				if(pError) {
					*pError = FF_ERR_DEVICE_DRIVER_FAILED | FF_FINDFREECLUSTER;
				}
				return 0;
			}
		}

		if(FF_getLong(pBuffer->pBuffer, 0) == 0x41615252 && FF_getLong(pBuffer->pBuffer, 484) == 0x61417272) {
			nCluster = FF_getLong(pBuffer->pBuffer, 492);
		}

		Error = FF_ReleaseBuffer(pIoman, pBuffer);
	}
#endif

	EntriesPerSector = pIoman->BlkSize / EntrySize;
	FatOffset = nCluster * EntrySize;

	for(FatSector = (FatOffset / pIoman->pPartition->BlkSize);
		FatSector < pIoman->pPartition->SectorsPerFAT;
		FatSector++) {
		pBuffer = FF_GetBuffer(pIoman, pIoman->pPartition->FatBeginLBA + FatSector, FF_MODE_READ);
		{
			if(!pBuffer) {
				*pError = FF_ERR_DEVICE_DRIVER_FAILED | FF_FINDFREECLUSTER;
				return 0;
			}
			for(x = nCluster % EntriesPerSector; x < EntriesPerSector; x++) {
				// HT double-check: don't use non-existing clusters
				if (nCluster >= uNumClusters) {
					FF_ReleaseBuffer(pIoman, pBuffer);	// Returning an error already, so don't check error here.
					*pError = FF_ERR_IOMAN_NOT_ENOUGH_FREE_SPACE | FF_FINDFREECLUSTER;
					return 0;
				}
				FatSectorEntry	= FatOffset % pIoman->pPartition->BlkSize;
				if(pIoman->pPartition->Type == FF_T_FAT32) {
					FatEntry = FF_getLong(pBuffer->pBuffer, FatSectorEntry);
					FatEntry &= 0x0fffffff;	// Clear the top 4 bits.
				} else {
					FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, FatSectorEntry);
				}
				if(FatEntry == 0x00000000) {
					*pError = FF_ReleaseBuffer(pIoman, pBuffer);
					if(FF_isERR(*pError)) {
						return 0;
					}
					pIoman->pPartition->LastFreeCluster = nCluster;
					return nCluster;
				}
				FatOffset += EntrySize;
				nCluster++;
			}
		}
		Error = FF_ReleaseBuffer(pIoman, pBuffer);
		if(FF_isERR(Error)) {
			if(pError) {
				*pError = Error;
			}
			return 0;
		}
	}
	if(pError) {
		*pError = FF_ERR_IOMAN_NOT_ENOUGH_FREE_SPACE | FF_FINDFREECLUSTER;
	}
	return 0;
}

/**
 * @private
 * @brief	Create's a Cluster Chain
 *	@return > 0 New created cluster
 *	@return = 0 See pError
 **/
FF_T_UINT32 FF_CreateClusterChain(FF_IOMAN *pIoman, FF_ERROR *pError) {
	FF_T_UINT32	iStartCluster;
	FF_ERROR	Error;
	*pError = FF_ERR_NONE;

	FF_lockFAT(pIoman);
	{
		iStartCluster = FF_FindFreeCluster(pIoman, &Error);
		if(FF_isERR(Error)) {
			*pError = Error;
			FF_unlockFAT(pIoman);
			return 0;
		}

		if(iStartCluster) {
			Error = FF_putFatEntry(pIoman, iStartCluster, 0xFFFFFFFF, NULL); // Mark the cluster as End-Of-Chain
			if(FF_isERR(Error)) {
				*pError = Error;
				FF_unlockFAT(pIoman);
				return 0;
			}
		}
	}
	FF_unlockFAT(pIoman);

	if(iStartCluster) {
		Error = FF_DecreaseFreeClusters(pIoman, 1);
		if(FF_isERR(Error)) {
			*pError = Error;
			return 0;
		}
	}

	return iStartCluster;
}

FF_T_UINT32 FF_GetChainLength(FF_IOMAN *pIoman, FF_T_UINT32 pa_nStartCluster, FF_T_UINT32 *piEndOfChain, FF_ERROR *pError) {
	FF_T_UINT32 iLength = 0;
	FF_FatBuffers FatBuf;
	FF_InitFatBuffer (&FatBuf, FF_MODE_READ);

	*pError = FF_ERR_NONE;

	FF_lockFAT(pIoman);
	{
		while(!FF_isEndOfChain(pIoman, pa_nStartCluster)) {
			pa_nStartCluster = FF_getFatEntry(pIoman, pa_nStartCluster, pError, &FatBuf);
			if(FF_isERR(*pError)) {
				iLength = 0;
				break;
			}
			iLength++;
		}
		if(piEndOfChain) {
			*piEndOfChain = pa_nStartCluster;
		}
	}
	*pError = FF_ReleaseFatBuffer(pIoman, &FatBuf);
	FF_unlockFAT(pIoman);
	return iLength;
}

/**
 *	@private
 *	@brief Free's Disk space by freeing unused links on Cluster Chains
 *
 *	@param	pIoman,			IOMAN object.
 *	@param	StartCluster	Cluster Number that starts the chain.
 *	@param	Count			Number of Clusters from the end of the chain to unlink.
 *	@param	Count			0 Means Free the entire chain (delete file).
 *	@param	Count			1 Means mark the start cluster with EOF.
 *
 *	@return 0 On Success.
 *	@return	-1 If the device driver failed to provide access.
 *
 **/
FF_ERROR FF_UnlinkClusterChain(FF_IOMAN *pIoman, FF_T_UINT32 StartCluster, FF_T_BOOL bTruncate) {

	FF_T_UINT32 fatEntry;
	FF_T_UINT32 currentCluster;
	FF_T_UINT32	iLen = 0;
	FF_T_UINT32 lastFree = StartCluster;	/* HT addition : reset LastFreeCluster */
	FF_ERROR	Error = FF_ERR_NONE;
	FF_FatBuffers FatBuf;
	FF_InitFatBuffer (&FatBuf, FF_MODE_WRITE);

	fatEntry = StartCluster;

	// Free all clusters in the chain!
	currentCluster = StartCluster;
	fatEntry = currentCluster;
	do {
		// Sector will now be fetched in write-mode
		fatEntry = FF_getFatEntry(pIoman, fatEntry, &Error, &FatBuf);
		if(FF_isERR(Error)) {
			goto out;
		}

		if(bTruncate && currentCluster == StartCluster) {
			Error = FF_putFatEntry(pIoman, currentCluster, 0xFFFFFFFF, &FatBuf);
		}else {
			Error = FF_putFatEntry(pIoman, currentCluster, 0x00000000, &FatBuf);
		}
		if(FF_isERR(Error)) {
			goto out;
		}

		if (lastFree > currentCluster) {
			lastFree = currentCluster;
		}
		currentCluster = fatEntry;
		iLen ++;

	}while(!FF_isEndOfChain(pIoman, fatEntry));

	if (pIoman->pPartition->LastFreeCluster > lastFree) {
		pIoman->pPartition->LastFreeCluster = lastFree;
	}
out:
	Error = FF_ReleaseFatBuffer(pIoman, &FatBuf);
	if(FF_isERR(Error)) {
		FF_IncreaseFreeClusters(pIoman, iLen);
		return Error;
	}

	Error = FF_IncreaseFreeClusters(pIoman, iLen);
	return Error;
}

#ifdef FF_FAT12_SUPPORT
FF_T_UINT32 FF_CountFreeClustersOLD(FF_IOMAN *pIoman, FF_ERROR *pError) {
	FF_T_UINT32 i;
	FF_T_UINT32 TotalClusters = pIoman->pPartition->DataSectors / pIoman->pPartition->SectorsPerCluster;
	FF_T_UINT32 FatEntry;
	FF_T_UINT32 FreeClusters = 0;

	*pError = FF_ERR_NONE;

	for(i = 0; i < TotalClusters; i++) {
		FatEntry = FF_getFatEntry(pIoman, i, pError, NULL);
		if(FF_isERR(*pError)) {
			return 0;
		}
		if(!FatEntry) {
			FreeClusters++;
		}
	}

	return FreeClusters;
}
#endif


FF_T_UINT32 FF_CountFreeClusters(FF_IOMAN *pIoman, FF_ERROR *pError) {
	FF_BUFFER	*pBuffer;
	FF_T_UINT32	i, x;
	FF_T_UINT32	FatEntry;
	FF_T_UINT32	EntriesPerSector;
	FF_T_UINT32	FreeClusters = 0;
	FF_T_UINT32	ClusterNum = 0;
	FF_ERROR Error;

#ifdef FF_FSINFO_TRUSTED
	FF_T_BOOL bInfoCounted = FF_FALSE;
#endif

	*pError = FF_ERR_NONE;

#ifdef FF_FAT12_SUPPORT
	if(pIoman->pPartition->Type == FF_T_FAT12) {	// FAT12 tables are too small to optimise, and would make it very complicated!
		FreeClusters = FF_CountFreeClustersOLD(pIoman, pError);
		if(FF_isERR(*pError)) {
			return 0;
		}
	}
#endif

#ifdef FF_FSINFO_TRUSTED
	if(pIoman->pPartition->Type == FF_T_FAT32) {
		pBuffer = FF_GetBuffer(pIoman, pIoman->pPartition->FSInfoLBA, FF_MODE_READ);
		{
			if(!pBuffer) {
				if(pError) {
					*pError = FF_ERR_DEVICE_DRIVER_FAILED | FF_COUNTFREECLUSTERS;
				}
				return 0;
			}

		}

		if(FF_getLong(pBuffer->pBuffer, 0) == 0x41615252 && FF_getLong(pBuffer->pBuffer, 484) == 0x61417272) {
			pIoman->pPartition->FreeClusterCount = FF_getLong(pBuffer->pBuffer, 488);
			bInfoCounted = FF_TRUE;
		}

		Error = FF_ReleaseBuffer(pIoman, pBuffer);
		if(bInfoCounted) {
			return pIoman->pPartition->FreeClusterCount;
		}
	}
#endif

	if(pIoman->pPartition->Type == FF_T_FAT32) {
		EntriesPerSector = pIoman->BlkSize / 4;
	} else {
		EntriesPerSector = pIoman->BlkSize / 2;
	}

	if (!pIoman->pPartition->BlkSize)
		return 0;  // better double-check than...

	for(i = 0; i < pIoman->pPartition->SectorsPerFAT; i++) {
		pBuffer = FF_GetBuffer(pIoman, pIoman->pPartition->FatBeginLBA + i, FF_MODE_READ);
		{
			if(!pBuffer) {
				*pError = FF_ERR_DEVICE_DRIVER_FAILED | FF_COUNTFREECLUSTERS;
				return 0;
			}
			for(x = 0; x < EntriesPerSector; x++) {
				if(pIoman->pPartition->Type == FF_T_FAT32) {
					FatEntry = FF_getLong(pBuffer->pBuffer, x * 4) & 0x0fffffff; // Clearing the top 4 bits.
				} else {
					FatEntry = (FF_T_UINT32) FF_getShort(pBuffer->pBuffer, x * 2);
				}
				if (!FatEntry) {
					FreeClusters++;
				}
				if(ClusterNum > pIoman->pPartition->NumClusters) {	// FAT table might not be cluster aligned
					break;											// Stop counting if thats the case.
				}
				ClusterNum++;
			}
		}
		Error = FF_ReleaseBuffer(pIoman, pBuffer);
		if(FF_isERR(Error)) {
			if(pError) {
				*pError = Error;
			}
			return 0;
		}
		if(ClusterNum > pIoman->pPartition->NumClusters) {
			break;													// Break out of 2nd loop too ^^
		}
	}
	// FreeClusters is -2 because the first 2 fat entries in the table are reserved.
	return FreeClusters <= pIoman->pPartition->NumClusters ? FreeClusters : pIoman->pPartition->NumClusters;
}

#ifdef FF_64_NUM_SUPPORT
FF_T_UINT64 FF_GetFreeSize(FF_IOMAN *pIoman, FF_ERROR *pError) {
	FF_T_UINT32 FreeClusters;
	FF_T_UINT64 FreeSize;
	FF_ERROR	Error;

	if(pIoman) {
		FF_lockFAT(pIoman);
		{
			if(!pIoman->pPartition->FreeClusterCount) {
				pIoman->pPartition->FreeClusterCount = FF_CountFreeClusters(pIoman, &Error);
				if(FF_isERR(Error)) {
					if(pError) {
						*pError = Error;
					}
					FF_unlockFAT(pIoman);
					return 0;
				}
			}
			FreeClusters = pIoman->pPartition->FreeClusterCount;
		}
		FF_unlockFAT(pIoman);
		FreeSize = (FF_T_UINT64) ((FF_T_UINT64)FreeClusters * (FF_T_UINT64)((FF_T_UINT64)pIoman->pPartition->SectorsPerCluster * (FF_T_UINT64)pIoman->pPartition->BlkSize));
		return FreeSize;
	}
	return 0;
}
#else
FF_T_UINT32 FF_GetFreeSize(FF_IOMAN *pIoman, FF_ERROR *pError) {
	FF_T_UINT32 FreeClusters;
	FF_T_UINT32 FreeSize;

	if(pIoman) {
		FF_lockFAT(pIoman);
		{
			if(!pIoman->pPartition->FreeClusterCount) {
				 pIoman->pPartition->FreeClusterCount = FF_CountFreeClusters(pIoman, pError);
				 if(FF_isERR(*pError)) {
					  FF_unlockFAT(pIoman);
					  return 0;
				 }
			}
			FreeClusters = pIoman->pPartition->FreeClusterCount;
		}
		FF_unlockFAT(pIoman);
		FreeSize = (FF_T_UINT32) ((FF_T_UINT32)FreeClusters * (FF_T_UINT32)((FF_T_UINT32)pIoman->pPartition->SectorsPerCluster * (FF_T_UINT32)pIoman->pPartition->BlkSize));
		return FreeSize;
	}
	return 0;
}
#endif
