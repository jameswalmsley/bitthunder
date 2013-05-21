/**
 *	ARM Cortex-Mx Systick Peripheral Driver
 *
 **/

#include <bitthunder.h>
#include "timer.h"

BT_DEF_MODULE_NAME					("BCM2835 System Timer driver")
BT_DEF_MODULE_DESCRIPTION			("System timer interface for BCM2835 timer")
BT_DEF_MODULE_AUTHOR				("James Walmsley")
BT_DEF_MODULE_EMAIL					("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 				h;
	volatile BCM2835_TIMER_REGS	   *pRegs;
	BT_FN_INTERRUPT_HANDLER			pfnHandler;
	void *pParam;
};

static BT_ERROR timer_irq_handler(BT_u32 ulIRQ, void *pParam) {
	BT_HANDLE hTimer = (BT_HANDLE) pParam;

	if(hTimer->pfnHandler) {
		hTimer->pfnHandler(ulIRQ, hTimer->pParam);
	}

	// ACK the timer interrupt.
	hTimer->pRegs->CLI = 0;

	return BT_ERR_NONE;
}

static BT_ERROR timer_cleanup(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_u32 timer_get_clock_rate(BT_HANDLE hTimer, BT_ERROR *pError) {
	BT_u32 InputClk = BT_GetCpuClockFrequency();
	if(!(hTimer->pRegs->DIV & DIV_VAL)) {
		return InputClk;
	}
	return InputClk / hTimer->pRegs->DIV & DIV_VAL;
}

static BT_ERROR timer_register_interrupt(BT_HANDLE hTimer, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	hTimer->pfnHandler 	= pfnHandler;
	hTimer->pParam 		= pParam;
	return BT_ERR_NONE;
}

static BT_ERROR timer_start(BT_HANDLE hTimer) {
	hTimer->pRegs->CTL |= CTL_ENABLE;
	return BT_ERR_NONE;
}

static BT_ERROR timer_stop(BT_HANDLE hTimer) {
	hTimer->pRegs->CTL &= ~CTL_ENABLE;
	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_interrupt(BT_HANDLE hTimer) {
	hTimer->pRegs->CTL |= CTL_INT_ENABLE;
	return BT_ERR_NONE;
}

static BT_ERROR timer_disable_interrupt(BT_HANDLE hTimer) {
	hTimer->pRegs->CTL &= ~CTL_INT_ENABLE;
	return BT_ERR_NONE;
}

static BT_u32 timer_get_frequency(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(pError) {
		*pError = BT_ERR_NONE;
	}

	BT_u32 ulTickRate 	= timer_get_clock_rate(hTimer, pError);
	BT_u32 ulFrequency 	= ulTickRate / hTimer->pRegs->LOD;

	return ulFrequency;
}

static BT_ERROR timer_set_frequency(BT_HANDLE hTimer, BT_u32 ulFrequencyHz) {

	BT_ERROR Error;
	hTimer->pRegs->DIV = 0;
	BT_u32 ulTickRate 	= timer_get_clock_rate(hTimer, &Error);
	BT_u32 ulDivisor  	= ulTickRate / ulFrequencyHz;

	hTimer->pRegs->LOD = ulDivisor;

	return BT_ERR_NONE;
}

static BT_u32 timer_get_offset(BT_HANDLE hTimer, BT_ERROR *pError) {
	return hTimer->pRegs->VAL;
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
static BT_HANDLE timer_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_HANDLE hTimer = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hTimer) {
		Error = BT_ERR_NO_MEMORY;
		goto err_out;
	}

	const BT_RESOURCE *pResource;
	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hTimer->pRegs = (BCM2835_TIMER_REGS *) pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	BT_u32 ulIRQ = pResource->ulStart;

	Error = BT_RegisterInterrupt(ulIRQ, timer_irq_handler, hTimer);
	if(Error) {
		goto err_free_out;
	}

	hTimer->pRegs->CTL = 0x003E0002;

	BT_EnableInterrupt(ulIRQ);

	return hTimer;

err_free_out:
	BT_DestroyHandle(hTimer);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oIntegratedWdtDriver = {
	.name 		= "bcm2835,timer",
	.pfnProbe	= timer_probe,
};
