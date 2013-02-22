/**
 *	ARM Cortex-Mx Systick Peripheral Driver
 *
 **/

#include <bitthunder.h>
#include "systick.h"

BT_DEF_MODULE_NAME					("ARM Cortex-Mx Systick Driver")
BT_DEF_MODULE_DESCRIPTION			("Timer driver implementation for Cortex-M3 Systick")
BT_DEF_MODULE_AUTHOR				("James Walmsley")
BT_DEF_MODULE_EMAIL					("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 				h;
	volatile SYSTICK_REGS  		   *pRegs;
	void *pParam;
};

static BT_ERROR timer_cleanup(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}

static BT_u32 timer_get_clock_rate(BT_HANDLE hTimer, BT_ERROR *pError) {
	BT_u32 InputClk = BT_GetCpuClockFrequency();
	if(!(hTimer->pRegs->CTRL & SYSTICK_CTRL_CLKSOURCE)) {
		return InputClk / 8;
	}
	return InputClk;
}

static BT_ERROR timer_register_interrupt(BT_HANDLE hTimer, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	//hTimer->pfnHandler 	= pfnHandler;
	//hTimer->pParam 		= pParam;
	return BT_ERR_NONE;
}

static BT_ERROR timer_start(BT_HANDLE hTimer) {
	hTimer->pRegs->CTRL |= SYSTICK_CTRL_ENABLE;
	return BT_ERR_NONE;
}

static BT_ERROR timer_stop(BT_HANDLE hTimer) {
	hTimer->pRegs->CTRL &= ~SYSTICK_CTRL_ENABLE;
	return BT_ERR_NONE;
}

static BT_ERROR timer_enable_interrupt(BT_HANDLE hTimer) {
	hTimer->pRegs->CTRL |= SYSTICK_CTRL_TICKINT;
	return BT_ERR_NONE;
}

static BT_ERROR timer_disable_interrupt(BT_HANDLE hTimer) {
	hTimer->pRegs->CTRL &= ~SYSTICK_CTRL_TICKINT;
	return BT_ERR_NONE;
}

static BT_u32 timer_get_frequency(BT_HANDLE hTimer, BT_ERROR *pError) {
	if(pError) {
		*pError = BT_ERR_NONE;
	}

	BT_u32 ulTickRate 	= timer_get_clock_rate(hTimer, pError);
	BT_u32 ulFrequency 	= ulTickRate / hTimer->pRegs->LOAD;

	return ulFrequency;
}

static BT_ERROR timer_set_frequency(BT_HANDLE hTimer, BT_u32 ulFrequencyHz) {

	BT_ERROR Error;
	BT_u32 ulTickRate 	= timer_get_clock_rate(hTimer, &Error);
	BT_u32 ulDivisor  	= ulTickRate / ulFrequencyHz;

	hTimer->pRegs->LOAD = ulDivisor;

	return BT_ERR_NONE;
}

static BT_u32 timer_get_offset(BT_HANDLE hTimer, BT_ERROR *pError) {
	return hTimer->pRegs->VALUE;
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

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hTimer->pRegs = (SYSTICK_REGS *) pResource->ulStart;
	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	/*
	 *	In normal timer drivers we might register the interrupt here.
	 *	By default we want the linker to patch our vector table for us
	 *	and therefore its up to the Kernel to override the
	 *	correct symbol name... i.e. BT_NVIC_SysTick_Handler
	 *
	 */

	return hTimer;

err_free_out:
	BT_kFree(hTimer);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oIntegratedWdtDriver = {
	.name 		= "arm,cortex-mx,systick",
	.pfnProbe	= timer_probe,
};
