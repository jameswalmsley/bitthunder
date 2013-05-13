/**
 *	I2C Configuration API.
 *
 *
 **/
#include <bitthunder.h>
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isI2CHandle(BT_HANDLE hI2C) {
	if(!hI2C || !BT_IF_DEVICE(hI2C) || (BT_IF_DEVICE_TYPE(hI2C) != BT_DEV_IF_T_I2C)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

/**
 *	@brief	Set the Baudrate of the I2C device specified by the provided BT_HANDLE.
 *
 **/
BT_ERROR BT_I2CSetClockrate(BT_HANDLE hI2C, BT_I2C_CLOCKRATE eClockrate) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	// If we did kernel mode switching we'd do it here, but for now its now supported.

	return BT_IF_I2C_OPS(hI2C)->pfnSetClockrate(hI2C, eClockrate);
}


/**
 *	@brief	Set a Complete I2C configuration for the I2C device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_I2CSetConfiguration(BT_HANDLE hI2C, BT_I2C_CONFIG *pConfig) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_I2C_OPS(hI2C)->pfnSetConfig(hI2C, pConfig);
}


/**
 *	@brief	Get a Complete I2C configuration for the I2C device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_I2CGetConfiguration(BT_HANDLE hI2C, BT_I2C_CONFIG *pConfig) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_I2C_OPS(hI2C)->pfnGetConfig(hI2C, pConfig);
}

BT_ERROR BT_I2CEnable(BT_HANDLE hI2C) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_I2C_OPS(hI2C)->pfnEnable(hI2C);
}

BT_ERROR BT_I2CDisable(BT_HANDLE hI2C) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnDisable(hI2C);
}


BT_ERROR BT_I2CStart(BT_HANDLE hI2C) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnStart(hI2C);
}

BT_ERROR BT_I2CSendAddress(BT_HANDLE hI2C, BT_u32 ulAddress, BT_I2C_ACCESS_MODE eAccessMode) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnSendAddress(hI2C, ulAddress, eAccessMode);
}

BT_ERROR BT_I2CGetData(BT_HANDLE hI2C, BT_u8 *pDest, BT_u32 ulLength) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnGetData(hI2C, pDest, ulLength);
}

BT_ERROR BT_I2CSendData(BT_HANDLE hI2C, BT_u8 *pSrc, BT_u32 ulLength) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnSendData(hI2C, pSrc, ulLength);
}

BT_ERROR BT_I2CSendNack(BT_HANDLE hI2C) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnSendNack(hI2C);
}

BT_ERROR BT_I2CSendAck(BT_HANDLE hI2C) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnSendAck(hI2C);
}

BT_BOOL  BT_I2CGetAck(BT_HANDLE hI2C, BT_ERROR *pError) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnGetAck(hI2C, pError);
}

BT_ERROR BT_I2CStop(BT_HANDLE hI2C) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnStop(hI2C);
}


BT_ERROR BT_I2CWrite(BT_HANDLE hI2C, BT_u8 ucDevice, BT_u8 *pucSource, BT_u32 ulLength) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnWrite(hI2C, ucDevice, pucSource, ulLength);
}

BT_ERROR BT_I2CRead(BT_HANDLE hI2C, BT_u8 ucDevice, BT_u8 *pucDest, BT_u32 ulLength) {
	if(!isI2CHandle(hI2C)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_I2C_OPS(hI2C)->pfnRead(hI2C, ucDevice, pucDest, ulLength);
}


