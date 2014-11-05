/**
 *	Provides the LPC17xx dac for BitThunder.
 **/

#include <bitthunder.h>
#include "dac.h"
#include "rcc.h"
#include <collections/bt_fifo.h>


BT_DEF_MODULE_NAME			("LPC17xx-DAC")
BT_DEF_MODULE_DESCRIPTION	("LPC17xx dac kernel driver")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a DAC driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;				///< All handles must include a handle header.
	LPC17xx_DAC_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_DAC_OPERATING_MODE	eMode;			///< Operational mode, i.e. buffered/polling mode.
	BT_HANDLE		   		hFifo[1];			///< fifo - ring buffer for every data channel.
};

static BT_HANDLE g_DAC_HANDLES[1] = {
	NULL,
};

static const BT_u32 g_DAC_PERIPHERAL[1] = {11};

static void disableDacPeripheralClock(BT_HANDLE hDac);


static BT_ERROR dac_cleanup(BT_HANDLE hDac) {

	// Disable peripheral clock.
	disableDacPeripheralClock(hDac);

	// Free any buffers if used.
	BT_CloseHandle(hDac->hFifo[0]);

	/*const BT_RESOURCE *pResource = BT_GetIntegratedResource(hDac->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);*/

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hDac->pDevice, BT_RESOURCE_ENUM, 0);

	g_DAC_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

static BT_ERROR dac_setconfig(BT_HANDLE hDac, BT_DAC_CONFIG *pConfig) {
	BT_ERROR Error = BT_ERR_NONE;

	switch(pConfig->eMode) {
	case BT_DAC_MODE_POLLED: {
		if(hDac->eMode !=  BT_DAC_MODE_POLLED) {

			BT_CloseHandle(hDac->hFifo[0]);

			hDac->eMode = BT_DAC_MODE_POLLED;
		}

		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		if(hDac->eMode != BT_DAC_MODE_BUFFERED) {
			if(!hDac->hFifo[0]) {
				hDac->hFifo[0] = BT_FifoCreate(pConfig->ulBufferSize, 4, BT_FIFO_NONBLOCKING, &Error);
			}

			hDac->eMode = BT_DAC_MODE_BUFFERED;
		}
		break;
	}

	default:
		// Unsupported operating mode!
		break;
	}

	return BT_ERR_NONE;
}

static BT_ERROR dac_getconfig(BT_HANDLE hDac, BT_DAC_CONFIG *pConfig) {
	BT_ERROR Error = BT_ERR_NONE;

	pConfig->ulBufferSize		= BT_FifoSize(hDac->hFifo[0]);
	pConfig->eMode				= hDac->eMode;
	pConfig->ulUpdateInterval 	= 0;
	pConfig->ulResolution		= 10;

	return BT_ERR_NONE;
}


static BT_ERROR dac_start(BT_HANDLE hDac) {
	if (hDac->eMode == BT_DAC_MODE_BUFFERED) {
	}

	return BT_ERR_NONE;
}

static BT_ERROR dac_stop(BT_HANDLE hDac) {
	if (hDac->eMode == BT_DAC_MODE_BUFFERED) {
	}

	return BT_ERR_NONE;
}

static BT_ERROR dac_Write(BT_HANDLE hDac, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pSrc) {
	volatile LPC17xx_DAC_REGS *pRegs = hDac->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hDac->eMode) {
	case BT_DAC_MODE_POLLED:
	{
		while(ulSize--) {
			BT_u32 ulValue = (*pSrc << 6) & 0x0000FFC0;
			pRegs->DACR = ulValue;
		}

		break;
	}

	case BT_DAC_MODE_BUFFERED:
	{
		// Write bytes to TX buffer very quickly.
		BT_FifoWrite(hDac->hFifo[ulChannel], ulSize, pSrc, 0);
		break;
	}

	default:
		// ERR, invalid handle configuration.
		break;
	}
	return BT_ERR_NONE;
}

/**
 *	This actually allows the DAC'S to be clocked!
 **/
static void enableDacPeripheralClock(BT_HANDLE hDac) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hDac->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		//DAC has not control bit in PCONP
		break;
	}
	default: {
		break;
	}
	}
}

/**
 *	If the serial port is not in use, we can make it sleep!
 **/
static void disableDacPeripheralClock(BT_HANDLE hDac) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hDac->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		//DAC has not control bit in PCONP
		break;
	}
	default: {
		break;
	}
	}
}

/**
 *	Function to test the current peripheral clock gate status of the devices.
 **/
static BT_BOOL isDacPeripheralClockEnabled(BT_HANDLE hDac) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hDac->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		//DAC has not control bit in PCONP
		return BT_TRUE;
		break;
	}
	default: {
		break;
	}
	}

	return BT_FALSE;
}

/**
 *	This implements the DAC power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR DacSetPowerState(BT_HANDLE hDac, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableDacPeripheralClock(hDac);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableDacPeripheralClock(hDac);
		break;
	}

	default: {
		//return BT_ERR_POWER_STATE_UNSUPPORTED;
		return (BT_ERROR) -1;
	}
	}

	return BT_ERR_NONE;
}

/**
 *	This implements the DAC power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR DacGetPowerState(BT_HANDLE hDac, BT_POWER_STATE *pePowerState) {
	if(isDacPeripheralClockEnabled(hDac)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_DAC oDacDeviceInterface= {
	.pfnSetConfig 		= dac_setconfig,
	.pfnGetConfig		= dac_getconfig,
	.pfnStart			= dac_start,
	.pfnStop			= dac_stop,
	.pfnWrite			= dac_Write,
};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oDacDeviceInterface,
};

static const BT_IF_POWER oPowerInterface = {
	DacSetPowerState,											///< Pointers to the power state API implementations.
	DacGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LPC17xx_DAC_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_DAC,											///< Allow configuration through the DAC api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oDacDeviceInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LPC17xx_DAC_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = dac_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE dac_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hDac = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_DAC_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hDac = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hDac) {
		goto err_out;
	}

	g_DAC_HANDLES[pResource->ulStart] = hDac;

	hDac->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hDac->pRegs = (LPC17xx_DAC_REGS *) pResource->ulStart;

	DacSetPowerState(hDac, BT_POWER_STATE_AWAKE);

	/*@@pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	BT_SetInterruptPriority(pResource->ulStart, 1);*/

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, dac_irq_handler, hDac);
	if(Error) {
		goto err_free_out;
	}*/


	//@@Error = BT_EnableInterrupt(pResource->ulStart);

	return hDac;

err_free_out:
	BT_DestroyHandle(hDac);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF dac_driver = {
	.name 		= "LPC17xx,dac",
	.pfnProbe	= dac_probe,
};


#ifdef BT_CONFIG_MACH_LPC17xx_DAC_0
static const BT_RESOURCE oLPC17xx_dac0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_DAC0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_DAC0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_dac0_device = {
	.name 					= "LPC17xx,dac",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_dac0_resources),
	.pResources 			= oLPC17xx_dac0_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_dac0_inode = {
	.szpName = "dac0",
	.pDevice = &oLPC17xx_dac0_device,
};
#endif
