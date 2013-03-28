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
 *	@file		ff_memory.c
 *	@author		James Walmsley
 *	@ingroup	MEMORY
 *
 *	@defgroup	MEMORY	FullFAT Memory Access Routines
 *	@brief		Handles memory access in a portable way.
 *
 *	Provides simple, fast, and portable access to memory routines.
 *	These are only used to read data from buffers. That are LITTLE ENDIAN
 *	due to the FAT specification.
 *
 *	These routines may need to be modified to your platform.
 *
 **/

#include "ff_memory.h"
#include "ff_config.h"

/*
 * HT inlined these functions
 *
 * Not much left for the C-module
 */


#ifndef FF_INLINE_MEMORY_ACCESS
FF_T_UINT8 FF_getChar(FF_T_UINT8 *pBuffer, FF_T_UINT32 aOffset) {
	return (FF_T_UINT8) (pBuffer[aOffset]);
}

FF_T_UINT16 FF_getShort(FF_T_UINT8 *pBuffer, FF_T_UINT32 aOffset) {
	FF_T_UN16 u16;
	pBuffer += aOffset;
	u16.bytes.u8_1 = pBuffer[1];
	u16.bytes.u8_0 = pBuffer[0];
	return u16.u16;
}

FF_T_UINT32 FF_getLong(FF_T_UINT8 *pBuffer, FF_T_UINT32 aOffset) {
	FF_T_UN32 u32;
	pBuffer += aOffset;
	u32.bytes.u8_3 = pBuffer[3];
	u32.bytes.u8_2 = pBuffer[2];
	u32.bytes.u8_1 = pBuffer[1];
	u32.bytes.u8_0 = pBuffer[0];
	return u32.u32;
}

void FF_putChar(FF_T_UINT8 *pBuffer, FF_T_UINT32 aOffset, FF_T_UINT8 Value) {
	pBuffer[aOffset] = Value;
}

void FF_putShort(FF_T_UINT8 *pBuffer, FF_T_UINT32 aOffset, FF_T_UINT16 Value) {
	FF_T_UN16 u16;
	u16.u16 = Value;
	pBuffer += aOffset;
	pBuffer[0] = u16.bytes.u8_0;
	pBuffer[1] = u16.bytes.u8_1;
}

void FF_putLong(FF_T_UINT8 *pBuffer, FF_T_UINT32 aOffset, FF_T_UINT32 Value) {
	FF_T_UN32 u32;
	u32.u32 = Value;
	pBuffer += aOffset;
	pBuffer[0] = u32.bytes.u8_0;
	pBuffer[1] = u32.bytes.u8_1;
	pBuffer[2] = u32.bytes.u8_2;
	pBuffer[3] = u32.bytes.u8_3;
}
#endif
