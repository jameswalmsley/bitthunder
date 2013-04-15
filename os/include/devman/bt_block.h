/**
 *	Block Device Manager for BitThunder
 *
 *
 **/

#ifndef _BT_BLOCK_H_
#define _BT_BLOCK_H_

typedef struct _BT_BLOCK_GEOMETRY {
	BT_u32	ulBlockSize;
	BT_u32	ulTotalBlocks;
} BT_BLOCK_GEOMETRY;

typedef struct _BT_BLKDEV_DESCRIPTOR {
	BT_u32 				ulFlags;
	BT_BLOCK_GEOMETRY 	oGeometry;
} BT_BLKDEV_DESCRIPTOR;

BT_ERROR BT_RegisterBlockDevice(BT_HANDLE hDevice, const char *szpName, BT_BLKDEV_DESCRIPTOR *pDescriptor);

BT_u32 BT_BlockRead		(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError);
BT_u32 BT_BlockWrite	(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError);
BT_ERROR BT_GetBlockGeometry(BT_HANDLE hBlock, BT_BLOCK_GEOMETRY *pGeometry);
BT_HANDLE BT_BlockGetInode(BT_HANDLE hDevice);

#endif
