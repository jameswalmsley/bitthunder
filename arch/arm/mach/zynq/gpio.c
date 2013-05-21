/**
 *	Zynq GPIO driver.
 *
 **/
#include <bitthunder.h>
#include "gpio.h"

BT_DEF_MODULE_NAME				("Zynq GPIO")
BT_DEF_MODULE_DESCRIPTION		("Provides abstract control of GPIO signals for Zynq platform.")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	volatile ZYNQ_GPIO_REGS 	   *pRegs;
};

static BT_ERROR gpio_irq_handler(BT_u32 ulIRQ, void *pParam) {
	BT_HANDLE hGPIO = (BT_HANDLE) pParam;

	hGPIO->pRegs->bank[0].INT_STAT = 1;		///< Acknowledge the interrupt.

	hGPIO->pRegs->DATA[0] ^= 1 << 12;

	return BT_ERR_NONE;
}

static BT_ERROR gpio_cleanup(BT_HANDLE hGPIO) {
	return BT_ERR_NONE;
}

static BT_ERROR gpio_set(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_BOOL bValue) {
	BT_u32 ulBank 	= ulGPIO / 16;
	BT_u32 ulBit	= ulGPIO % 16;

	/*
	 *	Create a mask to control the bit.
	 */
	BT_u32 ulMask 	= (bValue << ulBit) | ((~1) << (ulBit + 16));

	/*while(1) {
		hGPIO->pRegs->MASK_DATA[ulBank] = ulMask;
		ulMask ^= (1 << ulBit);
	}*/

	hGPIO->pRegs->MASK_DATA[ulBank] = ulMask;

	return BT_ERR_NONE;
}

static BT_BOOL gpio_get(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError) {
	if(pError) {
		*pError = BT_ERR_NONE;
	}
	return BT_FALSE;
}

static BT_ERROR gpio_set_direction(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_GPIO_DIRECTION eDirection) {
	BT_u32 ulBank 	= ulGPIO / 32;
	BT_u32 ulBit	= ulGPIO % 32;


	switch(eDirection) {
	case BT_GPIO_DIR_OUTPUT:
		hGPIO->pRegs->bank[ulBank].DIRM 	|= 1 << ulBit;
		hGPIO->pRegs->bank[ulBank].OEN  	|= 1 << ulBit;
		break;

	case BT_GPIO_DIR_INPUT:
		hGPIO->pRegs->bank[ulBank].OEN 		&= ~(1 << ulBit);
		hGPIO->pRegs->bank[ulBank].DIRM 	&= ~(1 << ulBit);
		break;

	default:
		return BT_ERR_GENERIC;

	}

	return BT_ERR_NONE;
}

static BT_GPIO_DIRECTION gpio_get_direction(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError) {
	if(pError) {
		*pError = BT_ERR_NONE;
	}
	return BT_GPIO_DIR_UNKNOWN;
}

static BT_ERROR gpio_enable_interrupt(BT_HANDLE hGPIO, BT_u32 ulGPIO) {
	BT_u32 ulBank = ulGPIO / 32;
	BT_u32 ulBit  = ulGPIO % 32;

	hGPIO->pRegs->bank[ulBank].INT_EN = (1 << ulBit);

	return BT_ERR_NONE;
}

static BT_ERROR gpio_disable_interrupt(BT_HANDLE hGPIO, BT_u32 ulGPIO) {
	BT_u32 ulBank = ulGPIO / 32;
	BT_u32 ulBit  = ulGPIO % 32;

	hGPIO->pRegs->bank[ulBank].INT_DIS = (1 << ulBit);

	return BT_ERR_NONE;
}

static const BT_DEV_IF_GPIO gpio_ops = {
	.pfnSet					= gpio_set,
	.pfnGet					= gpio_get,
	.pfnSetDirection		= gpio_set_direction,
	.pfnGetDirection		= gpio_get_direction,
	.pfnEnableInterrupt		= gpio_enable_interrupt,
	.pfnDisableInterrupt 	= gpio_disable_interrupt,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_GPIO,
	.unConfigIfs = {
		.pGpioIF = &gpio_ops,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = gpio_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

static BT_HANDLE gpio_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_u32 i;

	BT_ERROR Error;
	if(pError) {
		*pError = BT_ERR_NONE;
	}

	BT_HANDLE hGPIO = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hGPIO) {
		goto err_out;
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IO, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	BT_u32 base 	= pResource->ulStart;
	BT_u32 total 	= (pResource->ulEnd - pResource->ulStart) + 1;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hGPIO->pRegs = (ZYNQ_GPIO_REGS *) pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	Error = BT_RegisterInterrupt(pResource->ulStart, gpio_irq_handler, hGPIO);
	if(Error) {
		goto err_free_out;
	}

	// Prevent Pending Interrupts
	for(i = 0; i < total; i += 32) {
		hGPIO->pRegs->bank[i/32].INT_DIS = 0xFFFFFFFF;
	}

	Error = BT_EnableInterrupt(pResource->ulStart);

	Error = BT_RegisterGpioController(base, total, hGPIO);
	if(Error) {
		goto err_free_irq;
	}

	return hGPIO;

err_free_irq:
	BT_UnregisterInterrupt(pResource->ulStart, gpio_irq_handler, hGPIO);

err_free_out:
	BT_DestroyHandle(hGPIO);

	if(pError) {
		*pError = Error;
	}

err_out:
	return NULL;
}


BT_INTEGRATED_DRIVER_DEF gpio_driver = {
	.name 		= "zynq,gpio",
	.pfnProbe	= gpio_probe,
};
