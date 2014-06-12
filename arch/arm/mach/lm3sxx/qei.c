/**
 *	Provides the LM3Sxx  qei for BitThunder.
 **/

#include <bitthunder.h>
#include "qei.h"
#include "rcc.h"


BT_DEF_MODULE_NAME			("LM3Sxx-QEI")
BT_DEF_MODULE_DESCRIPTION	("LM3Sxx qei kernel driver")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a QEI driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/

struct _QEI_CALLBACK_HANDLE {
	BT_HANDLE_HEADER	h;
	BT_QEI_CALLBACK		pfnCallback;
	void 			   *pParam;
	BT_u32				ulInt;
	void			   *pNext;
} ;


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 				h;			///< All handles must include a handle header.
	LM3Sxx_QEI_REGS	   			   *pRegs;
	const BT_INTEGRATED_DEVICE	   *pDevice;
	BT_u32							id;
	BT_BOOL							bUseIndexToResetPosition;
	struct _QEI_CALLBACK_HANDLE	   *pCallback;
};

static BT_HANDLE g_QEI_HANDLES[2] = {
	NULL,
	NULL,
};

static void disableQEIPeripheralClock(BT_HANDLE hQEI);

static const BT_IF_HANDLE oCallbackHandleInterface;

BT_ERROR BT_NVIC_IRQ_29(void) {
	volatile BT_HANDLE hQEI = g_QEI_HANDLES[0];
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;
	// Clear the position interrupt
	BT_u32 ulIntStatus = pRegs->QEIISC;
	pRegs->QEIISC = ulIntStatus;

	struct _QEI_CALLBACK_HANDLE *pCallBack = (struct _QEI_CALLBACK_HANDLE*)hQEI->pCallback;
	//while (pCallBack) {
	if (pCallBack) {
		if ((pCallBack->ulInt & ulIntStatus) == pCallBack->ulInt)
			pCallBack->pfnCallback(hQEI, pCallBack->pParam);
		pCallBack = pCallBack->pNext;
	}

	if (hQEI->bUseIndexToResetPosition)
		pRegs->QEICTL = LM3Sxx_QEI_QEICTL_RESMODE;

	return 0;
}

BT_ERROR BT_NVIC_IRQ_54(void) {
	volatile BT_HANDLE hQEI = g_QEI_HANDLES[1];
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;
	// Clear the position interrupt
	BT_u32 ulIntStatus = pRegs->QEIISC;
	pRegs->QEIISC = ulIntStatus;

	struct _QEI_CALLBACK_HANDLE *pCallBack = (struct _QEI_CALLBACK_HANDLE*)hQEI->pCallback;
	//while (pCallBack) {
	if (pCallBack) {
		if ((pCallBack->ulInt & ulIntStatus) == pCallBack->ulInt)
			pCallBack->pfnCallback(hQEI, pCallBack->pParam);
		pCallBack = pCallBack->pNext;
	}

	if (hQEI->bUseIndexToResetPosition)
		pRegs->QEICTL = LM3Sxx_QEI_QEICTL_RESMODE;

	return 0;
}


static void ResetQEI(BT_HANDLE hQEI)
{
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	pRegs->QEICTL = 0x00000000;
	pRegs->QEIINTEN = 0x00000000;
}

static BT_ERROR qei_cleanup(BT_HANDLE hQEI) {
	ResetQEI(hQEI);

	// Disable peripheral clock.
	disableQEIPeripheralClock(hQEI);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hQEI->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hQEI->pDevice, BT_RESOURCE_ENUM, 0);

	g_QEI_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

static BT_u32 qei_get_index_count(BT_HANDLE hQEI, BT_ERROR *pError) {
	if (pError)
		*pError= BT_ERR_NONE;

	return 0;
}

static BT_u32 qei_get_position(BT_HANDLE hQEI, BT_ERROR *pError) {
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	if (pError)
		*pError = BT_ERR_NONE;

	return pRegs->QEIPOS;
}

static BT_ERROR qei_set_maximum_position(BT_HANDLE hQEI, BT_u32 ulValue) {
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	pRegs->QEIMAXPOS = ulValue;

	return BT_ERR_NONE;
}

