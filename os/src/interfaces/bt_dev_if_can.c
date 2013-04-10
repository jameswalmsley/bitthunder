/**
 *	CAN Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isCANHandle(BT_HANDLE hCAN) {
	if(!hCAN || !BT_IF_DEVICE(hCAN) || (BT_IF_DEVICE_TYPE(hCAN) != BT_DEV_IF_T_CAN)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

/**
 *	@brief	Set the Baudrate of the CAN device specified by the provided BT_HANDLE.
 *
 **/
BT_ERROR BT_CANSetBaudrate(BT_HANDLE hCAN, BT_u32 ulBaudrate) {
	if(!isCANHandle(hCAN)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	// If we did kernel mode switching we'd do it here, but for now its now supported.

	return BT_IF_CAN_OPS(hCAN)->pfnSetBaudrate(hCAN, ulBaudrate);
}


/**
 *	@brief	Set a Complete CAN configuration for the CAN device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_CANSetConfiguration(BT_HANDLE hCAN, BT_CAN_CONFIG *pConfig) {
	if(!isCANHandle(hCAN)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_CAN_OPS(hCAN)->pfnSetConfig(hCAN, pConfig);
}


/**
 *	@brief	Get a Complete CAN configuration for the CAN device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_CANGetConfiguration(BT_HANDLE hCAN, BT_CAN_CONFIG *pConfig) {
	if(!isCANHandle(hCAN)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_CAN_OPS(hCAN)->pfnGetConfig(hCAN, pConfig);
}

BT_ERROR BT_CANEnable(BT_HANDLE hCAN) {
	if(!isCANHandle(hCAN)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_CAN_OPS(hCAN)->pfnEnable(hCAN);
}

BT_ERROR BT_CANDisable(BT_HANDLE hCAN) {
	if(!isCANHandle(hCAN)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_CAN_OPS(hCAN)->pfnDisable(hCAN);
}

BT_ERROR BT_CanSendMsg(BT_HANDLE hCAN, BT_CAN_MESSAGE *pCanMessage) {
	if(!isCANHandle(hCAN)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_CAN_OPS(hCAN)->pfnSendMessage(hCAN, pCanMessage);
}

BT_ERROR BT_CanReadMsg(BT_HANDLE hCAN, BT_CAN_MESSAGE *pCanMessage) {
	if(!isCANHandle(hCAN)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_CAN_OPS(hCAN)->pfnReadMessage(hCAN, pCanMessage);
}
