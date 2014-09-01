/**
 *	Provides the LM3Sxx  mcpwm for BitThunder.
 **/

#include <bitthunder.h>
#include "mcpwm.h"
#include "rcc.h"


BT_DEF_MODULE_NAME			("LM3Sxx-MCPWM")
BT_DEF_MODULE_DESCRIPTION	("LM3Sxx mcpwm kernel driver")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")


struct _MCPWM_CALLBACK_HANDLE {
	BT_HANDLE_HEADER	h;
	BT_MCPWM_CALLBACK	pfnCallback;
	void 			   *pParam;
} ;


/**
 *	We can define how a handle should look in a MCPWM driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 				h;			///< All handles must include a handle header.
	LM3Sxx_MCPWM_REGS			   *pRegs;
	const BT_INTEGRATED_DEVICE	   *pDevice;
	BT_u32							id;
	struct _MCPWM_CALLBACK_HANDLE  *pCallback;
};

static BT_HANDLE g_MCPWM_HANDLES[1] = {
	NULL,
};

static void disableMCPWMPeripheralClock(BT_HANDLE hMCPWM);

static const BT_IF_HANDLE oCallbackHandleInterface;


BT_ERROR BT_NVIC_IRQ_25(void) {
	volatile BT_HANDLE hMCPWM = g_MCPWM_HANDLES[0];
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	// Clear the position interrupt

	if (hMCPWM->pCallback)
		hMCPWM->pCallback->pfnCallback(hMCPWM, hMCPWM->pCallback->pParam);

	return 0;
}

static void ResetMCPWM(BT_HANDLE hMCPWM) {
}

static BT_ERROR mcpwm_cleanup(BT_HANDLE hMCPWM) {
	ResetMCPWM(hMCPWM);

	// Disable peripheral clock.
	disableMCPWMPeripheralClock(hMCPWM);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMCPWM->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hMCPWM->pDevice, BT_RESOURCE_ENUM, 0);

	g_MCPWM_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_enable_interrupt(BT_HANDLE hMCPWM) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_disable_interrupt(BT_HANDLE hMCPWM) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_callback_cleanup(BT_HANDLE hCallback) {
	struct _MCPWM_CALLBACK_HANDLE *hCleanup = (struct _MCPWM_CALLBACK_HANDLE*)hCallback;

	hCleanup->pfnCallback = NULL;
	hCleanup->pParam	  = NULL;

	BT_CloseHandle((BT_HANDLE)hCleanup);

	return BT_ERR_NONE;
}

static BT_HANDLE mcpwm_register_callback(BT_HANDLE hMCPWM, BT_MCPWM_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	struct _MCPWM_CALLBACK_HANDLE *pCallback = (struct _MCPWM_CALLBACK_HANDLE*)BT_CreateHandle(&oCallbackHandleInterface, sizeof(struct _MCPWM_CALLBACK_HANDLE),pError);

	if (pCallback) {
		pCallback->pfnCallback = pfnCallback;
		pCallback->pParam	   = pParam;

		hMCPWM->pCallback      = pCallback;
	}

	return (BT_HANDLE)pCallback;
}

static BT_ERROR mcpwm_unregister_callback(BT_HANDLE hMCPWM, BT_HANDLE hCallback) {
	mcpwm_callback_cleanup(hCallback);

	hMCPWM->pCallback = NULL;

	return BT_ERR_NONE;
}


static BT_ERROR mcpwm_set_channelconfig(BT_HANDLE hMCPWM, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	BT_u32 ulInputClk = BT_LM3Sxx_GetMainFrequency();

	pRegs->PWMBlocks[ulChannel].LOAD = (ulInputClk / pConfig->ulFrequency)-1;
	pRegs->PWMBlocks[ulChannel].CMPA = pConfig->ulPulsewidth;

	if (pConfig->eType == BT_MCPWM_CHANNEL_MODE_CENTER_ALIGNED) {
		pRegs->PWMBlocks[ulChannel].CTL |= 0x00000002;
		pRegs->PWMBlocks[ulChannel].GENA = 0x000000D0;
		pRegs->PWMBlocks[ulChannel].GENB = 0x000000B0;
	}
	else {
		pRegs->PWMBlocks[ulChannel].CTL &= ~0x00000002;
		pRegs->PWMBlocks[ulChannel].GENA = 0x0000008C;
		pRegs->PWMBlocks[ulChannel].GENB = 0x000000C8;
	}

	if (pConfig->bDeadtimeEnable){
		pRegs->PWMBlocks[ulChannel].DBRISE = pConfig->ulDeadtime;
		pRegs->PWMBlocks[ulChannel].DBFALL = pConfig->ulDeadtime;
		pRegs->PWMBlocks[ulChannel].DBCTL = 0x00000001;
	}
	else {
		pRegs->PWMBlocks[ulChannel].DBCTL = 0x00000000;
	}

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_get_channelconfig(BT_HANDLE hMCPWM, BT_u32 ulChannel, BT_MCPWM_CHANNEL_CONFIG *pConfig) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	BT_u32 ulInputClk = BT_LM3Sxx_GetMainFrequency();

	pConfig->ulFrequency	= ulInputClk/(pRegs->PWMBlocks[ulChannel].LOAD);
	pConfig->ulPulsewidth	= pRegs->PWMBlocks[ulChannel].CMPA;

	if (pRegs->PWMBlocks[ulChannel].CTL & 0x00000002)
		pConfig->eType = BT_MCPWM_CHANNEL_MODE_CENTER_ALIGNED;
	else
		pConfig->eType = BT_MCPWM_CHANNEL_MODE_EDGE_ALIGNED;

	if (pRegs->PWMBlocks[ulChannel].DBCTL & 0x00000001) {
		pConfig->bDeadtimeEnable = BT_TRUE;
		pConfig->ulDeadtime = pRegs->PWMBlocks[ulChannel].DBRISE;
	}
	else
		pConfig->bDeadtimeEnable = BT_FALSE;

	return BT_ERR_NONE;
}

static BT_u32 mcpwm_get_channelpulsewidth(BT_HANDLE hMCPWM, BT_u32 ulChannel, BT_ERROR *pError) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	if (pError)
		*pError = BT_ERR_NONE;

	return pRegs->PWMBlocks[ulChannel].CMPA;
}

static BT_ERROR mcpwm_set_channelpulsewidth(BT_HANDLE hMCPWM, BT_u32 ulChannel, BT_u32 ulValue) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	pRegs->PWMBlocks[ulChannel].CMPA = ulValue;

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_set_DCModePattern(BT_HANDLE hMCPWM, BT_MCPWM_DCMODE_PATTERN ulChannel0, BT_MCPWM_DCMODE_PATTERN ulChannel1, BT_MCPWM_DCMODE_PATTERN ulChannel2) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_start(BT_HANDLE hMCPWM, BT_u32 ulChannel) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;


	pRegs->PWMENABLE |= 0x00000003 << ulChannel*2;
	pRegs->PWMBlocks[ulChannel].CTL |= 0x00000001;

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_stop(BT_HANDLE hMCPWM, BT_u32 ulChannel) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	pRegs->PWMENABLE &= ~(0x00000003 << ulChannel*2);
	pRegs->PWMBlocks[ulChannel].CTL &= ~0x00000001;

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_setconfig(BT_HANDLE hMCPWM, BT_MCPWM_CONFIG *pConfig) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;
	BT_u32 i;

	pRegs->PWMENUPD = 0x0000AAAA;
	for (i = 0; i < 3; i++) {
		pRegs->PWMBlocks[i].CTL = 0x0000AA80;
	}

	return BT_ERR_NONE;
}

static BT_ERROR mcpwm_getconfig(BT_HANDLE hMCPWM, BT_MCPWM_CONFIG *pConfig) {
	volatile LM3Sxx_MCPWM_REGS *pRegs = hMCPWM->pRegs;

	/*@@if (pRegs->MCCON & LM3Sxx_MCPWM_MCCON_ACMODE)
		pConfig->eMode = BT_MCPWM_MODE_AC;
	else if (pRegs->MCCON & LM3Sxx_MCPWM_MCCON_DCMODE)
		pConfig->eMode = BT_MCPWM_MODE_DC;
	else pConfig->eMode = BT_MCPWM_MODE_INDEPENDENT;
	*/

	return BT_ERR_NONE;
}

