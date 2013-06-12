/**
 *	MAX7312 and derivatives driver.
 *
 *
 *	@author James Walmsley <james@fullfat-fs.co.uk>
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME				("Maxim 7312 GPIO Expander")
BT_DEF_MODULE_DESCRIPTION		("Maxim I2C 16-bit GPIO Port Expander")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	BT_HANDLE		 hI2C;
	BT_u32			 addr;
	BT_I2C_MESSAGE	 oMessages[2];
};

static BT_u8 gpio_read_register(BT_HANDLE hGPIO, BT_u8 reg, BT_ERROR *pError) {

	BT_ERROR Error;
	BT_u8 val = 0;

	hGPIO->oMessages[0].addr 	= hGPIO->addr;
	hGPIO->oMessages[0].len  	= 1;
	hGPIO->oMessages[0].buf  	= &reg;
	hGPIO->oMessages[0].flags   = 0;

	hGPIO->oMessages[1].addr 	= hGPIO->addr;
	hGPIO->oMessages[1].len		= 1;
	hGPIO->oMessages[1].buf		= &val;
	hGPIO->oMessages[1].flags 	= BT_I2C_M_RD;

	BT_I2C_Transfer(hGPIO->hI2C, hGPIO->oMessages, 2, &Error);

	return val;
}

static BT_ERROR gpio_write_register(BT_HANDLE hGPIO, BT_u8 reg, BT_u8 val) {

	BT_ERROR Error;

	hGPIO->oMessages[0].addr 	= hGPIO->addr;
	hGPIO->oMessages[0].len  	= 1;
	hGPIO->oMessages[0].buf  	= &reg;
	hGPIO->oMessages[0].flags   = 0;

	hGPIO->oMessages[1].addr 	= hGPIO->addr;
	hGPIO->oMessages[1].len		= 1;
	hGPIO->oMessages[1].buf		= &val;
	hGPIO->oMessages[1].flags 	= 0;

	BT_I2C_Transfer(hGPIO->hI2C, hGPIO->oMessages, 2, &Error);

	return Error;
}

static BT_u8 getreg(BT_u32 ulGPIO) {
	return (ulGPIO % 16) / 8;	// Wrap assigned IO space across all 16 GPIOs.
}

static BT_ERROR gpio_cleanup(BT_HANDLE hGPIO) {
	return BT_ERR_NONE;
}

static BT_ERROR gpio_set(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_BOOL bValue) {
	return BT_ERR_NONE;
}

static BT_BOOL gpio_get(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError) {
	BT_u8 reg = getreg(ulGPIO);
	BT_u8 bit = ulGPIO % 8;

	BT_u8 val = gpio_read_register(hGPIO, reg, pError);

	if(val & (1 << bit)) {
		return BT_TRUE;
	}

	return BT_FALSE;
}

static BT_ERROR gpio_set_direction(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_GPIO_DIRECTION eDirection) {
	return BT_ERR_NONE;
}

static BT_GPIO_DIRECTION gpio_get_direction(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError) {
	BT_u8 reg = getreg(ulGPIO);
	BT_u8 bit = ulGPIO % 8;

	BT_u8 val = gpio_read_register(hGPIO, reg, pError);

	if(val & (1 << bit)) {
		return BT_GPIO_DIR_INPUT;
	}

	return BT_GPIO_DIR_OUTPUT;
}

static BT_ERROR gpio_enable_interrupt(BT_HANDLE hGPIO, BT_u32 ulGPIO) {
	return BT_ERR_NONE;
}

static BT_ERROR gpio_disable_interrupt(BT_HANDLE hGPIO, BT_u32 ulGPIO) {
	return BT_ERR_NONE;
}

static const BT_DEV_IF_GPIO gpio_ops = {
	.pfnSet					= gpio_set,
	.pfnGet 				= gpio_get,
	.pfnSetDirection		= gpio_set_direction,
	.pfnGetDirection    	= gpio_get_direction,
	.pfnEnableInterrupt 	= gpio_enable_interrupt,
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

static BT_HANDLE gpio_probe(BT_HANDLE hI2C, const BT_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_HANDLE hGPIO = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hGPIO) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	hGPIO->hI2C = hI2C;

	const BT_RESOURCE *pResource = BT_GetDeviceResource(pDevice, BT_RESOURCE_BUSID, 1);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	hGPIO->addr = pResource->ulStart;

	pResource = BT_GetDeviceResource(pDevice, BT_RESOURCE_IO, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	BT_u32 base 	= pResource->ulStart;
	BT_u32 total 	= (pResource->ulEnd - pResource->ulStart) + 1;

	Error = BT_RegisterGpioController(base, total, hGPIO);
	if(Error) {
		goto err_free_out;
	}

	BT_kPrint("MAX7312 : Initialising GPIO Port Expander.");

	BT_u8 in1, in2;
	in1 = gpio_read_register(hGPIO, 0x00, &Error);
	in2 = gpio_read_register(hGPIO, 0x01, &Error);

	BT_kPrint("MAX7312 : in1 : %02X", in1);
	BT_kPrint("MAX7312 : in2 : %02X", in2);

	return hGPIO;

err_free_out:
	BT_DestroyHandle(hGPIO);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oDriver = {
	.name 	= "maxim,7312",
	.eType 	= BT_DEVICE_I2C,
	.pfnI2CProbe = gpio_probe,
};