static BT_ERROR qei_set_position_comparator(BT_HANDLE hQEI, BT_u32 ulChannel, BT_u32 ulValue) {

	return BT_ERR_NONE;
}

static BT_s32 qei_get_velocity(BT_HANDLE hQEI, BT_ERROR *pError) {
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	if (pError)
		*pError= BT_ERR_NONE;

	BT_u32 ulInputClk = BT_LM3Sxx_GetSystemFrequency();

	BT_s32 slVelocity = pRegs->QEISPEED * (ulInputClk / pRegs->QEILOAD);

	if (!(pRegs->QEICTL & LM3Sxx_QEI_QEICTL_SWAP))	{
		if (pRegs->QEISTAT & LM3Sxx_QEI_QEISTAT_DIR)
			slVelocity = -slVelocity;
	}
	else {
		if (!(pRegs->QEISTAT & LM3Sxx_QEI_QEISTAT_DIR))
			slVelocity = -slVelocity;
	}

	return slVelocity;
}


static BT_ERROR qei_setconfig(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig) {
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	pRegs->QEICTL = 0x00;

	if (pConfig->eDirection 	== BT_QEI_DIRECTION_NEG)
		pRegs->QEICTL |= LM3Sxx_QEI_QEICTL_SWAP;
	if (pConfig->eSignalMode 	== BT_QEI_SIGNAL_DIR_CLK)
		pRegs->QEICTL |= LM3Sxx_QEI_QEICTL_SIGMODE;
	if (pConfig->eCaptureMode 	== BT_QEI_CAPTURE_4_EDGES)
		pRegs->QEICTL |= LM3Sxx_QEI_QEICTL_CAPMODE;

	BT_u32 ulInputClk = BT_LM3Sxx_GetSystemFrequency();

	pRegs->QEILOAD = ulInputClk / pConfig->ulVelocityUpdateRate;

	pRegs->QEICTL |= LM3Sxx_QEI_QEICTL_FILTER(5);
	pRegs->QEICTL |= LM3Sxx_QEI_QEICTL_FILTER_ENABLE;
	pRegs->QEICTL |= LM3Sxx_QEI_QEICTL_VELEN;

	hQEI->bUseIndexToResetPosition = pConfig->bUseIndexToResetPosition;

	if (pConfig->bUseIndexToResetPosition)
	{
		pRegs->QEICTL = LM3Sxx_QEI_QEICTL_RESMODE;
	}

	return BT_ERR_NONE;
}

static BT_ERROR qei_getconfig(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig) {

	return BT_ERR_NONE;
}

static BT_ERROR qei_enable_interrupt(BT_HANDLE hQEI, BT_u32 ulType) {
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	pRegs->QEIINTEN |= ulType;

	return BT_ERR_NONE;
}

static BT_ERROR qei_disable_interrupt(BT_HANDLE hQEI, BT_u32 ulType) {
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	pRegs->QEIINTEN &= ~ulType;

	return BT_ERR_NONE;
}

static BT_ERROR qei_clear_interrupt(BT_HANDLE hQEI, BT_u32 ulType) {
	volatile LM3Sxx_QEI_REGS *pRegs = hQEI->pRegs;

	pRegs->QEIISC = ulType;

	return BT_ERR_NONE;
}

static BT_ERROR qei_callback_cleanup(BT_HANDLE hCallback) {
	struct _QEI_CALLBACK_HANDLE *hCleanup = (struct _QEI_CALLBACK_HANDLE*)hCallback;

	hCleanup->pfnCallback = NULL;
	hCleanup->pParam	  = NULL;

	BT_CloseHandle((BT_HANDLE)hCleanup);

	return BT_ERR_NONE;
}

static BT_HANDLE qei_register_callback(BT_HANDLE hQEI, BT_QEI_CALLBACK pfnCallback, void *pParam, BT_u32 ulInterruptID, BT_ERROR *pError) {
	struct _QEI_CALLBACK_HANDLE *pCallback = (struct _QEI_CALLBACK_HANDLE*)BT_CreateHandle(&oCallbackHandleInterface, sizeof(struct _QEI_CALLBACK_HANDLE),pError);

	if (pCallback) {
		pCallback->pfnCallback = pfnCallback;
		pCallback->pParam	   = pParam;
		pCallback->ulInt	   = ulInterruptID;

		hQEI->pCallback        = pCallback;
	}

	return (BT_HANDLE)pCallback;
}

