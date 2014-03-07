/**
 *	QEI Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isQEIHandle(BT_HANDLE hQEI) {
	if(!hQEI || !BT_IF_DEVICE(hQEI) || (BT_IF_DEVICE_TYPE(hQEI) != BT_DEV_IF_T_QEI)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}


BT_u32 BT_QEIGetIndexCount(BT_HANDLE hQEI, BT_ERROR *pError) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnGetIndexCount(hQEI, pError);
}
BT_EXPORT_SYMBOL(BT_QEIGetIndexCount);

BT_u32 BT_QEIGetPosition(BT_HANDLE hQEI, BT_ERROR *pError) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnGetPosition(hQEI, pError);
}
BT_EXPORT_SYMBOL(BT_QEIGetPosition);

BT_ERROR BT_QEISetMaximumPosition(BT_HANDLE hQEI, BT_u32 ulValue) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnSetMaximumPosition(hQEI, ulValue);
}
BT_EXPORT_SYMBOL(BT_QUISetMaximumPosition);

BT_ERROR BT_QEISetPositionComparator(BT_HANDLE hQEI, BT_u32 ulChannel, BT_u32 ulValue) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnSetPositionComparator(hQEI, ulChannel, ulValue);
}
BT_EXPORT_SYMBOL(BT_QEISetPositionComparator);

BT_s32 BT_QEIGetVelocity(BT_HANDLE hQEI, BT_ERROR *pError) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnGetVelocity(hQEI, pError);
}
BT_EXPORT_SYMBOL(BT_QEIGetVelocity);

/**
 *	@brief	Set a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_QEISetConfiguration(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_QEI_OPS(hQEI)->pfnSetConfig(hQEI, pConfig);
}
BT_EXPORT_SYMBOL(BT_QEISetConfiguration);

/**
 *	@brief	Get a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_QEIGetConfiguration(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_QEI_OPS(hQEI)->pfnGetConfig(hQEI, pConfig);
}
BT_EXPORT_SYMBOL(BT_QEIGetConfiguration);

BT_ERROR BT_QEIEnableInterrupt(BT_HANDLE hQEI, BT_u32 ulType) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnEnableInterrupt(hQEI, ulType);
}
BT_EXPORT_SYMBOL(BT_QEIEnableInterrupt);

BT_ERROR BT_QEIDisableInterrupt(BT_HANDLE hQEI, BT_u32 ulType) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnDisableInterrupt(hQEI, ulType);
}
BT_EXPORT_SYMBOL(BT_QEIDisableInterrupt);

BT_ERROR BT_QEIClearInterrupt(BT_HANDLE hQEI, BT_u32 ulType) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnClearInterrupt(hQEI, ulType);
}
BT_EXPORT_SYMBOL(BT_QEIClearInterrupt);

BT_HANDLE BT_QEIRegisterCallback(BT_HANDLE hQEI, BT_QEI_CALLBACK pfnCallback, void *pParam, BT_u32 ulInterruptID, BT_ERROR *pError) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return NULL;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnRegisterCallback(hQEI, pfnCallback, pParam, ulInterruptID, pError);
}
BT_EXPORT_SYMBOL(BT_QEIRegisterCallback);

BT_ERROR BT_QEIUnregisterCallback(BT_HANDLE hQEI, BT_HANDLE hCallback) {
	if(!isQEIHandle(hQEI)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_QEI_OPS(hQEI)->pfnUnregisterCallback(hQEI, hCallback);
}
BT_EXPORT_SYMBOL(BT_QEIUnregisterCallback);
