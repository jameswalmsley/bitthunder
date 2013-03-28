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
 *	@file		ff_fat.h
 *	@author		James Walmsley
 *	@ingroup	FAT
 **/

#ifndef _FF_FAT_H_
#define _FF_FAT_H_

#include "ff_config.h"
#include "ff_fatdef.h"
#include "ff_ioman.h"
#include "ff_blk.h"
#include "ff_types.h"

//---------- ERROR CODES


//---------- PROTOTYPES

// HT statistics Will be taken away after testing:

struct SFatStat {
	unsigned initCount;
	unsigned clearCount;
	unsigned getCount[2];  // 0 = read, 1 = write
	unsigned reuseCount[2];  // 0 = read, 1 = write
	unsigned missCount[2];  // 0 = read, 1 = write
};

extern struct SFatStat fatStat;

#if defined(FF_WRITE_BOTH_FATS) //  || defined(FF_FAT12_SUPPORT)
#define BUF_STORE_COUNT 2
#else
#define BUF_STORE_COUNT 1
#endif

typedef struct _FatBuffers {
	FF_BUFFER *pBuffers[BUF_STORE_COUNT];
	FF_T_UINT8 Mode; // FF_MODE_READ or WRITE
} FF_FatBuffers;

		FF_T_UINT32 FF_getRealLBA			(FF_IOMAN *pIoman, FF_T_UINT32 LBA);
		FF_T_UINT32 FF_Cluster2LBA			(FF_IOMAN *pIoman, FF_T_UINT32 Cluster);
		FF_T_UINT32 FF_LBA2Cluster			(FF_IOMAN *pIoman, FF_T_UINT32 Address);
		FF_T_UINT32 FF_getFatEntry			(FF_IOMAN *pIoman, FF_T_UINT32 nCluster, FF_ERROR *pError, FF_FatBuffers *pFatBuf);
		FF_ERROR	FF_putFatEntry			(FF_IOMAN *pIoman, FF_T_UINT32 nCluster, FF_T_UINT32 Value, FF_FatBuffers *pFatBuf);
		FF_T_BOOL	FF_isEndOfChain			(FF_IOMAN *pIoman, FF_T_UINT32 fatEntry);
		FF_T_UINT32 FF_FindFreeCluster		(FF_IOMAN *pIoman, FF_ERROR *pError);
		FF_T_UINT32	FF_ExtendClusterChain	(FF_IOMAN *pIoman, FF_T_UINT32 StartCluster, FF_T_UINT32 Count);
		FF_ERROR	FF_UnlinkClusterChain	(FF_IOMAN *pIoman, FF_T_UINT32 StartCluster, FF_T_BOOL bTruncate);
		FF_T_UINT32	FF_TraverseFAT			(FF_IOMAN *pIoman, FF_T_UINT32 Start, FF_T_UINT32 Count, FF_ERROR *pError);
		FF_T_UINT32 FF_CreateClusterChain	(FF_IOMAN *pIoman, FF_ERROR *pError);
		FF_T_UINT32 FF_GetChainLength		(FF_IOMAN *pIoman, FF_T_UINT32 pa_nStartCluster, FF_T_UINT32 *piEndOfChain, FF_ERROR *pError);
		FF_T_UINT32 FF_FindEndOfChain		(FF_IOMAN *pIoman, FF_T_UINT32 Start, FF_ERROR *pError);
		FF_ERROR	FF_ClearCluster			(FF_IOMAN *pIoman, FF_T_UINT32 nCluster);
#ifdef FF_64_NUM_SUPPORT
		FF_T_UINT64 FF_GetFreeSize			(FF_IOMAN *pIoman, FF_ERROR *pError);
#else
		FF_T_UINT32 FF_GetFreeSize			(FF_IOMAN *pIoman, FF_ERROR *pError);
#endif
		FF_T_UINT32 FF_CountFreeClusters	(FF_IOMAN *pIoman, FF_ERROR *pError);	// WARNING: If this protoype changes, it must be updated in ff_ioman.c also!
		void		FF_lockFAT				(FF_IOMAN *pIoman);
		void		FF_unlockFAT			(FF_IOMAN *pIoman);

FF_T_UINT32 FF_FindFreeCluster(FF_IOMAN *pIoman, FF_ERROR *pError);

FF_ERROR FF_ReleaseFatBuffer (FF_IOMAN *pIoman, FF_FatBuffers *pFatBuf);

FF_INLINE void FF_InitFatBuffer (FF_FatBuffers *pBuffer, unsigned aMode)
{
	pBuffer->pBuffers[0] = NULL;
#if BUF_STORE_COUNT > 1
	pBuffer->pBuffers[1] = NULL;
#endif
#if BUF_STORE_COUNT > 2
#error Please check this code, maybe it is time to use memset
#endif
	pBuffer->Mode = aMode; // FF_MODE_READ/WRITE
	fatStat.initCount++;
}

#endif

