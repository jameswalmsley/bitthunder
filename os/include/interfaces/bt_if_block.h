#ifndef _BT_IF_BLOCK_H_
#define _BT_IF_BLOCK_H_

/**
 *	@brief Defines the interfaces for block device I/O.
 *
 *	A device may implement the Read/WriteBlocks	functions, or the Request function.
 *	For simple devices, where Block read/write order does not affect the performance
 *	then simply implement Read/Write block functions.
 *
 *	For devices supporting the Request function, the block queue manager will generate
 *	block transaction requests, and the device can complete these in its own time
 *	and in any order.
 *
 *	@pfnReadBlocks	[OPTIONAL]	Reads the specified blocks from the block device.
 *	@pfnWriteBlocks	[OPTIONAL]	Writes the specified blocks from the block device.
 *	@pfnRequest		[OPTIONAL]	Implements a request queue processor callback function.
 *
 **/
typedef struct _BT_IF_BLOCK {
	BT_s32		(*pfnReadBlocks)	(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer);
	BT_s32		(*pfnWriteBlocks)	(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer);
} BT_IF_BLOCK;


#endif
