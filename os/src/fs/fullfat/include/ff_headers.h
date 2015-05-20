/*
 * FreeRTOS+FAT Labs Build 150406 (C) 2015 Real Time Engineers ltd.
 * Authors include James Walmsley, Hein Tibosch and Richard Barry
 *
 *******************************************************************************
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 ***                                                                         ***
 ***                                                                         ***
 ***   FREERTOS+FAT IS STILL IN THE LAB:                                     ***
 ***                                                                         ***
 ***   This product is functional and is already being used in commercial    ***
 ***   products.  Be aware however that we are still refining its design,    ***
 ***   the source code does not yet fully conform to the strict coding and   ***
 ***   style standards mandated by Real Time Engineers ltd., and the         ***
 ***   documentation and testing is not necessarily complete.                ***
 ***                                                                         ***
 ***   PLEASE REPORT EXPERIENCES USING THE SUPPORT RESOURCES FOUND ON THE    ***
 ***   URL: http://www.FreeRTOS.org/contact  Active early adopters may, at   ***
 ***   the sole discretion of Real Time Engineers Ltd., be offered versions  ***
 ***   under a license other than that described below.                      ***
 ***                                                                         ***
 ***                                                                         ***
 ***** NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ******* NOTE ***
 *******************************************************************************
 *
 * - Open source licensing -
 * While FreeRTOS+FAT is in the lab it is provided only under version two of the
 * GNU General Public License (GPL) (which is different to the standard FreeRTOS
 * license).  FreeRTOS+FAT is free to download, use and distribute under the
 * terms of that license provided the copyright notice and this text are not
 * altered or removed from the source files.  The GPL V2 text is available on
 * the gnu.org web site, and on the following
 * URL: http://www.FreeRTOS.org/gpl-2.0.txt.  Active early adopters may, and
 * solely at the discretion of Real Time Engineers Ltd., be offered versions
 * under a license other then the GPL.
 *
 * FreeRTOS+FAT is distributed in the hope that it will be useful.  You cannot
 * use FreeRTOS+FAT unless you agree that you use the software 'as is'.
 * FreeRTOS+FAT is provided WITHOUT ANY WARRANTY; without even the implied
 * warranties of NON-INFRINGEMENT, MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE. Real Time Engineers Ltd. disclaims all conditions and terms, be they
 * implied, expressed, or statutory.
 *
 * 1 tab == 4 spaces!
 *
 * http://www.FreeRTOS.org
 * http://www.FreeRTOS.org/plus
 * http://www.FreeRTOS.org/labs
 *
 */

#ifndef PLUS_FAT_H
#define PLUS_FAT_H

#include <string.h>

#ifdef	__cplusplus
extern "C" {
#endif

//#include "FreeRTOS.h"
//#include "task.h"
//#include "semphr.h"

#include "FreeRTOSFATConfig.h"
#include "FreeRTOSFATConfigDefaults.h"
#include "ff_error.h"
#include "ff_ioman.h"
#include "ff_fat.h"
#include "ff_fatdef.h"
#include "ff_memory.h"
#include "ff_time.h"
#include "ff_crc.h"
#include "ff_file.h"
#include "ff_dir.h"
#include "ff_string.h"
#include "ff_format.h"
#include "ff_locking.h"

/* See if any older defines with a prefix "FF_" are still defined: */
#include "ff_old_config_defines.h"

#ifndef configUSE_RECURSIVE_MUTEXES
	#error configUSE_RECURSIVE_MUTEXES must be set to 1 in FreeRTOSConfig.h
#else
	#if( configUSE_RECURSIVE_MUTEXES != 1 )
		#error configUSE_RECURSIVE_MUTEXES must be set to 1 in FreeRTOSConfig.h
	#endif
#endif /* configUSE_RECURSIVE_MUTEXES */

#ifdef	__cplusplus
}	// extern "C"
#endif

#endif
