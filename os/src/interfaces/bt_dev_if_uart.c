/**
 *	UART Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isUartHandle(BT_HANDLE hUart) {
	if(!hUart || !hUart->h.pIf->oIfs.pDevIF || (hUart->h.pIf->oIfs.pDevIF->eConfigType != BT_DEV_IF_T_UART)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

/**
 *	@brief	Set the Baudrate of the UART device specified by the provided BT_HANDLE.
 *
 **/
BT_ERROR BT_UartSetBaudrate(BT_HANDLE hUart, BT_u32 ulBaudrate) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	// If we did kernel mode switching we'd do it here, but for now its now supported.

	return hUart->h.pIf->oIfs.pDevIF->unConfigIfs.pUartIF->pfnSetBaudrate(hUart, ulBaudrate);
}


/**
 *	@brief	Set a Complete UART configuration for the UART device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_UartSetConfiguration(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return hUart->h.pIf->oIfs.pDevIF->unConfigIfs.pUartIF->pfnSetConfig(hUart, pConfig);
}


/**
 *	@brief	Get a Complete UART configuration for the UART device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_UartGetConfiguration(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return hUart->h.pIf->oIfs.pDevIF->unConfigIfs.pUartIF->pfnGetConfig(hUart, pConfig);
}

BT_ERROR BT_UartEnable(BT_HANDLE hUart) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return hUart->h.pIf->oIfs.pDevIF->unConfigIfs.pUartIF->pfnEnable(hUart);
}

BT_ERROR BT_UartDisable(BT_HANDLE hUart) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return hUart->h.pIf->oIfs.pDevIF->unConfigIfs.pUartIF->pfnDisable(hUart);
}
