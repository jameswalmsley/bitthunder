/**
 *	DAC Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isDacHandle(BT_HANDLE hDac) {
	if(!hDac || !BT_IF_DEVICE(hDac) || (BT_IF_DEVICE_TYPE(hDac) != BT_DEV_IF_T_DAC)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}

BT_ERROR BT_DacStart(BT_HANDLE hDac) {
	if(!isDacHandle(hDac)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_DAC_OPS(hDac)->pfnStart(hDac);
}

BT_ERROR BT_DacStop(BT_HANDLE hDac) {
	if(!isDacHandle(hDac)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_DAC_OPS(hDac)->pfnStop(hDac);
}

BT_HANDLE BT_DacRegisterCallback(BT_HANDLE hDac, BT_DAC_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	if(!isDacHandle(hDac)) {
		// ERR_INVALID_HANDLE_TYPE
		return NULL;
	}

	return BT_IF_DAC_OPS(hDac)->pfnRegisterCallback(hDac, pfnCallback, pParam, pError);
}

BT_ERROR BT_DacUnregisterCallback(BT_HANDLE hDac, BT_HANDLE hCallback) {
	if(!isDacHandle(hDac)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_DAC_OPS(hDac)->pfnUnregisterCallback(hDac, hCallback);
}


BT_ERROR BT_DacWrite(BT_HANDLE hDac, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pSrc) {
	if(!isDacHandle(hDac)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_DAC_OPS(hDac)->pfnWrite(hDac, ulChannel, ulSize, pSrc);
}

/**
 *	@brief	Set a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_DacSetConfiguration(BT_HANDLE hDac, BT_DAC_CONFIG *pConfig) {
	if(!isDacHandle(hDac)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_DAC_OPS(hDac)->pfnSetConfig(hDac, pConfig);
}

/**
 *	@brief	Get a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_DacGetConfiguration(BT_HANDLE hDac, BT_DAC_CONFIG *pConfig) {
	if(!isDacHandle(hDac)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_DAC_OPS(hDac)->pfnGetConfig(hDac, pConfig);
}
