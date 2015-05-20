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

#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "portable.h"

#include "ff_headers.h"
#include "ff_sys.h"

#ifndef ARRAY_SIZE
#	define	ARRAY_SIZE(x)	(int)(sizeof(x)/sizeof(x)[0])
#endif

/*
 * Define a collection of 'file systems' as a simple array
 */
typedef struct xSYSTEM
{
	FF_SubSystem_t systems[ ffconfigMAX_FILE_SYS ];
	int fsCount;
} ff_sys_t;


static ff_sys_t file_systems;
static const char rootDir[] = "/";

int FF_FS_Count( void )
{
	return file_systems.fsCount;
}
/*-----------------------------------------------------------*/

void FF_FS_Init( void )
{
	memset( &file_systems, '\0', sizeof file_systems );

	/* There is always a root file system, even if it doesn't have a
	IO manager. */
	file_systems.fsCount = 1;
	strcpy( file_systems.systems[0].path, rootDir );
	file_systems.systems[0].pathlen = 1;
}
/*-----------------------------------------------------------*/

int FF_FS_Add( const char *pcPath, FF_Disk_t *pxDisk )
{
	int ret = 0;

	configASSERT( pxDisk );

	if (*pcPath != '/')
	{
		FF_PRINTF( "FF_FS_Add: Need a \"/\": '%s'\n", pcPath );
	}
	else
	{
		int index = -1;
		int len = ( int ) strlen (pcPath);

		if( file_systems.fsCount == 0 )
		{
			FF_FS_Init();
		}

		if( len == 1 )
		{
			/* This is the "/" path
			 * and will be put at index 0 */
			index = 0;
		}
		else
		{
			int i;
			FF_SubSystem_t *pxSubSystem = file_systems.systems + 1;	/* Skip the root entry */
			for( i = 1; i < file_systems.fsCount; i++, pxSubSystem++ )
			{
				if( pxSubSystem->pathlen == len &&
					memcmp( pxSubSystem->path, pcPath, ( size_t )len ) == 0 )
				{
					index = i;	/* A system is updated with a new handler. */
					break;
				}
			}
		}
		if( index < 0 && file_systems.fsCount >= ARRAY_SIZE( file_systems.systems ) )
		{
			FF_PRINTF( "FF_FS_Add: Table full '%s' (max = %d)\n", pcPath, (int)ARRAY_SIZE( file_systems.systems ) );
		}
		else
		{
			vTaskSuspendAll();
			{
				if( index < 0 )
				{
					index = file_systems.fsCount++;
				}

				strncpy( file_systems.systems[ index ].path, pcPath, sizeof file_systems.systems[ index ].path );
				file_systems.systems[ index ].pathlen = len;
				file_systems.systems[ index ].pxManager = pxDisk->pxIOManager;
			}
			xTaskResumeAll( );
			ret = 1;
		}
	}

	return ret;
}
/*-----------------------------------------------------------*/

void FF_FS_Remove( const char *pcPath )
{
int index;
int len;
int i;

	if( pcPath[0] == '/' )
	{
		index = -1;
		len = ( int ) strlen( pcPath );
		/* Is it the "/" path ? */
		if (len == 1)
		{
			index = 0;
		}
		else
		{
			FF_SubSystem_t *pxSubSystem = file_systems.systems + 1;
			for( i = 1; i < file_systems.fsCount; i++, pxSubSystem++ )
			{
				if( ( pxSubSystem->pathlen == len ) &&
					( memcmp( pxSubSystem->path, pcPath, ( size_t ) len ) == 0 ) )
				{
					index = i;
					break;
				}
			}
		}
		if( index >= 0 )
		{
			file_systems.systems[ index ].pxManager = NULL;
		}
	}
}
/*-----------------------------------------------------------*/

int FF_FS_Find( const char *apContext, const char *pcPath, FF_DirHandler_t *pxHandler )
{
FF_SubSystem_t *pxSubSystem;
int len;
int index;
int iReturn;

	pxSubSystem = file_systems.systems + 1;
	len = ( int ) strlen( pcPath );

	memset( pxHandler, '\0', sizeof( *pxHandler ) );
	for( index = 1; index < file_systems.fsCount; index++, pxSubSystem++ )
	{
		if( ( len >= pxSubSystem->pathlen ) &&
			( memcmp( pxSubSystem->path, pcPath, ( size_t ) pxSubSystem->pathlen ) == 0 ) &&
			( ( pcPath[pxSubSystem->pathlen] == '\0' ) || ( pcPath[pxSubSystem->pathlen] == '/') ) )	/* System "/ram" should not match with "/ramc/etc". */
		{
			if( pcPath[pxSubSystem->pathlen] == '\0')
			{
				pxHandler->path = rootDir;
			}
			else
			{
				pxHandler->path = pcPath + pxSubSystem->pathlen;
			}

			pxHandler->pxManager = pxSubSystem->pxManager;
			break;
		}
	}

	if( index == file_systems.fsCount )
	{
		pxHandler->path = pcPath;
		pxHandler->pxManager = file_systems.systems[0].pxManager;
	}

	if( FF_Mounted( pxHandler->pxManager ) )
	{
		iReturn = pdTRUE;
	}
	else
	{
		if( apContext )
			FF_PRINTF( "%s: No handler for \"%s\"\n", apContext, pcPath );
		iReturn = pdFALSE;
	}

	return iReturn;
}
/*-----------------------------------------------------------*/

int FF_FS_Get( int iIndex, FF_SubSystem_t *pxSystem )
{
int iReturn;

	/* Get a copy of a fs info. */
	if( ( iIndex < 0 ) || ( iIndex >= file_systems.fsCount ) )
	{
		iReturn = 0;
	}
	else
	{
		*pxSystem = file_systems.systems[ iIndex ];
		iReturn = 1;
	}

	return iReturn;
}
/*-----------------------------------------------------------*/

