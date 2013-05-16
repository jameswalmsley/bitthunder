/**
 *	ADC Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


static BT_BOOL isAdcHandle(BT_HANDLE hAdc) {
	if(!hAdc || !BT_IF_DEVICE(hAdc) || (BT_IF_DEVICE_TYPE(hAdc) != BT_DEV_IF_T_ADC)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}


BT_ERROR BT_AdcStart(BT_HANDLE hAdc) {
	if(!isAdcHandle(hAdc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnStart(hAdc);
}

BT_ERROR BT_AdcStop(BT_HANDLE hAdc) {
	if(!isAdcHandle(hAdc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnStop(hAdc);
}

BT_HANDLE BT_AdcRegisterCallback(BT_HANDLE hAdc, BT_ADC_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	if(!isAdcHandle(hAdc)) {
		// ERR_INVALID_HANDLE_TYPE
		return NULL;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnRegisterCallback(hAdc, pfnCallback, pParam, pError);
}

BT_ERROR BT_AdcUnregisterCallback(BT_HANDLE hAdc, BT_HANDLE hCallback) {
	if(!isAdcHandle(hAdc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnUnregisterCallback(hAdc, hCallback);
}

BT_ERROR BT_AdcRead(BT_HANDLE hAdc, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pucDest) {
	if(!isAdcHandle(hAdc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnRead(hAdc, ulChannel, ulSize, pucDest);
}

/**
 *	@brief	Set a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_AdcSetConfiguration(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {
	if(!isAdcHandle(hAdc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_ADC_OPS(hAdc)->pfnSetConfig(hAdc, pConfig);
}

/**
 *	@brief	Get a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_AdcGetConfiguration(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {
	if(!isAdcHandle(hAdc)) {
		// ERR_INVALID_HANDLE_TYPE
		return (BT_ERROR) -1;
	}
	return BT_IF_ADC_OPS(hAdc)->pfnGetConfig(hAdc, pConfig);
}