/**
 *	This actually allows the mcpwm'S to be clocked!
 **/
static void enableMCPWMPeripheralClock(BT_HANDLE hMCPWM) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMCPWM->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[0] |= LM3Sxx_RCC_RCGC_PWM0EN;
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
static void disableMCPWMPeripheralClock(BT_HANDLE hMCPWM) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMCPWM->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[0] &= ~LM3Sxx_RCC_RCGC_PWM0EN;
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
static BT_BOOL isMCPWMPeripheralClockEnabled(BT_HANDLE hMCPWM) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hMCPWM->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LM3Sxx_RCC->RCGC[0] & LM3Sxx_RCC_RCGC_PWM0EN) {
			return BT_TRUE;
		}
		break;
	}
	default: {
		break;
	}
	}

	return BT_FALSE;
}

/**
 *	This implements the MCPWM power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR MCPWMSetPowerState(BT_HANDLE hMCPWM, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableMCPWMPeripheralClock(hMCPWM);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableMCPWMPeripheralClock(hMCPWM);
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
 *	This implements the MCPWM power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR MCPWMGetPowerState(BT_HANDLE hMCPWM, BT_POWER_STATE *pePowerState) {
	if(isMCPWMPeripheralClockEnabled(hMCPWM)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_MCPWM oMCPWMDeviceInterface= {
	.pfnSetConfig 				= mcpwm_setconfig,
	.pfnGetConfig				= mcpwm_getconfig,
	.pfnStart					= mcpwm_start,
	.pfnStop					= mcpwm_stop,
	.pfnGetChannelConfig		= mcpwm_get_channelconfig,
	.pfnSetChannelConfig		= mcpwm_set_channelconfig,
	.pfnGetChannelPulsewidth	= mcpwm_get_channelpulsewidth,
	.pfnSetChannelPulsewidth	= mcpwm_set_channelpulsewidth,
	.pfnSetDCModePattern		= mcpwm_set_DCModePattern,
	.pfnRegisterCallback		= mcpwm_register_callback,
	.pfnUnregisterCallback		= mcpwm_unregister_callback,
	.pfnEnableInterrupt			= mcpwm_enable_interrupt,
	.pfnDisableInterrupt		= mcpwm_disable_interrupt,
};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oMCPWMDeviceInterface,
};

static const BT_IF_POWER oPowerInterface = {
	MCPWMSetPowerState,											///< Pointers to the power state API implementations.
	MCPWMGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LM3Sxx_MCPWM_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_MCPWM,											///< Allow configuration through the MCPWM api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oMCPWMDeviceInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_MCPWM_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = mcpwm_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE mcpwm_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hMCPWM = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_MCPWM_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hMCPWM = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hMCPWM) {
		goto err_out;
	}

	hMCPWM->id = pResource->ulStart;

	g_MCPWM_HANDLES[pResource->ulStart] = hMCPWM;

	hMCPWM->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hMCPWM->pRegs = (LM3Sxx_MCPWM_REGS *) pResource->ulStart;

	MCPWMSetPowerState(hMCPWM, BT_POWER_STATE_AWAKE);

	ResetMCPWM(hMCPWM);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, mcpwm_irq_handler, hMCPWM);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hMCPWM;

err_free_out:
	BT_DestroyHandle(hMCPWM);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF mcpwm_driver = {
	.name 		= "LM3Sxx,mcpwm",
	.pfnProbe	= mcpwm_probe,
};


#ifdef BT_CONFIG_MACH_LM3Sxx_MCPWM_0
static const BT_RESOURCE oLM3Sxx_mcpwm0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_MCPWM0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_MCPWM0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 25,
		.ulEnd				= 25,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_mcpwm0_device = {
	.name 					= "LM3Sxx,mcpwm",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_mcpwm0_resources),
	.pResources 			= oLM3Sxx_mcpwm0_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_mcpwm0_inode = {
	.szpName = "mcpwm0",
	.pDevice = &oLM3Sxx_mcpwm0_device,
};
#endif
