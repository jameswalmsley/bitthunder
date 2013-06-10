#include <bitthunder.h>
#include <collections/bt_fifo.h>
#include "i2c.h"
#include "slcr.h"

BT_DEF_MODULE_NAME			("Zynq-I2C")
BT_DEF_MODULE_DESCRIPTION	("I2C device driver for Zynq")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	ZYNQ_I2C_REGS 	   		   *pRegs;
	const BT_INTEGRATED_DEVICE *pDevice;
};

static BT_ERROR i2c_irq_handler(BT_u32 ulIRQ, void *pParam) {
	return BT_ERR_NONE;
}

static BT_ERROR i2c_cleanup(BT_HANDLE hI2C) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hI2C->pDevice, BT_RESOURCE_IRQ, 0);

	BT_ERROR Error = BT_UnregisterInterrupt(pResource->ulStart, i2c_irq_handler, hI2C);

	return Error;
}

static BT_ERROR i2c_set_clock_rate(BT_HANDLE hI2C, BT_I2C_CLOCKRATE eClockRate) {

	BT_u32 clk = BT_ZYNQ_GetCpu1xFrequency();
	BT_u32 target;

	switch(eClockRate) {
	case BT_I2C_CLOCKRATE_100kHz:
		target = 100000;
		break;

	case BT_I2C_CLOCKRATE_400kHz:
		target = 400000;
		break;

	case BT_I2C_CLOCKRATE_1000kHz:	///< Unsupported Freq!
	case BT_I2C_CLOCKRATE_3400kHz:	///< Unsupported Freq!
	default:
		return BT_ERR_GENERIC;
		break;
	}

	BT_DIVIDER_PARAMS oDiv;
	oDiv.diva_max = 4;
	oDiv.diva_min = 0;
	oDiv.divb_max = 64;
	oDiv.divb_min = 0;

	BT_CalculateClockDivider(clk/22, target, &oDiv);

	CONTROL_DIV_A_SET(hI2C->pRegs->CONTROL, (oDiv.diva_val-1));
	CONTROL_DIV_B_SET(hI2C->pRegs->CONTROL, (oDiv.divb_val-1));

	BT_kPrint("Found a divider: a=%d, b=%d, clkout= %d Hz", oDiv.diva_val, oDiv.divb_val, oDiv.clk_out);

	return BT_ERR_NONE;
}

static BT_u32 i2c_master_transfer(BT_HANDLE hI2C, BT_I2C_MESSAGE *msgs, BT_u32 num, BT_ERROR *pError) {
	return 0;
}

static const BT_DEV_IF_I2C oI2CInterface = {
	.ulFunctionality	= BT_I2C_FUNC_I2C | BT_I2C_FUNC_10BIT_ADDR,
	.pfnMasterTransfer	= i2c_master_transfer,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.pPowerIF = NULL,
	.eConfigType = BT_DEV_IF_T_I2C,
	.unConfigIfs = {
		.pI2CIF = &oI2CInterface,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
	.eType = BT_HANDLE_T_DEVICE,
	.pfnCleanup = i2c_cleanup,
};

static BT_HANDLE i2c_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hI2C = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_out;
	}

	hI2C = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hI2C) {
		goto err_out;
	}

	hI2C->pRegs = (ZYNQ_I2C_REGS *) pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_INVALID_RESOURCE;
		goto err_free_out;
	}

	BT_u32 ulIRQ = pResource->ulStart;

	Error = BT_RegisterInterrupt(ulIRQ, i2c_irq_handler, hI2C);
	if(Error) {
		Error = BT_ERR_GENERIC;	// Device already in use!
		goto err_free_out;
	}

	hI2C->pDevice = pDevice;

	i2c_set_clock_rate(hI2C, BT_I2C_CLOCKRATE_400kHz);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_BUSID, 0);
	if(!pResource) {
		goto err_free_int_out;
	}

	BT_u32 ulBusID = pResource->ulStart;

	BT_I2C_RegisterBusWithID(hI2C, ulBusID);
	if(pError) {
		*pError = Error;
	}

	return hI2C;

err_free_int_out:
	BT_UnregisterInterrupt(ulIRQ, i2c_irq_handler, hI2C);

err_free_out:
	BT_DestroyHandle(hI2C);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oDriver = {
	.name 		= "zynq,i2c",
	.pfnProbe 	= i2c_probe,
};
