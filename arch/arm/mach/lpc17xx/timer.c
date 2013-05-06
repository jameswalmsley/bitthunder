/**
 *	Provides the Xilinx system timer for BitThunder.
 **/

#include <bitthunder.h>
#include "timer.h"
#include "rcc.h"


BT_DEF_MODULE_NAME			("LPC17xx-TIMER")
BT_DEF_MODULE_DESCRIPTION	("LPC17xx Timers kernel driver, also providing kernel tick")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a Timer driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LPC17xx_TIMER_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
};

static BT_HANDLE g_TIMER_HANDLES[4] = {
	NULL,
	NULL,
	NULL,
	NULL,
};

static const BT_u32 g_TIMER_PERIPHERAL[4] = {1, 2, 22, 23};

static void disableTimerPeripheralClock(BT_HANDLE hTimer);

BT_ERROR BT_NVIC_IRQ_17(void) {
	BT_u32 IRValue = TIMER0->TMRBIR;

	TIMER0->TMRBIR = IRValue;

	return 0;
}

BT_ERROR BT_NVIC_IRQ_18(void) {
	BT_u32 IRValue = TIMER1->TMRBIR;

	TIMER1->TMRBIR = IRValue;

	return 0;
}

BT_ERROR BT_NVIC_IRQ_19(void) {
	BT_u32 IRValue = TIMER2->TMRBIR;

	TIMER2->TMRBIR = IRValue;

	return 0;
}

BT_ERROR BT_NVIC_IRQ_20(void) {
	BT_u32 IRValue = TIMER3->TMRBIR;

	TIMER3->TMRBIR = IRValue;

	return BT_ERR_NONE;
}


static void ResetTimer(BT_HANDLE hTimer)
{
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	pRegs->TMRBIR	= 0;
	pRegs->TMRBTCR	= 0;
	pRegs->TMRBTC	= 0;
	pRegs->TMRBPR	= 0;
	pRegs->TMRBPC	= 0;
	pRegs->TMRBMCR	= 0;
	pRegs->TMRBMR0	= 0;
	pRegs->TMRBMR1	= 0;
	pRegs->TMRBMR2	= 0;
	pRegs->TMRBMR3	= 0;
	pRegs->TMRBCCR	= 0;
	pRegs->TMRBCR0	= 0;
	pRegs->TMRBCR1	= 0;
	pRegs->TMRBEMR	= 0;
	pRegs->TMRBCTCR	= 0;
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
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	*pError = BT_ERR_NONE;

	return BT_LPC17xx_GetPeripheralClock(g_TIMER_PERIPHERAL[hTimer->pDevice->id]) / (pRegs->TMRBPR+1);
}

