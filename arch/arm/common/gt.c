/**
 *	ARM Global Timer for mpcore processors.
 *
 **/

#include <bitthunder.h>
#include "gt.h"

BT_DEF_MODULE_NAME					("ARM Cortex-A9 Global Timer")
BT_DEF_MODULE_DESCRIPTION			("Timer driver implementation for Cortex-A9 Global timer")
BT_DEF_MODULE_AUTHOR				("James Walmsley")
BT_DEF_MODULE_EMAIL					("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	volatile GT_REGS *pRegs;
};

BT_ERROR gt_cleanup(BT_HANDLE hTimer) {
	return BT_ERR_NONE;
}


BT_u32 gt_rate(BT_HANDLE hTimer, BT_ERROR *pError) {
	BT_u32 InputClk = BT_GetCpuClockFrequency();
	InputClk /= 2;
	BT_u32 prescaler = (hTimer->pRegs->control >> 8) & 0xFF;
	InputClk /= prescaler+1;
	return InputClk >> 5;
}

BT_u64 gt_value(BT_HANDLE hTimer, BT_ERROR *pError) {
	return ((BT_u64) (hTimer->pRegs->count_0 | (BT_u64) hTimer->pRegs->count_1 << 32)) >> 5;
}

static const BT_DEV_IF_GTIMER gt_ops = {
	.pfnGetClockRate = gt_rate,
	.pfnGetValue = gt_value,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_GTIMER,
	.unConfigIfs = {
		.pGTimerIF = &gt_ops,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = gt_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};


static BT_HANDLE gt_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_HANDLE hTimer = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hTimer) {
		return NULL;
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		goto err_free_out;
	}

	hTimer->pRegs = (GT_REGS *) bt_ioremap((void *) pResource->ulStart, sizeof(GT_REGS));

	BT_SetGlobalTimerHandle(hTimer);

	return hTimer;

err_free_out:

	BT_DestroyHandle(hTimer);

	return NULL;
}


BT_INTEGRATED_DRIVER_DEF gt_driver = {
	.name = "arm,cortex-a9,gt",
	.pfnProbe = gt_probe,
};

static const BT_RESOURCE gt_resources[] = {
	{
		.ulStart			= BT_CONFIG_ARCH_ARM_CORTEX_A9_MPCORE_BASE + 0x0200,
		.ulEnd				= BT_CONFIG_ARCH_ARM_CORTEX_A9_MPCORE_BASE + 0x0200 + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
};

BT_INTEGRATED_DEVICE_DEF oZynq_cpu_timer_device = {
	.name					= "arm,cortex-a9,gt",
	.ulTotalResources		= BT_ARRAY_SIZE(gt_resources),
	.pResources				= gt_resources,
};
