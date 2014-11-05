/**
 *	FIFO structure definition.
 **/

#ifndef _BT_FIFO_H_
#define _BT_FIFO_H_

#include <bt_types.h>
#include <fs/bt_file.h>


#define	BT_FIFO_NONBLOCKING		BT_FILE_NON_BLOCK
#define	BT_FIFO_OVERWRITE		0x00000002


/**
 *	@brief	Create FIFO (circular buffer) primitive.
 *
 *	This primitive can invoke scheduler facilities to sleep while blocking,
 *	hence the existence of the WriteFromISR and ReadFromISR APIs.
 *
 *	@param[IN]			ulElements			Number of elements that the FIFO can hold.
 *	@param[IN]			ulElementWidth		Size of an element in bytes.
 *	@param[IN]			ulFlags				FIFO behaviour flags.
 *	@param[OUT, OPT]	pError				Error code in case of error, expect BT_ERR_NONE.
 *
 *	@return 	Handle to the created FIFO primitive on success.
 *	@return		NULL on error. In this case pError should contain a valid reason.
 *
 **/
BT_HANDLE BT_FifoCreate(BT_u32 ulElements, BT_u32 ulElementWidth, BT_u32 ulFlags, BT_ERROR *pError);

/**
 *	@brief	Places a number of items into the FIFO.
 *
 *	Writes ulElements elements into the FIFO specified by hFifo.
 *
 *	@param[IN]	hFifo		Handle to the FIFO to be written to.
 *	@param[IN]	ulElements	Number of elements to be written into the FIFO.
 *	@param[IN]	pData		Pointer to data to be written to the FIFO.
 *	@param[IN]	ulFlags		BT_FIFO_NONBLOCKING | BT_FILE_NON_BLOCK to prevent sleeping.
 *
 *	@return		Number of elements written to the FIFO.
 *	@return		< 0 on Error, (A BT_ERROR code).
 *
 **/
BT_s32 BT_FifoWrite(BT_HANDLE hFifo, BT_u32 ulElements, const void *pData, BT_u32 ulFlags);
BT_s32 BT_FifoWriteFromISR(BT_HANDLE hFifo, BT_u32 ulElements, const void *pData);
BT_s32 BT_FifoRead(BT_HANDLE hFifo, BT_u32 ulElements, void *pData, BT_u32 ulFlags);
BT_s32 BT_FifoReadFromISR(BT_HANDLE hFifo, BT_u32 ulElements, void *pData);
BT_BOOL BT_FifoIsEmpty(BT_HANDLE hFifo, BT_ERROR *pError);
BT_BOOL BT_FifoIsFull(BT_HANDLE hFifo, BT_ERROR *pError);
BT_s32 BT_FifoFillLevel(BT_HANDLE hFifo);
BT_s32 BT_FifoGetAvailable(BT_HANDLE hFifo);
BT_s32 BT_FifoSize(BT_HANDLE hFifo);


#endif
