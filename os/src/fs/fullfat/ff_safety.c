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
 *	@file		ff_safety.c
 *	@author		James Walmsley
 *	@ingroup	SAFETY
 *
 *	@defgroup	SAFETY	Process Safety for FullFAT
 *	@brief		Provides semaphores, and thread-safety for FullFAT.
 *
 *	This module aims to be as portable as possible. It is necessary to modify
 *	the functions FF_CreateSemaphore, FF_PendSemaphore, FF_ReleaseSemaphore,
 *  and FF_DestroySemaphore, as appropriate for your platform.
 *
 *	If your application has no OS and is therefore single threaded, simply
 *	have:
 *
 *	FF_CreateSemaphore() return NULL.
 *
 *	FF_PendSemaphore() should do nothing.
 *
 *	FF_ReleaseSemaphore() should do nothing.
 *
 *	FF_DestroySemaphore() should do nothing.
 *
 **/

#include "ff_safety.h"	// Íncludes ff_types.h

void *FF_CreateSemaphore(void) {
	// Call your OS's CreateSemaphore function
	//

	// return pointer to semaphore
	return NULL;	// Comment this out for your implementation.
}

void FF_PendSemaphore(void *pSemaphore) {
	// Call your OS's PendSemaphore with the provided pSemaphore pointer.
	//
	// This should block indefinitely until the Semaphore
	// becomes available. (No timeout!)
	// If your OS doesn't do it for you, you should sleep
	// this thread until the Semaphore is available.
	pSemaphore = 0;
}

void FF_ReleaseSemaphore(void *pSemaphore) {
	// Call your OS's ReleaseSemaphore with the provided pSemaphore pointer.
	//

	//
	pSemaphore = 0;
}

void FF_DestroySemaphore(void *pSemaphore) {
	// Call your OS's DestroySemaphore with the provided pSemaphore pointer.
	//

	//
	pSemaphore = 0;
}

void FF_Yield(void) {
	// Call your OS's thread Yield function.
	// If this doesn't work, then a deadlock will occur
}

void FF_Sleep(FF_T_UINT32 TimeMs) {
	// Call your OS's thread sleep function,
	// Sleep for TimeMs milliseconds
	TimeMs = 0;
}


/**
 *	Notes on implementation.
 *
 *
 *
 **/


