/**
 *	FIFO structure definition.
 **/

#ifndef _BT_FIFO_H_
#define _BT_FIFO_H_

#include <bt_types.h>


#define	BT_FIFO_NONBLOCKING		0x00000001
#define	BT_FIFO_OVERWRITE		0x00000002



BT_HANDLE BT_FifoCreate(BT_u32 ulElements, BT_u32 ulElementWidth, BT_u32 ulFlags, BT_ERROR *pError);
BT_u32 BT_FifoWrite(BT_HANDLE hFifo, BT_u32 ulElements, const void *pData, BT_ERROR *pError);
BT_u32 BT_FifoRead(BT_HANDLE hFifo, BT_u32 ulElements, void * pData, BT_ERROR *pError);
BT_BOOL BT_FifoIsEmpty(BT_HANDLE hFifo, BT_ERROR *pError);
BT_BOOL BT_FifoIsFull(BT_HANDLE hFifo, BT_ERROR *pError);
BT_ERROR BT_FifoWaitFull(BT_HANDLE hFifo);
BT_ERROR BT_FifoWaitEmpty(BT_HANDLE hFifo);
BT_u32 BT_FifoFillLevel(BT_HANDLE hFifo, BT_ERROR *pError);
BT_u32 BT_FifoSize(BT_HANDLE hFifo, BT_ERROR *pError);


#endif
