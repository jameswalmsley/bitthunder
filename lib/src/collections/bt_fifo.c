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
	BT_u8			*pBuf;						///< Pointer to the start of the ring-buffer.
	BT_u8			*pIn;						///< Input pointer.
	BT_u8			*pOut;						///< Output pointer.
	BT_u8			*pEnd;						///< Pointer to end of buffer.
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

	hFifo->pBuf 	= BT_kMalloc((ulElements+1) * ulElementWidth);
	// Check malloc succeeded!
	if (!hFifo->pBuf) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}
	hFifo->pIn 	= hFifo->pBuf;
	hFifo->pOut = hFifo->pBuf;
	hFifo->pEnd = hFifo->pBuf + (ulElements+1) * ulElementWidth;
	hFifo->ulElementWidth = ulElementWidth;
	hFifo->ulFlags = ulFlags;

	return hFifo;

	err_free_out:
		BT_DestroyHandle(hFifo);

	err_out:
		if (pError) *pError = Error;
		return NULL;
}

BT_u32 BT_FifoWrite(BT_HANDLE hFifo, BT_u32 ulElements, void * pData, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	BT_u8 *pSrc  = pData;
	BT_u8 *pDest = hFifo->pIn;
//	BT_u8 *pOut  = hFifo->pOut;
	BT_u32 ulElementWidth = hFifo->ulElementWidth;
	BT_u32 ulWritten;

	for (ulWritten = 0; ulWritten < ulElements; ulWritten++) {
		// We should prevent overflow, and block!
		if (hFifo->ulFlags & BT_FIFO_NONBLOCKING) {
			if (BT_FifoIsFull(hFifo, pError)) return ulWritten;
		}

		if (!(hFifo->ulFlags & BT_FIFO_OVERWRITE))
			BT_FifoWaitFull(hFifo);
		memcpy(pDest, pSrc, ulElementWidth);
		pDest += ulElementWidth;
		pSrc += ulElementWidth;
		if(pDest >= hFifo->pEnd) {
			pDest = hFifo->pBuf;
		}
//			if (pDest == pOut) {
//				pOut += ulElementWidth;
//				if(pOut >= hFifo->pEnd) {
//					pOut = hFifo->pBuf;
//				}
//				hFifo->pOut = pOut;
//			}
		hFifo->pIn = pDest;
	}
	return ulWritten;
}

BT_u32 BT_FifoRead(BT_HANDLE hFifo, BT_u32 ulElements, void * pData, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	BT_u8 *pSrc = hFifo->pOut;
	BT_u8 *pDest = pData;
	BT_u32 ulElementWidth = hFifo->ulElementWidth;
	BT_u32 ulRead;

	// Get bytes from RX buffer very quickly.
	for (ulRead = 0; ulRead < ulElements; ulRead++) {
		if(hFifo->pOut == hFifo->pIn) {
			if (hFifo->ulFlags & BT_FIFO_NONBLOCKING)
				return ulRead;
		}
		BT_FifoWaitEmpty(hFifo);
		if (hFifo->ulFlags & BT_FIFO_OVERWRITE) {
			pSrc = hFifo->pOut;
		}
		memcpy(pDest, pSrc, ulElementWidth);
		pDest += ulElementWidth;
		pSrc += ulElementWidth;
		if(pSrc >= hFifo->pEnd) {
			pSrc = hFifo->pBuf;
		}
		hFifo->pOut = pSrc;
	}
	return ulRead;
}

BT_BOOL BT_FifoIsEmpty(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	return (hFifo->pIn == hFifo->pOut);
}

BT_BOOL BT_FifoIsFull(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	if (pError) *pError = BT_ERR_NONE;

	BT_u8 *pWrite  = hFifo->pIn + hFifo->ulElementWidth;
	BT_u8 *pRead = hFifo->pOut;

	if(pWrite >= hFifo->pEnd) {
		pWrite = hFifo->pBuf;
	}
	return (pRead == pWrite);
}

BT_ERROR BT_FifoWaitFull(BT_HANDLE hFifo) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	BT_ERROR Error = BT_ERR_NONE;

	while (BT_FifoIsFull(hFifo, &Error))
		BT_ThreadYield();

	return Error;
}

BT_ERROR BT_FifoWaitEmpty(BT_HANDLE hFifo) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		return BT_ERR_INVALID_HANDLE_TYPE;
	}

	BT_ERROR Error = BT_ERR_NONE;

	while (BT_FifoIsEmpty(hFifo, &Error))
		BT_ThreadYield();

	return Error;
}

BT_u32 BT_FifoFillLevel(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		*pError = BT_ERR_INVALID_HANDLE_TYPE;
		return 0;
	}
	if (pError) *pError = BT_ERR_NONE;

	BT_u8 *pWrite  = hFifo->pIn;
	BT_u8 *pRead   = hFifo->pOut;
	BT_u32 ulSize  = (hFifo->pEnd - hFifo->pBuf);
	BT_u32 ulLevel;

	if (pRead <= pWrite)
		ulLevel = pWrite - pRead;
	else
		ulLevel = ulSize - (pRead - pWrite);

	return ulLevel/hFifo->ulElementWidth;
}

BT_u32 BT_FifoSize(BT_HANDLE hFifo, BT_ERROR *pError) {
	if(!isFifoHandle(hFifo)) {
		// ERR_INVALID_HANDLE_TYPE
		*pError = BT_ERR_INVALID_HANDLE_TYPE;
		return 0;
	}
	if (pError) *pError = BT_ERR_NONE;

	return (hFifo->pEnd - hFifo->pBuf) / hFifo->ulElementWidth;
}


BT_ERROR fifo_cleanup(BT_HANDLE hFifo) {
	BT_kFree(hFifo->pBuf);

	return BT_ERR_NONE;
}


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType		= BT_HANDLE_T_FIFO,											///< Handle Type!
	.pfnCleanup	= fifo_cleanup,												///< Handle's cleanup routine.
};
