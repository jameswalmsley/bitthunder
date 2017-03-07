#include <bitthunder.h>
//#include "gpio.h"

BT_DEF_MODULE_NAME              ("IMX GPIO")
BT_DEF_MODULE_DESCRIPTION       ("Driver for controlling IMX GPIOs.");
BT_DEF_MODULE_AUTHOR            ("James Walmsley")
BT_DEF_MODULE_EMAIL             ("james@fullfat-fs.co.uk")

typedef struct _IMX_GPIO_REGS {
    BT_u32  DR;
    BT_u32  DIR;
    BT_u32  PSR;
    BT_u32  ICR[2];
    BT_u32  IMR;
    BT_u32  ISR;
    BT_u32  EDGE_SEL;

    BT_STRUCT_RESERVED_u32(0, 0x1C, 0x4000);

} IMX_GPIO_REGS;

struct _BT_OPAQUE_HANDLE {
    BT_HANDLE_HEADER    h;
    volatile IMX_GPIO_REGS  *pbank;
};

static BT_ERROR imx_gpio_set(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_BOOL bValue) {

    BT_u32 port = ulGPIO / 32;
    BT_u32 bit = ulGPIO % 32;

    if(bValue) {
        hGPIO->pbank[port].DR |= (1 << bit);
    } else {
        hGPIO->pbank[port].DR &= ~(1 << bit);
    }

    return BT_ERR_NONE;
}

static BT_BOOL imx_gpio_get(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError) {
    return BT_ERR_NONE;
}

static BT_ERROR imx_gpio_set_direction(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_GPIO_DIRECTION eDirection) {

    BT_u32 port = ulGPIO / 32;
    BT_u32 bit = ulGPIO % 32;

    if(eDirection == BT_GPIO_DIR_OUTPUT) {
        hGPIO->pbank[port].DIR |= (1 << bit);
    } else {
        hGPIO->pbank[port].DIR &= ~(1 << bit);
    }

    return BT_ERR_NONE;
}

static BT_GPIO_DIRECTION imx_gpio_get_direction(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError) {
    return BT_ERR_NONE;
}

static BT_ERROR imx_gpio_enable_interrupt(BT_HANDLE hGPIO, BT_u32 ulGPIO) {
    return BT_ERR_NONE;
}

static BT_ERROR imx_gpio_disable_interrupt(BT_HANDLE hGPIO, BT_u32 ulGPIO) {
    return BT_ERR_NONE;
}

static const BT_DEV_IF_GPIO gpio_ops = {
	.pfnSet					= imx_gpio_set,
	.pfnGet					= imx_gpio_get,
	.pfnSetDirection		= imx_gpio_set_direction,
	.pfnGetDirection		= imx_gpio_get_direction,
	.pfnEnableInterrupt		= imx_gpio_enable_interrupt,
	.pfnDisableInterrupt 	= imx_gpio_disable_interrupt,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_GPIO,
	.unConfigIfs = {
		.pGpioIF = &gpio_ops,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = NULL,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

static BT_HANDLE imx_gpio_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError)
{
    BT_HANDLE hGPIO = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
    if(!hGPIO) {
        goto err_out;
    }

    hGPIO->pbank = (volatile IMX_GPIO_REGS *) bt_ioremap(0x0209c000, 0x1000*8);


    BT_RegisterGpioController(0, 32*8, hGPIO);

    return hGPIO;

err_out:
    return NULL;
}


BT_INTEGRATED_DRIVER_DEF imx_gpio_driver = {
    .name = "imx,gpio",
    .pfnProbe = imx_gpio_probe,
};
