/**
 *	Provides the Xilinx system timer for BitThunder.
 **/

#include <bitthunder.h>
#include "timer.h"
#include "rcc.h"


BT_DEF_MODULE_NAME			("LM3Sxx-TIMER")
BT_DEF_MODULE_DESCRIPTION	("LM3Sxx Timers kernel driver, also providing kernel tick")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

struct _TIMER_CALLBACK_HANDLE {
	BT_HANDLE_HEADER	h;
	BT_TIMER_CALLBACK	pfnCallback;
	void 			   *pParam;
} ;

/**
 *	We can define how a handle should look in a Timer driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LM3Sxx_TIMER_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	struct _TIMER_CALLBACK_HANDLE	   *pCallback;
};

static BT_HANDLE g_TIMER_HANDLES[4] = {
	NULL,
	NULL,
	NULL,
};

static void disableTimerPeripheralClock(BT_HANDLE hTimer);

static const BT_IF_HANDLE oCallbackHandleInterface;

BT_ERROR BT_NVIC_IRQ_17(void) {
	volatile BT_HANDLE Handle = g_TIMER_HANDLES[0];
	volatile LM3Sxx_TIMER_REGS *pRegs = Handle->pRegs;

	BT_u32 IRValue = pRegs->TMRMIS;

	pRegs->TMRICR = IRValue;

	if (Handle->pCallback)
		Handle->pCallback->pfnCallback(Handle, Handle->pCallback->pParam);

	return 0;
}

BT_ERROR BT_NVIC_IRQ_18(void) {
	volatile BT_HANDLE Handle = g_TIMER_HANDLES[1];
	volatile LM3Sxx_TIMER_REGS *pRegs = Handle->pRegs;

	BT_u32 IRValue = pRegs->TMRMIS;

	pRegs->TMRICR = IRValue;

	if (Handle->pCallback)
		Handle->pCallback->pfnCallback(Handle, Handle->pCallback->pParam);

	return 0;
}

BT_ERROR BT_NVIC_IRQ_19(void) {
	volatile BT_HANDLE Handle = g_TIMER_HANDLES[2];
	volatile LM3Sxx_TIMER_REGS *pRegs = Handle->pRegs;

	BT_u32 IRValue = pRegs->TMRMIS;

	pRegs->TMRICR = IRValue;

	if (Handle->pCallback)
		Handle->pCallback->pfnCallback(Handle, Handle->pCallback->pParam);

	return 0;
}

BT_ERROR BT_NVIC_IRQ_20(void) {
	volatile BT_HANDLE Handle = g_TIMER_HANDLES[3];
	volatile LM3Sxx_TIMER_REGS *pRegs = Handle->pRegs;

	BT_u32 IRValue = pRegs->TMRMIS;

	pRegs->TMRICR = IRValue;

	if (Handle->pCallback)
		Handle->pCallback->pfnCallback(Handle, Handle->pCallback->pParam);

	return BT_ERR_NONE;
}


static void ResetTimer(BT_HANDLE hTimer) {
}

static BT_ERROR timer_cleanup(BT_HANDLE hTimer) {
	ResetTimer(hTimer);

	// Disable peripheral clock.
	disableTimerPeripheralClock(hTimer);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	g_TIMER_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

static BT_u32 timer_getinputclock(BT_HANDLE hTimer, BT_ERROR *pError) {
	if (pError)
		*pError = BT_ERR_NONE;

	return BT_LM3Sxx_GetMainFrequency();
}

static BT_ERROR timer_setconfig(BT_HANDLE hTimer, void *pConfig) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	return BT_ERR_NONE;
}

static BT_ERROR timer_getconfig(BT_HANDLE hTimer, void *pConfig) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	return BT_ERR_NONE;
}


static BT_ERROR timer_start(BT_HANDLE hTimer) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	//pRegs->TMRBTCR |= LM3Sxx_TIMER_TMRBTCR_CEn;

	return BT_ERR_NONE;
}

static BT_ERROR timer_stop(BT_HANDLE hTimer) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	//pRegs->TMRBTCR &= ~LM3Sxx_TIMER_TMRBTCR_CEn;

	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_ERROR timer_disable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_ERROR timer_callback_cleanup(BT_HANDLE hCallback) {
	struct _TIMER_CALLBACK_HANDLE *hCleanup = (struct _TIMER_CALLBACK_HANDLE*)hCallback;

	hCleanup->pfnCallback = NULL;
	hCleanup->pParam	  = NULL;

	BT_CloseHandle((BT_HANDLE)hCleanup);

	return BT_ERR_NONE;
}

static BT_HANDLE timer_register_callback(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	struct _TIMER_CALLBACK_HANDLE *pCallback = (struct _TIMER_CALLBACK_HANDLE*)BT_CreateHandle(&oCallbackHandleInterface, sizeof(struct _TIMER_CALLBACK_HANDLE),pError);

	if (pCallback) {
		pCallback->pfnCallback = pfnCallback;
		pCallback->pParam	   = pParam;

		hTimer->pCallback      = pCallback;
	}

	return (BT_HANDLE)pCallback;
}

static BT_ERROR timer_unregister_callback(BT_HANDLE hTimer, BT_HANDLE hCallback) {
	timer_callback_cleanup(hCallback);

	hTimer->pCallback = NULL;

	return BT_ERR_NONE;
}


static BT_u32 timer_get_prescaler(BT_HANDLE hTimer, BT_ERROR *pError) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if (pError)
		*pError = BT_ERR_NONE;

	//return pRegs->TMRBPR+1;
	return 1;
}

static BT_ERROR timer_set_prescaler(BT_HANDLE hTimer, BT_u32 ulPrescaler) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	//pRegs->TMRBPR = ulPrescaler-1;

	return BT_ERR_NONE;
}

static BT_u32 timer_get_period_count(BT_HANDLE hTimer, BT_ERROR *pError) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if (pError)
		*pError= BT_ERR_NONE;

	//return pRegs->TMRBMR3;
	return 1;
}

static BT_ERROR timer_set_period_count(BT_HANDLE hTimer, BT_u32 ulValue) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	//pRegs->TMRBMR3 = ulValue;
	//pRegs->TMRBMCR |= LM3Sxx_TIMER_TMRMCR_MR3R;

	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_reload(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_disable_reload(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_getvalue(BT_HANDLE hTimer, BT_ERROR *pError) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	if (pError)
		*pError= BT_ERR_NONE;

	return 1;
	//return pRegs->TMRBTC;
}

static BT_ERROR timer_setvalue(BT_HANDLE hTimer, BT_u32 ulValue) {
	//volatile LM3Sxx_TIMER_REGS *pRegs = hTimer->pRegs;

	//pRegs->TMRBTC = ulValue;

	return BT_ERR_NONE;
}

/**
 *	This actually allows the TimerS to be clocked!
 **/
