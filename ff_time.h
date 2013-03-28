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
 *	@file		ff_time.h
 *	@author		James Walmsley
 *	@ingroup	TIME
 *
 *	Provides a means for receiving the time on any platform.
 **/

#ifndef _FF_TIME_H_
#define _FF_TIME_H_

#include "ff_config.h"
#include "ff_types.h"

/**
 *	@public
 *	@brief	A TIME and DATE object for FullFAT. A FullFAT time driver must populate these values.
 *
 **/
typedef struct {
	FF_T_UINT16	Year;		///< Year	(e.g. 2009).
	FF_T_UINT16 Month;		///< Month	(e.g. 1 = Jan, 12 = Dec).
	FF_T_UINT16	Day;		///< Day	(1 - 31).
	FF_T_UINT16 Hour;		///< Hour	(0 - 23).
	FF_T_UINT16 Minute;		///< Min	(0 - 59).
	FF_T_UINT16 Second;		///< Second	(0 - 59).
} FF_SYSTEMTIME;

//---------- PROTOTYPES

FF_T_SINT32	FF_GetSystemTime(FF_SYSTEMTIME *pTime);

#endif

