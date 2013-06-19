/**
 *	ARM CPU Private Timer & Watchdog Timer Driver for BitThunder
 *
 *	@author	James Walmsley
 *
 **/
#include <bitthunder.h>
#include <arch/common/cortex-a9-cpu-timers.h>

BT_DEF_MODULE_NAME					("ARM Cortex-A9 CPU Timers")
BT_DEF_MODULE_DESCRIPTION			("Timer driver implementation for Cortex-A9 CPU timers")
BT_DEF_MODULE_AUTHOR				("James Walmsley")
BT_DEF_MODULE_EMAIL					("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 				h;
	volatile CORTEX_A9_TIMER_REGS  *pRegs;
	BT_u32							ulTimerID;
	const BT_INTEGRATED_DEVICE 	   *pDevice;		///< Get the IRQ resources for cleanup.
	BT_FN_INTERRUPT_HANDLER		  	pfnHandler;
	void *pParam;
};

BT_BOOL g_toogle = BT_TRUE;

static BT_ERROR timer_irq_handler(BT_u32 ulIRQ, void *pParam) {
	BT_HANDLE hTimer = (BT_HANDLE) pParam;

	if(hTimer->pfnHandler) {
		hTimer->pfnHandler(ulIRQ, hTimer->pParam);
	}

	// ACK the timer interrupt.
	hTimer->pRegs->timers[hTimer->ulTimerID].INT_STATUS = 1;

	return BT_ERR_NONE;
}

static BT_ERROR timer_cleanup(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_u32 timer_get_clock_rate(BT_HANDLE hTimer, BT_ERROR *pError) {
	BT_u32 InputClk = BT_GetCpuClockFrequency();
	InputClk /= 2;
	InputClk /= CA9_TIM_CONTROL_PRESCALER_GET(hTimer->pRegs->timers[hTimer->ulTimerID].CONTROL) + 1;
	return InputClk;
}

static BT_ERROR timer_register_interrupt(BT_HANDLE hTimer, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	hTimer->pfnHandler 	= pfnHandler;
	hTimer->pParam 		= pParam;
	return BT_ERR_NONE;
}

static BT_ERROR timer_start(BT_HANDLE hTimer) {
	hTimer->pRegs->timers[hTimer->ulTimerID].COUNT = 0;
	hTimer->pRegs->timers[hTimer->ulTimerID].CONTROL |= 0x1;
	return BT_ERR_NONE;
}

static BT_ERROR timer_stop(BT_HANDLE hTimer) {
	hTimer->pRegs->timers[hTimer->ulTimerID].CONTROL &= ~0x1;
	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_interrupt(BT_HANDLE hTimer) {
	hTimer->pRegs->timers[hTimer->ulTimerID].CONTROL |= 0x4;
	return BT_ERR_NONE;
}

static BT_ERROR timer_disable_interrupt(BT_HANDLE hTimer) {
	hTimer->pRegs->timers[hTimer->ulTimerID].CONTROL &= ~0x4;
	return BT_ERR_NONE;
}

static BT_u32 timer_get_frequency(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(pError) {
		*pError = BT_ERR_NONE;
	}

	BT_u32 ulTickRate 	= timer_get_clock_rate(hTimer, pError);
	BT_u32 ulFrequency 	= ulTickRate / hTimer->pRegs->timers[hTimer->ulTimerID].LOAD;



	return ulFrequency;
}

static BT_ERROR timer_set_frequency(BT_HANDLE hTimer, BT_u32 ulFrequencyHz) {

	BT_ERROR Error;
	BT_u32 ulTickRate 	= timer_get_clock_rate(hTimer, &Error);
	BT_u32 ulDivisor  	= ulTickRate / ulFrequencyHz;

	hTimer->pRegs->timers[hTimer->ulTimerID].LOAD = ulDivisor;

	return BT_ERR_NONE;
}

static BT_u32 timer_get_offset(BT_HANDLE hTimer, BT_ERROR *pError) {
	return hTimer->pRegs->timers[hTimer->ulTimerID].LOAD - hTimer->pRegs->timers[hTimer->ulTimerID].COUNT;
}

static const BT_DEV_IF_SYSTIMER oDeviceOps = {
	.pfnGetClockRate		= timer_get_clock_rate,
	.pfnRegisterInterrupt	= timer_register_interrupt,
	.pfnEnableInterrupt		= timer_enable_interrupt,
	.pfnDisableInterrupt	= timer_disable_interrupt,
	.pfnGetFrequency		= timer_get_frequency,
	.pfnSetFrequency		= timer_set_frequency,
	.pfnGetOffset			= timer_get_offset,
	.pfnStart				= timer_start,
	.pfnStop				= timer_stop,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_SYSTIMER,
	.unConfigIfs = {
		.pSysTimerIF = &oDeviceOps,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = timer_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

/**
 *
 *
 **/
static BT_HANDLE timer_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError, BT_BOOL bWatchdog) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_HANDLE hTimer = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hTimer) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	if(bWatchdog) {
		hTimer->ulTimerID = CA9_WD_TIMER;
	} else {
		hTimer->ulTimerID = CA9_PRIV_TIMER;
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hTimer->pRegs = (CORTEX_A9_TIMER_REGS *) bt_ioremap((void *)pResource->ulStart, BT_SIZE_4K, &Error);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	BT_u32 ulIRQ;

	if(!bWatchdog) {
		ulIRQ = pResource->ulStart;	// ulStart provides the IRQ number for CPU private timer
	} else {
		ulIRQ = pResource->ulEnd;	// ulEnd should contain the IRQ for the WD timer.
	}

	Error = BT_RegisterInterrupt(ulIRQ, timer_irq_handler, hTimer);
	if(Error) {
		goto err_free_out;
	}

	BT_EnableInterrupt(ulIRQ);

	hTimer->pRegs->timers[hTimer->ulTimerID].CONTROL = 0x6;

	return hTimer;

err_free_out:
	BT_DestroyHandle(hTimer);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

static BT_HANDLE timer_cpu_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {
	return timer_probe(pDevice, pError, BT_FALSE);
}

static BT_HANDLE timer_wdt_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {
	return timer_probe(pDevice, pError, BT_TRUE);
}

BT_INTEGRATED_DRIVER_DEF oIntegratedTimerDriver = {
	.name		= "arm,cortex-a9,cpu-timer",
	.pfnProbe 	= timer_cpu_probe,
};

BT_INTEGRATED_DRIVER_DEF oIntegratedWdtDriver = {
	.name 		= "arm,cortex-a9,wdt",
	.pfnProbe	= timer_wdt_probe,
};
