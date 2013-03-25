/**
 *	Block Device Manager for BitThunder
 *
 *
 **/

#ifndef _BT_BLOCK_H_
#define _BT_BLOCK_H_

typedef struct _BT_BLKDEV_DESCRIPTOR {
	BT_u32 ulFlags;
	BT_u32 ulBlockSize;
	BT_u32 ulTotalBlocks;
} BT_BLKDEV_DESCRIPTOR;

BT_ERROR BT_RegisterBlockDevice(BT_HANDLE hDevice, BT_BLKDEV_DESCRIPTOR *pDescriptor);

BT_u32 BT_BlockRead		(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError);
BT_u32 BT_BlockWrite	(BT_HANDLE hBlock, BT_u32 ulAddress, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError);

#endif
