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

/**
 *	@file		ff_format.c
 *	@ingroup	FORMAT
 *
 **/


#ifndef _FF_FORMAT_H_
#define _FF_FORMAT_H_

#ifdef	__cplusplus
extern "C" {
#endif


#ifndef PLUS_FAT_H
	#error this header will be included from "plusfat.h"
#endif

/*---------- PROTOTYPES */
/* PUBLIC (Interfaces): */

typedef enum _FF_SizeType {
	eSizeIsQuota,    /* Assign a quotum (sum of xSizes is free, all disk space will be allocated) */
	eSizeIsPercent,  /* Assign a percentage of the available space (sum of xSizes must be <= 100) */
	eSizeIsSectors,  /* Assign fixed number of sectors (sum of xSizes must be < ulSectorCount) */
} eSizeType_t;

typedef struct _FF_PartitionParameters {
	uint32_t ulSectorCount;     /* Total number of sectors on the disk, including hidden/reserved */
								/* Must be obtained from the block driver */
	uint32_t ulHiddenSectors;   /* Keep at least these initial sectors free  */
	uint32_t ulInterSpace;      /* Number of sectors to keep free between partitions (when 0 -> 2048) */
	BaseType_t xSizes[ ffconfigMAX_PARTITIONS ];  /* E.g. 80, 20, 0, 0 (see eSizeType) */
    BaseType_t xPrimaryCount;    /* The number of partitions that must be "primary" */
	eSizeType_t eSizeType;
} FF_PartitionParameters_t;

FF_Error_t FF_Partition( FF_Disk_t *pxDisk, FF_PartitionParameters_t *pParams );

FF_Error_t FF_Format( FF_Disk_t *pxDisk, BaseType_t xPartitionNumber, BaseType_t xPreferFAT16, BaseType_t xSmallClusters );
FF_Error_t FF_FormatRegion( FF_Disk_t *pxDisk, BaseType_t xPreferFAT16, BaseType_t xSmallClusters, uint32_t ulStartSector, uint32_t ulSectorCount);
/* Private : */

#ifdef	__cplusplus
} /* extern "C" */
#endif

#endif
