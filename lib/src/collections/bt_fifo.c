/**
 *	BitThunder FIFO structure.
 *
 **/

#include <bitthunder.h>
#include <collections/bt_fifo.h>
#include <string.h>

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME						("FIFO")
BT_DEF_MODULE_DESCRIPTION				("Simple fifo for bitthunder")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")


/**
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;			///< All handles must include a handle header.
	BT_HANDLE		 oQueue;						///< Pointer to the start of the ring-buffer.
	BT_u32	 		 ulElements;
	BT_u32	 		 ulElementWidth;
	BT_u32	 		 ulFlags;
};

static const BT_IF_HANDLE oHandleInterface;

static BT_BOOL isFifoHandle(BT_HANDLE hFifo) {
	if(!hFifo || (hFifo->h.pIf->eType != BT_HANDLE_T_FIFO)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}


BT_HANDLE BT_FifoCreate(BT_u32 ulElements, BT_u32 ulElementWidth, BT_u32 ulFlags, BT_ERROR *pError) {

	BT_HANDLE hFifo;
	BT_ERROR Error;

	hFifo = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);

	if(!hFifo) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hFifo->oQueue   = BT_CreateQueue(ulElements, ulElementWidth, &Error);
	// Check BT_CreateQueue succeeded!
	if (!hFifo->oQueue) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}
	hFifo->ulElementWidth = ulElementWidth;
	hFifo->ulElements     = ulElements;
	hFifo->ulFlags        = ulFlags;

	return hFifo;

	err_free_out:
		BT_DestroyHandle(hFifo);

	err_out:
		if (pError) *pError = Error;
		return NULL;
}

BT_u32 BT_FifoWrite(BT_HANDLE hFifo, BT_u32 ulElements, const void *pData, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	const BT_u8 *pSrc = pData;
	BT_u32 ulWritten;
	BT_BOOL bHigherPriorityTaskWoken;

	for (ulWritten = 0; ulWritten < ulElements; ulWritten++) {
		// We should prevent overflow, and block!
		if (hFifo->ulFlags & BT_FIFO_NONBLOCKING) {
			if (BT_FifoIsFull(hFifo->oQueue,pError))
				return ulWritten;
		}
		if (BT_GetActiveInterrupt(pError) >= 0)
			BT_QueueSendFromISR(hFifo->oQueue, pSrc, &bHigherPriorityTaskWoken);
		else
			BT_QueueSend(hFifo->oQueue, pSrc, -1);
		pSrc += hFifo->ulElementWidth;
	}

	return ulWritten;
}

BT_u32 BT_FifoWriteFromISR(BT_HANDLE hFifo, BT_u32 ulElements, const void *pData, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	const BT_u8 *pSrc = pData;
	BT_u32 ulWritten;
	BT_BOOL bHigherPriorityTaskWoken;

	for (ulWritten = 0; ulWritten < ulElements; ulWritten++) {
		// We should prevent overflow, and block!
		if (hFifo->ulFlags & BT_FIFO_NONBLOCKING) {
			if (BT_FifoIsFull(hFifo->oQueue,pError))
				return ulWritten;
		}

		BT_QueueSendFromISR(hFifo->oQueue, pSrc, &bHigherPriorityTaskWoken);
		pSrc += hFifo->ulElementWidth;
	}

	return ulWritten;
}

BT_u32 BT_FifoRead(BT_HANDLE hFifo, BT_u32 ulElements, void *pData, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	BT_u8 *pSrc = pData;
	BT_u32 ulRead;
	BT_BOOL bHigherPriorityTaskWoken;

	// Get bytes from RX buffer very quickly.
	for (ulRead = 0; ulRead < ulElements; ulRead++) {
		if(BT_FifoIsEmpty(hFifo, pError)) {
			if (hFifo->ulFlags & BT_FIFO_NONBLOCKING)
				return ulRead;
		}
		if (BT_GetActiveInterrupt(pError) >= 0)
			BT_QueueReceiveFromISR(hFifo->oQueue, pData, &bHigherPriorityTaskWoken);
		else
			BT_QueueReceive(hFifo->oQueue, pData, -1);
		pSrc += hFifo->ulElementWidth;
	}
	return ulRead;
}

BT_u32 BT_FifoReadFromISR(BT_HANDLE hFifo, BT_u32 ulElements, void *pData, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	BT_u8 *pSrc = pData;
	BT_u32 ulRead;
	BT_BOOL bHigherPriorityTaskWoken;

	// Get bytes from RX buffer very quickly.
	for (ulRead = 0; ulRead < ulElements; ulRead++) {
		if(BT_FifoIsEmpty(hFifo, pError)) {
			if (hFifo->ulFlags & BT_FIFO_NONBLOCKING)
				return ulRead;
		}
		BT_QueueReceiveFromISR(hFifo->oQueue, pData, &bHigherPriorityTaskWoken);
		pSrc += hFifo->ulElementWidth;
	}
	return ulRead;
}

BT_BOOL BT_FifoIsEmpty(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	return (BT_QueueMessagesWaiting(hFifo->oQueue) == 0);
}

BT_BOOL BT_FifoIsFull(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	return (hFifo->ulElements == BT_QueueMessagesWaiting(hFifo->oQueue));
}


BT_u32 BT_FifoFillLevel(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		*pError = BT_ERR_INVALID_HANDLE_TYPE;
		return 0;
	}
	if (pError) *pError = BT_ERR_NONE;

	return BT_QueueMessagesWaiting(hFifo->oQueue);
}

BT_u32 BT_FifoSize(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		*pError = BT_ERR_INVALID_HANDLE_TYPE;
		return 0;
	}
	if (pError) *pError = BT_ERR_NONE;


	return hFifo->ulElements;
}


BT_ERROR fifo_cleanup(BT_HANDLE hFifo) {
	BT_CloseHandle(hFifo->oQueue);

	return BT_ERR_NONE;
}


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType		= BT_HANDLE_T_FIFO,											///< Handle Type!
	.pfnCleanup	= fifo_cleanup,												///< Handle's cleanup routine.
};