static void enableTimerPeripheralClock(BT_HANDLE hTimer) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER1EN;
		break;
	}
	case 2: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER2EN;
		break;
	}
	case 3: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_TIMER3EN;
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
static void disableTimerPeripheralClock(BT_HANDLE hTimer) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER1EN;
		break;
	}
	case 2: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER2EN;
		break;
	}
	case 3: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_TIMER3EN;
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
static BT_BOOL isTimerPeripheralClockEnabled(BT_HANDLE hTimer) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER1EN) {
			return BT_TRUE;
		}
		break;
	}
	case 2: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER2EN) {
			return BT_TRUE;
		}
		break;
	}
	case 3: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_TIMER3EN) {
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
 *	This implements the Timer power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR TimerSetPowerState(BT_HANDLE hTimer, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableTimerPeripheralClock(hTimer);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableTimerPeripheralClock(hTimer);
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
 *	This implements the Timer power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR TimerGetPowerState(BT_HANDLE hTimer, BT_POWER_STATE *pePowerState) {
	if(isTimerPeripheralClockEnabled(hTimer)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/*
 *
 *
 *
 */

static const BT_DEV_IF_TIMER oTimerDeviceInterface= {
	timer_getinputclock,
	timer_setconfig,
	timer_getconfig,
	timer_start,
	timer_stop,
	timer_enable_interrupt,
	timer_disable_interrupt,
	NULL,
	timer_register_callback,
	timer_unregister_callback,
	timer_get_prescaler,
	timer_set_prescaler,
	timer_get_period_count,
	timer_set_period_count,
	timer_enable_reload,
	timer_disable_reload,
	timer_getvalue,
	timer_setvalue,
};

static const BT_DEV_IFS oDeviceInterface = {
	(BT_DEV_INTERFACE) &oTimerDeviceInterface,
};

static const BT_IF_POWER oPowerInterface = {
	TimerSetPowerState,											///< Pointers to the power state API implementations.
	TimerGetPowerState,											///< This gets the current power state.
};

const BT_IF_DEVICE BT_LM3Sxx_TIMER_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_TIMER,											///< Allow configuration through the Timer api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oTimerDeviceInterface,
	},
};

static const BT_IF_HANDLE oCallbackHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType		= BT_HANDLE_T_CALLBACK,											///< Handle Type!
	.pfnCleanup = timer_callback_cleanup,												///< Handle's cleanup routine.
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_TIMER_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = timer_cleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE timer_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hTimer = NULL;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_TIMER_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hTimer = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hTimer) {
		goto err_out;
	}

	g_TIMER_HANDLES[pResource->ulStart] = hTimer;

	hTimer->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hTimer->pRegs = (LM3Sxx_TIMER_REGS *) pResource->ulStart;

	TimerSetPowerState(hTimer, BT_POWER_STATE_AWAKE);

	ResetTimer(hTimer);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, timer_irq_handler, hTimer);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hTimer;

err_free_out:
	BT_kFree(hTimer);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF timer_driver = {
	.name 		= "LM3Sxx,timer",
	.pfnProbe	= timer_probe,
};


#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_0
static const BT_RESOURCE oLM3Sxx_timer0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 35,
		.ulEnd				= 36,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer0_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer0_resources),
	.pResources 			= oLM3Sxx_timer0_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer0_inode = {
	.szpName = "timer0",
	.pDevice = &oLM3Sxx_timer0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_1
static const BT_RESOURCE oLM3Sxx_timer1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 37,
		.ulEnd				= 38,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer1_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer1_resources),
	.pResources 			= oLM3Sxx_timer1_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer1_inode = {
	.szpName = "timer1",
	.pDevice = &oLM3Sxx_timer1_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_2
static const BT_RESOURCE oLM3Sxx_timer2_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER2_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER2_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 2,
		.ulEnd				= 2,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 39,
		.ulEnd				= 40,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer2_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer2_resources),
	.pResources 			= oLM3Sxx_timer2_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer2_inode = {
	.szpName = "timer2",
	.pDevice = &oLM3Sxx_timer2_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_TIMER_3
static const BT_RESOURCE oLM3Sxx_timer3_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_TIMER3_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_TIMER3_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 3,
		.ulEnd				= 3,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 51,
		.ulEnd				= 52,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLM3Sxx_timer3_device = {
	.name 					= "LM3Sxx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_timer3_resources),
	.pResources 			= oLM3Sxx_timer3_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_timer3_inode = {
	.szpName = "timer3",
	.pDevice = &oLM3Sxx_timer3_device,
};
#endif
