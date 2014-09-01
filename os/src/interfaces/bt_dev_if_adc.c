/**
 *	ADC Configuration API.
 *
 *
 **/
#include <bitthunder.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_LIST_HEAD(g_adc_devices);
static BT_u32 g_total_adcs = 0;

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {
	BT_ADC_INFO *pInfo = (BT_ADC_INFO *) bt_container_of(node, BT_ADC_INFO, node);
	if(!pInfo->ulReferenceCount) {
		pInfo->ulReferenceCount += 1;
		//BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &pInfo->hAdc);
		return (BT_HANDLE) pInfo->hAdc;
	}

	return NULL;
}

static const BT_DEVFS_OPS adc_devfs_ops = {
	.pfnOpen = devfs_open,
};


static BT_BOOL isAdcHandle(BT_HANDLE hAdc) {
	if(!hAdc || !BT_IF_DEVICE(hAdc) || (BT_IF_DEVICE_TYPE(hAdc) != BT_DEV_IF_T_ADC)) {
		return BT_FALSE;
	}
	return BT_TRUE;
}


BT_ERROR BT_AdcStart(BT_HANDLE hAdc) {
	if(!isAdcHandle(hAdc)) {
		return BT_ERR_INVALID_HANDLE_TYPE;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnStart(hAdc);
}
BT_EXPORT_SYMBOL(BT_AdcStart);

BT_ERROR BT_AdcStop(BT_HANDLE hAdc) {
	if(!isAdcHandle(hAdc)) {
		return BT_ERR_INVALID_HANDLE_TYPE;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnStop(hAdc);
}
BT_EXPORT_SYMBOL(BT_AdcStop);

BT_HANDLE BT_AdcRegisterCallback(BT_HANDLE hAdc, BT_ADC_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	if(!isAdcHandle(hAdc)) {
		if (pError) {
			*pError = BT_ERR_INVALID_HANDLE_TYPE;
		}
		return NULL;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnRegisterCallback(hAdc, pfnCallback, pParam, pError);
}
BT_EXPORT_SYMBOL(BT_AdcRegisterCallback);

BT_ERROR BT_AdcUnregisterCallback(BT_HANDLE hAdc, BT_HANDLE hCallback) {
	if(!isAdcHandle(hAdc)) {
		return BT_ERR_INVALID_HANDLE_TYPE;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnUnregisterCallback(hAdc, hCallback);
}
BT_EXPORT_SYMBOL(BT_AdcUnregisterCallback);

BT_s32 BT_AdcRead(BT_HANDLE hAdc, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pucDest) {
	if(!isAdcHandle(hAdc)) {
		return BT_ERR_INVALID_HANDLE_TYPE;
	}

	return BT_IF_ADC_OPS(hAdc)->pfnRead(hAdc, ulChannel, ulSize, pucDest);
}
BT_EXPORT_SYMBOL(BT_AdcRead);

/**
 *	@brief	Set a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_AdcSetConfiguration(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {
	if(!isAdcHandle(hAdc)) {
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	return BT_IF_ADC_OPS(hAdc)->pfnSetConfig(hAdc, pConfig);
}
BT_EXPORT_SYMBOL(BT_AdcSetConfiguration);

/**
 *	@brief	Get a Complete TIMER configuration for the TIMER device specified by the BT_HANDLE.
 *
 **/
BT_ERROR BT_AdcGetConfiguration(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig) {
	if(!isAdcHandle(hAdc)) {
		return BT_ERR_INVALID_HANDLE_TYPE;
	}
	return BT_IF_ADC_OPS(hAdc)->pfnGetConfig(hAdc, pConfig);
}
BT_EXPORT_SYMBOL(BT_AdcGetConfiguration);

BT_ERROR BT_AdcRegisterDevice(BT_HANDLE hDevice, BT_ADC_INFO *adc) {

	bt_list_add(&adc->item, &g_adc_devices);
	adc->node.pOps = &adc_devfs_ops;
	adc->hAdc = hDevice;

	char name[10];

	const BT_RESOURCE *pResource = BT_GetDeviceResource(adc->pDevice, BT_RESOURCE_STRING, 0);
	if(!pResource) {
		bt_sprintf(name, "adc%d", g_total_adcs);
	}
	else {
		strncpy(name,  pResource->szpName, 10);
	}
	g_total_adcs++;

	BT_kPrint("Registering %s as /dev/%s", adc->pDevice->name, name);

	return BT_DeviceRegister(&adc->node, name);
}
BT_EXPORT_SYMBOL(BT_DACRegisterDevice);
