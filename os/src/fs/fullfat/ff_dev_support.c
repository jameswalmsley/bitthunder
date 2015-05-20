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
#include "ff_devices.h"

#include "logbuf.h"

#ifndef ARRAY_SIZE
#	define	ARRAY_SIZE( x )	( int )( sizeof( x ) / sizeof( x )[ 0 ] )
#endif

#if( ffconfigDEV_SUPPORT == 0 )
	#error No use to include this module if ffconfigDEV_SUPPORT is disabled
#endif /* ffconfigDEV_SUPPORT == 0 */

struct SFileCache
{
	char pcFileName[16];
	uint32_t ulFileLength;
	uint32_t ulFilePointer;
};

struct SFileCache xFiles[ 16 ];

enum eCACHE_ACTION
{
	eCACHE_LOOKUP,
	eCACHE_ADD,
	eCACHE_REMOVE,
};

const char pcDevicePath[] = ffconfigDEV_PATH;

struct SFileCache *pxFindFile( const char *pcFname, enum eCACHE_ACTION eAction )
{
BaseType_t xIndex, xFreeIndex = -1;
struct SFileCache *pxResult = NULL;

	for( xIndex = 0; xIndex < ARRAY_SIZE( xFiles ); xIndex++ )
	{
		if( xFiles[ xIndex ].pcFileName[ 0 ] == '\0' )
		{
			if( xFreeIndex < 0 )
			{
				xFreeIndex = xIndex;
			}
		}
		else if( strcmp( xFiles[ xIndex ].pcFileName, pcFname ) == 0 )
		{
			if( eAction == eCACHE_REMOVE )
			{
				xFiles[ xIndex ].pcFileName[ 0 ] = '\0';
			}

			pxResult = xFiles + xIndex;
			break;
		}
	}

	if( ( pxResult == NULL ) && ( eAction == eCACHE_ADD ) && ( xFreeIndex >= 0 ) )
	{
		pxResult = xFiles + xFreeIndex;
		snprintf( pxResult->pcFileName, sizeof pxResult->pcFileName, "%s", pcFname );
		pxResult->ulFileLength = 0;
		pxResult->ulFilePointer = 0;
	}

	return pxResult;
}

BaseType_t xCheckDevicePath( const char *pcPath )
{
BaseType_t xDevLength;
BaseType_t xPathLength;
BaseType_t xIsDevice;

	xDevLength = sizeof( pcDevicePath ) - 1;
	xPathLength = strlen( pcPath );

	/* System "/dev" should not match with "/device/etc". */
	if( ( xPathLength >= xDevLength ) &&
		( memcmp( pcDevicePath, pcPath, xDevLength ) == 0 ) &&
		( ( pcPath[ xDevLength ] == '\0' ) || ( pcPath[ xDevLength ] == '/' ) ) )
	{
		xIsDevice = FF_DEV_CHAR_DEV;
	}
	else
	{
		xIsDevice = FF_DEV_NO_DEV;
	}

	return xIsDevice;
}

BaseType_t FF_Device_Open( const char *pcPath, FF_FILE *pxStream )
{
uint8_t ucIsDevice;

	ucIsDevice = xCheckDevicePath( pcPath );
	if( ucIsDevice != pdFALSE )
	{
	const char *pcBaseName = pcPath;

		if( memcmp( pcBaseName, pcDevicePath, sizeof pcDevicePath - 1 ) == 0 )
		{
			pcBaseName = pcBaseName + sizeof pcDevicePath;
		}

		pxStream->pxDevNode = pxFindFile( pcBaseName, eCACHE_ADD );
		if( pxStream->pxDevNode != NULL )
		{
			pxStream->pxDevNode->ulFilePointer = 0;
			if( ( pxStream->ucMode & ( FF_MODE_WRITE | FF_MODE_APPEND | FF_MODE_CREATE ) ) == 0 )
			{
				pxStream->ulFileSize = pxStream->pxDevNode->ulFileLength;
			}
		}
	}

	return ucIsDevice;
}

void FF_Device_Close( FF_FILE * pxStream )
{
	if( pxStream->pxDevNode != NULL )
	{
		pxStream->ulFileSize = 0ul;
		pxStream->ulFilePointer = 0ul;
	}
}

size_t FF_Device_Read( void *pvBuf, size_t lSize, size_t lCount, FF_FILE * pxStream )
{
	lCount *= lSize;
	return lCount;
}

size_t FF_Device_Write( const void *pvBuf, size_t lSize, size_t lCount, FF_FILE * pxStream )
{
	lCount *= lSize;

	if( pxStream->pxDevNode != NULL )
	{

		pxStream->pxDevNode->ulFilePointer += lCount;
		if( pxStream->pxDevNode->ulFileLength < pxStream->pxDevNode->ulFilePointer )
		{
			pxStream->pxDevNode->ulFileLength = pxStream->pxDevNode->ulFilePointer;
		}
	}
	return lCount;
}

int FF_Device_Seek( FF_FILE *pxStream, long lOffset, int iWhence )
{
	if( pxStream->pxDevNode != NULL )
	{
		if( iWhence == FF_SEEK_SET )
		{
			pxStream->pxDevNode->ulFilePointer = lOffset;
		}
		else if( iWhence == FF_SEEK_END )
		{
			pxStream->pxDevNode->ulFilePointer = pxStream->pxDevNode->ulFileLength - lOffset;
		}
	}

	return 0;
}

int FF_Device_GetDirEnt( const char *pcPath, FF_DirEnt_t *pxDirEnt )
{
	if( pxDirEnt->pcFileName[ 0 ] != '.' )
	{
	struct SFileCache *pxDevNode;

		pxDevNode = pxFindFile( pxDirEnt->pcFileName, eCACHE_LOOKUP );

		pxDirEnt->ucIsDeviceDir = FF_DEV_CHAR_DEV;
		if( pxDevNode != NULL )
		{
			pxDirEnt->ulFileSize = pxDevNode->ulFileLength;
		}
		else if( pxDirEnt->ulFileSize < 2048 )
		{
			pxDirEnt->ulFileSize = 2048;
		}
	}

	return 1024;
}
