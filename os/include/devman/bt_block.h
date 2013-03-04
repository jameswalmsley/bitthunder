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



#endif