static BT_ERROR qei_unregister_callback(BT_HANDLE hQEI, BT_HANDLE hCallback) {
	qei_callback_cleanup(hCallback);

	hQEI->pCallback = NULL;

	return BT_ERR_NONE;
}



/**
 *	This actually allows the qei'S to be clocked!
 **/
static void enableQEIPeripheralClock(BT_HANDLE hQEI) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hQEI->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_QEI0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_QEI1EN;
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
static void disableQEIPeripheralClock(BT_HANDLE hQEI) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hQEI->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_QEI0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_QEI1EN;
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
static BT_BOOL isQEIPeripheralClockEnabled(BT_HANDLE hQEI) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hQEI->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_QEI0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_QEI1EN) {
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
 *	This implements the QEI power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR QEISetPowerState(BT_HANDLE hQEI, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableQEIPeripheralClock(hQEI);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableQEIPeripheralClock(hQEI);
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
 *	This implements the QEI power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR QEIGetPowerState(BT_HANDLE hQEI, BT_POWER_STATE *pePowerState) {
	if(isQEIPeripheralClockEnabled(hQEI)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_QEI oQEIDeviceInterface= {
	.pfnSetConfig 				= qei_setconfig,
	.pfnGetConfig				= qei_getconfig,
	.pfnGetIndexCount			= qei_get_index_count,
	.pfnGetPosition				= qei_get_position,
	.pfnSetMaximumPosition		= qei_set_maximum_position,
	.pfnSetPositionComparator	= qei_set_position_comparator,
	.pfnGetVelocity				= qei_get_velocity,
	.pfnEnableInterrupt			= qei_enable_interrupt,
	.pfnDisableInterrupt		= qei_disable_interrupt,
	.pfnClearInterrupt			= qei_clear_interrupt,
	.pfnRegisterCallback 		= qei_register_callback,
	.pfnUnregisterCallback		= qei_unregister_callback,

};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oQEIDeviceInterface,
};

static const BT_IF_POWER oPowerInterface = {
	QEISetPowerState,											///< Pointers to the power state API implementations.
	QEIGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LM3Sxx_QEI_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_QEI,											///< Allow configuration through the QEI api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oQEIDeviceInterface,
	},
};

static const BT_IF_HANDLE oCallbackHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType		= BT_HANDLE_T_CALLBACK,											///< Handle Type!
	.pfnCleanup = qei_callback_cleanup,												///< Handle's cleanup routine.
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_QEI_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = qei_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE qei_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hQEI = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_QEI_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hQEI = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hQEI) {
		goto err_out;
	}

	hQEI->id = pResource->ulStart;

	g_QEI_HANDLES[pResource->ulStart] = hQEI;

	hQEI->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hQEI->pRegs = (LM3Sxx_QEI_REGS *) pResource->ulStart;

	QEISetPowerState(hQEI, BT_POWER_STATE_AWAKE);

	ResetQEI(hQEI);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, qei_irq_handler, hQEI);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hQEI;

err_free_out:
	BT_DestroyHandle(hQEI);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF qei_driver = {
	.name 		= "LM3Sxx,qei",
	.pfnProbe	= qei_probe,
};


#ifdef BT_CONFIG_MACH_LM3Sxx_QEI_0
static const BT_RESOURCE oLM3Sxx_qei0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_QEI0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_QEI0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 29,
		.ulEnd				= 29,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_qei0_device = {
	.name 					= "LM3Sxx,qei",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_qei0_resources),
	.pResources 			= oLM3Sxx_qei0_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_qei0_inode = {
	.szpName = "qei0",
	.pDevice = &oLM3Sxx_qei0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_QEI_1
static const BT_RESOURCE oLM3Sxx_qei1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_QEI1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_QEI1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 54,
		.ulEnd				= 54,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_qei1_device = {
	.name 					= "LM3Sxx,qei",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_qei1_resources),
	.pResources 			= oLM3Sxx_qei1_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_qei1_inode = {
	.szpName = "qei1",
	.pDevice = &oLM3Sxx_qei1_device,
};
#endif
