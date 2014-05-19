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
	if(!hUart || !BT_IF_DEVICE(hUart) || (BT_IF_DEVICE_TYPE(hUart) != BT_DEV_IF_T_UART)) {
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

	return BT_IF_UART_OPS(hUart)->pfnSetBaudrate(hUart, ulBaudrate);
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
	return BT_IF_UART_OPS(hUart)->pfnSetConfig(hUart, pConfig);
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
	return BT_IF_UART_OPS(hUart)->pfnGetConfig(hUart, pConfig);
}

BT_ERROR BT_UartEnable(BT_HANDLE hUart) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_UART_OPS(hUart)->pfnEnable(hUart);
}

BT_ERROR BT_UartDisable(BT_HANDLE hUart) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_UART_OPS(hUart)->pfnDisable(hUart);
}

BT_ERROR BT_UartGetAvailable(BT_HANDLE hUart, BT_u32 *pTransmit, BT_u32 *pReceive) {
	if(!isUartHandle(hUart)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_UART_OPS(hUart)->pfnGetAvailable(hUart, pTransmit, pReceive);
}
