/**
 *	SPI Configuration API.
 *
 *
 **/
#include <bitthunder.h>
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isSpiHandle(BT_HANDLE hSpi) {
	if(!hSpi || !BT_IF_DEVICE(hSpi) || (BT_IF_DEVICE_TYPE(hSpi) != BT_DEV_IF_T_SPI)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

/**
 *	@brief	Set the Baudrate of the SPI device specified by the provided BT_HANDLE.
 *
 **/
BT_ERROR BT_SpiSetBaudrate(BT_HANDLE hSpi, BT_u32 ulBaudrate) {
	if(!isSpiHandle(hSpi)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	// If we did kernel mode switching we'd do it here, but for now its now supported.

	return BT_IF_SPI_OPS(hSpi)->pfnSetBaudrate(hSpi, ulBaudrate);
}


/**
 *	@brief	Set a Complete SPI configuration for the SPI device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_SpiSetConfiguration(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig) {
	if(!isSpiHandle(hSpi)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_SPI_OPS(hSpi)->pfnSetConfig(hSpi, pConfig);
}


/**
 *	@brief	Get a Complete SPI configuration for the SPI device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_SpiGetConfiguration(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig) {
	if(!isSpiHandle(hSpi)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_SPI_OPS(hSpi)->pfnGetConfig(hSpi, pConfig);
}

BT_ERROR BT_SpiEnable(BT_HANDLE hSpi) {
	if(!isSpiHandle(hSpi)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_SPI_OPS(hSpi)->pfnEnable(hSpi);
}

BT_ERROR BT_SpiDisable(BT_HANDLE hSpi) {
	if(!isSpiHandle(hSpi)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_SPI_OPS(hSpi)->pfnDisable(hSpi);
}

BT_ERROR BT_SpiWrite(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucSource, BT_u32 ulSize) {
	if(!isSpiHandle(hSpi)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_SPI_OPS(hSpi)->pfnWrite(hSpi, ulFlags, pucSource, ulSize);
}

BT_ERROR BT_SpiRead(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucDest, BT_u32 ulSize) {
	if(!isSpiHandle(hSpi)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_SPI_OPS(hSpi)->pfnRead(hSpi, ulFlags, pucDest, ulSize);
}
