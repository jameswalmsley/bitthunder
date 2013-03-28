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
 *	@file		ff_hash.c
 *	@author		James Walmsley
 *	@ingroup	HASH
 *
 *	@defgroup	HASH HASH Table
 *	@brief		Provides a simple HASH lookup table.
 *
 **/

#include "ff_hash.h"
#include <stdlib.h>
#include <string.h>

#ifdef FF_HASH_CACHE
struct _FF_HASH_TABLE {
	FF_T_UINT8 bitTable[FF_HASH_TABLE_SIZE];
};

/**
 *
 *
 **/
FF_HASH_TABLE FF_CreateHashTable() {
	FF_HASH_TABLE pHash = (FF_HASH_TABLE) FF_MALLOC(sizeof(struct _FF_HASH_TABLE));

	if(pHash) {
		FF_ClearHashTable(pHash);
		return pHash;
	}

	return NULL;
}

FF_ERROR FF_ClearHashTable(FF_HASH_TABLE pHash) {
	if(pHash) {
		memset(pHash->bitTable, 0, FF_HASH_TABLE_SIZE);
		return FF_ERR_NONE;
	}

	return FF_ERR_NULL_POINTER | FF_CLEARHASHTABLE;
}

FF_ERROR FF_SetHash(FF_HASH_TABLE pHash, FF_T_UINT32 nHash) {
	FF_T_UINT32 tblIndex	= ((nHash / 8) % FF_HASH_TABLE_SIZE);
	FF_T_UINT32 tblBit		= nHash % 8;

	if(pHash) {
		pHash->bitTable[tblIndex] |= (0x80 >> tblBit);
		return FF_ERR_NONE;
	}

	return FF_ERR_NULL_POINTER | FF_SETHASH;
}

FF_ERROR FF_ClearHash(FF_HASH_TABLE pHash, FF_T_UINT32 nHash) {
	FF_T_UINT32 tblIndex	= ((nHash / 8) % FF_HASH_TABLE_SIZE);
	FF_T_UINT32 tblBit		= nHash % 8;

	if(pHash) {
		pHash->bitTable[tblIndex] &= ~(0x80 >> tblBit);
		return FF_ERR_NONE;
	}

	return FF_ERR_NULL_POINTER | FF_CLEARHASH;
}

FF_T_BOOL FF_isHashSet(FF_HASH_TABLE pHash, FF_T_UINT32 nHash) {
	FF_T_UINT32 tblIndex	= ((nHash / 8) % FF_HASH_TABLE_SIZE);
	FF_T_UINT32 tblBit		= nHash % 8;

	if(pHash) {
		if(pHash->bitTable[tblIndex] & (0x80 >> tblBit)) {
			return FF_TRUE;
		}
	}
	return FF_FALSE;
}

FF_ERROR FF_DestroyHashTable(FF_HASH_TABLE pHash) {
	if(pHash) {
		FF_FREE(pHash);
		return FF_ERR_NONE;
	}
	return FF_ERR_NULL_POINTER | FF_DESTROYHASHTABLE;
}

#endif
