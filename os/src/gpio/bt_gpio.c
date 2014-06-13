#include <bitthunder.h>
#include <gpio/bt_gpio.h>

typedef struct _BT_GPIO_CONTROLLER {
	BT_HANDLE 	hGPIO;
	BT_u32		ulBaseGPIO;
	BT_u32		ulTotalGPIOs;
} BT_GPIO_CONTROLLER;

static BT_GPIO_CONTROLLER 	g_oControllers[BT_CONFIG_MAX_GPIO_CONTROLLERS];
static BT_u32				g_ulRegistered = 0;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_GPIO_CONTROLLER *getGpioController(BT_u32 ulGPIO) {
	BT_u32 i;

	for(i=0; i < g_ulRegistered; i++) {
		BT_u32 min, max;
		min = g_oControllers[i].ulBaseGPIO;
		max = g_oControllers[i].ulBaseGPIO + g_oControllers[i].ulTotalGPIOs;

		if(ulGPIO >= min && ulGPIO <= max) {
			return &g_oControllers[i];
		}
	}

	return NULL;
}

BT_ERROR BT_RegisterGpioController(BT_u32 ulBaseGPIO, BT_u32 ulTotalGPIOs, BT_HANDLE hGPIO) {

	if(g_ulRegistered >= BT_CONFIG_MAX_GPIO_CONTROLLERS) {
		return -1;
	}

	g_oControllers[g_ulRegistered].hGPIO 		= hGPIO;

	// Ensure there is no conflict!
	g_oControllers[g_ulRegistered].ulBaseGPIO 	= ulBaseGPIO;
	g_oControllers[g_ulRegistered].ulTotalGPIOs = ulTotalGPIOs;

	g_ulRegistered++;

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_RegisterGpioController);

BT_ERROR BT_GpioSet(BT_u32 ulGPIO, BT_BOOL bValue) {

	BT_GPIO_CONTROLLER *pGPIO = getGpioController(ulGPIO);
	if(!pGPIO) {
		return BT_ERR_GENERIC;
	}

	BT_HANDLE hGPIO = pGPIO->hGPIO;

	return BT_IF_GPIO_OPS(hGPIO)->pfnSet(hGPIO, ulGPIO - pGPIO->ulBaseGPIO, bValue);
}
BT_EXPORT_SYMBOL(BT_GpioSet);

BT_BOOL BT_GpioGet(BT_u32 ulGPIO, BT_ERROR *pError) {
	BT_GPIO_CONTROLLER *pGPIO = getGpioController(ulGPIO);
	if(!pGPIO) {
		if(pError) {
			*pError = BT_ERR_GENERIC;
		}
		return BT_FALSE;
	}

	BT_HANDLE hGPIO = pGPIO->hGPIO;
	return BT_IF_GPIO_OPS(hGPIO)->pfnGet(hGPIO, ulGPIO - pGPIO->ulBaseGPIO, pError);
}
BT_EXPORT_SYMBOL(BT_GpioGet);

BT_ERROR BT_GpioSetDirection(BT_u32 ulGPIO, BT_GPIO_DIRECTION eDirection) {
	BT_GPIO_CONTROLLER *pGPIO = getGpioController(ulGPIO);
	if(!pGPIO) {
		return BT_ERR_GENERIC;
	}

	BT_HANDLE hGPIO = pGPIO->hGPIO;
	return  BT_IF_GPIO_OPS(hGPIO)->pfnSetDirection(hGPIO, ulGPIO - pGPIO->ulBaseGPIO, eDirection);
}
BT_EXPORT_SYMBOL(BT_GpioSetDirection);

BT_GPIO_DIRECTION BT_GpioGetDirection(BT_u32 ulGPIO, BT_ERROR *pError) {
	BT_GPIO_CONTROLLER *pGPIO = getGpioController(ulGPIO);
	if(!pGPIO) {
		if(pError) {
			*pError = BT_ERR_GENERIC;
		}
		return BT_GPIO_DIR_UNKNOWN;
	}

	BT_HANDLE hGPIO = pGPIO->hGPIO;
	return BT_IF_GPIO_OPS(hGPIO)->pfnGetDirection(hGPIO, ulGPIO - pGPIO->ulBaseGPIO, pError);
}
BT_EXPORT_SYMBOL(BT_GpioGetDirection);

BT_ERROR BT_GpioEnableInterrupt(BT_u32 ulGPIO) {
	BT_GPIO_CONTROLLER *pGPIO = getGpioController(ulGPIO);
	if(!pGPIO) {
		return BT_ERR_GENERIC;
	}

	BT_HANDLE hGPIO = pGPIO->hGPIO;
	return BT_IF_GPIO_OPS(hGPIO)->pfnEnableInterrupt(hGPIO, ulGPIO - pGPIO->ulBaseGPIO);
}
BT_EXPORT_SYMBOL(BT_GpioEnableInterrupt);

BT_ERROR BT_GpioDisableInterrupt(BT_u32 ulGPIO) {
	BT_GPIO_CONTROLLER *pGPIO = getGpioController(ulGPIO);
	if(!pGPIO) {
		return BT_ERR_GENERIC;
	}

	BT_HANDLE hGPIO = pGPIO->hGPIO;
	return BT_IF_GPIO_OPS(hGPIO)->pfnDisableInterrupt(hGPIO, ulGPIO - pGPIO->ulBaseGPIO);
}
BT_EXPORT_SYMBOL(BT_GpioDisableInterrupt);
