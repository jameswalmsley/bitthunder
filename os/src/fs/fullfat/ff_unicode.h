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
 *	@file		ff_unicode.c
 *	@author		James Walmsley
 *	@ingroup	UNICODE
 *
 **/

#ifndef _FF_UNICODE_H_
#define _FF_UNICODE_H_

#include "ff_config.h"
#include "ff_types.h"
#include "ff_error.h"

// UTF8 / UTF16 Transformation Functions

FF_T_UINT FF_GetUtf16SequenceLen	(FF_T_UINT16 usLeadChar);
FF_T_SINT32 FF_Utf8ctoUtf16c		(FF_T_UINT16 *utf16Dest, const FF_T_UINT8 *utf8Source, FF_T_UINT32 ulSize);
FF_T_SINT32 FF_Utf16ctoUtf8c		(FF_T_UINT8 *utf8Dest, const FF_T_UINT16 *utf16Source, FF_T_UINT32 ulSize);

// UTF16 / UTF32 Transformation Functions

FF_T_SINT32 FF_Utf16ctoUtf32c(FF_T_UINT32 *utf32Dest, const FF_T_UINT16 *utf16Source);
FF_T_SINT32 FF_Utf32ctoUtf16c(FF_T_UINT16 *utf16Dest, FF_T_UINT32 utf32char, FF_T_UINT32 ulSize);

// String transformations
FF_T_SINT32 FF_Utf32stoUtf8s(FF_T_UINT8 *Utf8String, FF_T_UINT32 *Utf32String);

#endif
