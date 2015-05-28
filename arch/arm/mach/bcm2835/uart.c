#include <bitthunder.h>
#include "aux_reg.h"

BT_DEF_MODULE_NAME				("BCM2835-UART")
BT_DEF_MODULE_DESCRIPTION		("Simple Uart device for the BCM2835")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
    BT_HANDLE_HEADER        h;
    volatile BCM2835_AUX_REGS       *pRegs;
    const BT_INTEGRATED_DEVICE *pDevice;
    BT_UART_OPERATING_MODE  eMode;
};

static BT_s32 uart_write(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer) {
    BT_u32 ulWritten = 0;
    BT_u8 *pucSource = (BT_u8 *) pBuffer;
    while(ulSize) {
        while(!(hUart->pRegs->AUX_MU_LSR & 0x20)) {
            BT_ThreadYield();
        }
        hUart->pRegs->AUX_MU_IO = *pucSource++;
        ulSize--;
        ulWritten += 1;
    }

    return ulWritten;
}

static BT_ERROR uart_set_config(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {
    return BT_ERR_NONE;
}

static BT_ERROR uart_enable(BT_HANDLE hUart) {
    return BT_ERR_NONE;
}

static const BT_DEV_IF_UART oUartConfigInterface = {
	//.pfnSetBaudrate = uart_set_baudrate,						///< UART setBaudrate implementation.
	.pfnSetConfig 	= uart_set_config,							///< UART set config imple.
	//.pfnGetConfig 	= uart_get_config,
	.pfnEnable 		= uart_enable,								///< Enable/disable the device.
	//.pfnDisable 	= uart_disable,
	//.pfnGetAvailable = uart_get_available,
	//.pfnTxBufferClear = uart_tx_buffer_clear,
};

static const BT_IF_POWER oPowerInterface = {
	//.pfnSetPowerState = uart_set_power_state,					///< Pointers to the power state API implementations.
	//.pfnGetPowerState = uart_get_power_state,					///< This gets the current power state.
};

static const BT_DEV_IFS oConfigInterface = {
	(BT_DEV_INTERFACE) &oUartConfigInterface,
};

static const BT_IF_DEVICE oDeviceInterface = {
	//.pPowerIF = &oPowerInterface,											///< Device does not support powerstate functionality.
    .eConfigType = BT_DEV_IF_T_UART,											///< Allow configuration through the UART api.
	.unConfigIfs = {
		.pUartIF = &oUartConfigInterface,
	},
};

static const BT_IF_FILE oFileInterface = {
	//.pfnRead = uart_read,
	.pfnWrite = uart_write,
	//.pfnFlush = uart_flush,
	.ulSupported = BT_FILE_NON_BLOCK,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &oDeviceInterface,
	},
	.pFileIF = &oFileInterface,
	.eType = BT_HANDLE_T_DEVICE,								///< Handle Type!
//	.pfnCleanup = uart_cleanup,									///< Handle's cleanup routine.
};

static BT_HANDLE uart_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {
    BT_ERROR Error = BT_ERR_NONE;
    BT_HANDLE hUart;

    hUart = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
    if(!hUart) {
        Error = BT_ERR_NO_MEMORY;
        goto err_out;
    }

    hUart->pDevice = pDevice;

    const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
    if(!pResource) {
        Error = BT_ERR_GENERIC;
        goto err_free_out;
    }

    hUart->pRegs = (BCM2835_AUX_REGS *) bt_ioremap((void *) pResource->ulStart, BT_SIZE_4K);
    hUart->pRegs->AUX_ENABLES |= BCM2835_AUX_ENABLES_MU;
    hUart->pRegs->AUX_MU_IER = 0;
    hUart->pRegs->AUX_MU_CNTL = 0;
    hUart->pRegs->AUX_MU_LCR = 3;
    hUart->pRegs->AUX_MU_MCR = 0;
    hUart->pRegs->AUX_MU_IIR = 0xc6;
    hUart->pRegs->AUX_MU_BAUD = 270;

    hUart->pRegs->AUX_MU_CNTL = 2;

    return hUart;

err_free_out:
    BT_DestroyHandle(hUart);

err_out:
    if(pError) {
        *pError = Error;
    }
}

BT_INTEGRATED_DRIVER_DEF bc2835_uart_driver = {
    .name   = "bcm2835,aux,uart",
    .eType  = BT_DRIVER_INTEGRATED | BT_DRIVER_DEVFS_PROBE,
    .pfnProbe = uart_probe,
};