static BT_ERROR timer_setconfig(BT_HANDLE hTimer, void *pConfig) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	pRegs->TMRBCTCR	= ((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Control;
	pRegs->TMRBMCR &= LPC17xx_TIMER_TMRMCR_MR3R;
	pRegs->TMRBMCR |= ((BT_LPC17xx_TIMER_CONFIG*)pConfig)->MatchControl;
	pRegs->TMRBMR0	= ((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Match[0];
	pRegs->TMRBMR1	= ((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Match[1];
	pRegs->TMRBMR2	= ((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Match[2];
	pRegs->TMRBCCR	= ((BT_LPC17xx_TIMER_CONFIG*)pConfig)->CaptureControl;
	pRegs->TMRBEMR	= ((BT_LPC17xx_TIMER_CONFIG*)pConfig)->ExtMatchControl;

	return BT_ERR_NONE;
}

static BT_ERROR timer_getconfig(BT_HANDLE hTimer, void *pConfig) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Control			= pRegs->TMRBCTCR;
	((BT_LPC17xx_TIMER_CONFIG*)pConfig)->MatchControl		= pRegs->TMRBMCR;
	((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Match[0]			= pRegs->TMRBMR0;
	((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Match[1]			= pRegs->TMRBMR1;
	((BT_LPC17xx_TIMER_CONFIG*)pConfig)->Match[2]			= pRegs->TMRBMR2;
	((BT_LPC17xx_TIMER_CONFIG*)pConfig)->CaptureControl	= pRegs->TMRBCCR;
	((BT_LPC17xx_TIMER_CONFIG*)pConfig)->ExtMatchControl	= pRegs->TMRBEMR;

	return BT_ERR_NONE;
}


static BT_ERROR timer_start(BT_HANDLE hTimer) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	pRegs->TMRBTCR |= LPC17xx_TIMER_TMRBTCR_CEn;

	return BT_ERR_NONE;
}

static BT_ERROR timer_stop(BT_HANDLE hTimer) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	pRegs->TMRBTCR &= ~LPC17xx_TIMER_TMRBTCR_CEn;

	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_ERROR timer_disable_interrupt(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_HANDLE timer_register_callback(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError) {
	return NULL;
}

static BT_ERROR timer_unregister_callback(BT_HANDLE hTimer, BT_HANDLE hCallback) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_get_prescaler(BT_HANDLE hTimer, BT_ERROR *pError) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	*pError = BT_ERR_NONE;

	return pRegs->TMRBPR+1;
}

static BT_ERROR timer_set_prescaler(BT_HANDLE hTimer, BT_u32 ulPrescaler) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	pRegs->TMRBPR = ulPrescaler-1;

	return BT_ERR_NONE;
}

static BT_u32 timer_get_period_count(BT_HANDLE hTimer, BT_ERROR *pError) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	*pError= BT_ERR_NONE;

	return pRegs->TMRBMR3;
}

static BT_ERROR timer_set_period_count(BT_HANDLE hTimer, BT_u32 ulValue) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	pRegs->TMRBMR3 = ulValue;
	pRegs->TMRBMCR |= LPC17xx_TIMER_TMRMCR_MR3R;

	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_reload(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_ERROR timer_disable_reload(BT_HANDLE hTimer) {
	return BT_ERR_UNIMPLEMENTED;
}

static BT_u32 timer_getvalue(BT_HANDLE hTimer, BT_ERROR *pError) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	*pError= BT_ERR_NONE;

	return pRegs->TMRBTC;
}

static BT_ERROR timer_setvalue(BT_HANDLE hTimer, BT_u32 ulValue) {
	volatile LPC17xx_TIMER_REGS *pRegs = hTimer->pRegs;

	pRegs->TMRBTC = ulValue;

	return BT_ERR_NONE;
}

/**
 *	This actually allows the TimerS to be clocked!
 **/
static void enableTimerPeripheralClock(BT_HANDLE hTimer) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hTimer->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC17xx_RCC->PCONP |= LPC17xx_RCC_PCONP_TIMER0EN;
		break;
	}
	case 1: {
		LPC17xx_RCC->PCONP |= LPC17xx_RCC_PCONP_TIMER1EN;
		break;
	}
	case 2: {
		LPC17xx_RCC->PCONP |= LPC17xx_RCC_PCONP_TIMER2EN;
		break;
	}
	case 3: {
		LPC17xx_RCC->PCONP |= LPC17xx_RCC_PCONP_TIMER3EN;
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
		LPC17xx_RCC->PCONP &= ~LPC17xx_RCC_PCONP_TIMER0EN;
		break;
	}
	case 1: {
		LPC17xx_RCC->PCONP &= ~LPC17xx_RCC_PCONP_TIMER1EN;
		break;
	}
	case 2: {
		LPC17xx_RCC->PCONP &= ~LPC17xx_RCC_PCONP_TIMER2EN;
		break;
	}
	case 3: {
		LPC17xx_RCC->PCONP &= ~LPC17xx_RCC_PCONP_TIMER3EN;
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
		if(LPC17xx_RCC->PCONP & LPC17xx_RCC_PCONP_TIMER0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LPC17xx_RCC->PCONP & LPC17xx_RCC_PCONP_TIMER1EN) {
			return BT_TRUE;
		}
		break;
	}
	case 2: {
		if(LPC17xx_RCC->PCONP & LPC17xx_RCC_PCONP_TIMER2EN) {
			return BT_TRUE;
		}
		break;
	}
	case 3: {
		if(LPC17xx_RCC->PCONP & LPC17xx_RCC_PCONP_TIMER3EN) {
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

const BT_IF_DEVICE BT_LPC17xx_TIMER_oDeviceInterface = {
	&oPowerInterface,											///< Device does support powerstate functionality.
	BT_DEV_IF_T_TIMER,											///< Allow configuration through the Timer api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oTimerDeviceInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LPC17xx_TIMER_oDeviceInterface,
	},
	BT_HANDLE_T_DEVICE,											///< Handle Type!
	timer_cleanup,												///< Handle's cleanup routine.
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

	hTimer->pRegs = (LPC17xx_TIMER_REGS *) pResource->ulStart;

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
	.name 		= "LPC17xx,timer",
	.pfnProbe	= timer_probe,
};


#ifdef BT_CONFIG_MACH_LPC17xx_TIMER_0
static const BT_RESOURCE oLPC17xx_timer0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_TIMER0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_TIMER0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 17,
		.ulEnd				= 17,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_timer0_device = {
	.id						= 0,
	.name 					= "LPC17xx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_timer0_resources),
	.pResources 			= oLPC17xx_timer0_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_timer0_inode = {
	.szpName = "timer0",
	.pDevice = &oLPC17xx_timer0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC17xx_TIMER_1
static const BT_RESOURCE oLPC17xx_timer1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_TIMER1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_TIMER1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 18,
		.ulEnd				= 18,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_timer1_device = {
	.id						= 1,
	.name 					= "LPC17xx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_timer1_resources),
	.pResources 			= oLPC17xx_timer1_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_timer1_inode = {
	.szpName = "timer1",
	.pDevice = &oLPC17xx_timer1_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC17xx_TIMER_2
static const BT_RESOURCE oLPC17xx_timer2_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_TIMER2_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_TIMER2_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 2,
		.ulEnd				= 2,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 19,
		.ulEnd				= 19,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_timer2_device = {
	.id						= 2,
	.name 					= "LPC17xx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_timer2_resources),
	.pResources 			= oLPC17xx_timer2_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_timer2_inode = {
	.szpName = "timer2",
	.pDevice = &oLPC17xx_timer2_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC17xx_TIMER_3
static const BT_RESOURCE oLPC17xx_timer3_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_TIMER3_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_TIMER3_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 3,
		.ulEnd				= 3,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 20,
		.ulEnd				= 20,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_timer3_device = {
	.id						= 3,
	.name 					= "LPC17xx,timer",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_timer3_resources),
	.pResources 			= oLPC17xx_timer3_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_timer3_inode = {
	.szpName = "timer3",
	.pDevice = &oLPC17xx_timer3_device,
};
#endif
