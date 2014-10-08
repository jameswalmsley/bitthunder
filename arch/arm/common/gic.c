/**
 *	ARM Generic Interrupt Controller Driver for BitThunder.
 *
 *	@author	James Walmsley
 *
 **/
#include <bitthunder.h>
#include <arch/common/gic.h>
#include <interrupts/bt_interrupts.h>
#include <bt_module.h>
#include <mm/bt_heap.h>
#include <string.h>

BT_DEF_MODULE_NAME				("Arm GIC")
BT_DEF_MODULE_DESCRIPTION		("Provides a complete interrupt controller interface for BitThunder")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

#ifndef BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS
#error "BT_CONFIG_ARCH_GIC_TOTAL_IRQS not defined in platform abstraction layer."
#endif

static BT_HANDLE				g_hActiveHandle = NULL;
static BT_INTERRUPT_VECTOR 		g_oVectorTable[BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS];
static BT_u32 					g_oVectorStats[BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS];

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	BT_u32						ulBaseIRQ;
	BT_u32						ulGicIRQs;
	volatile GICC_REGS 	   	   *pGICC;
	volatile GICD_REGS 	   	   *pGICD;
};

void BT_ARCH_ARM_GIC_IRQHandler() {
	BT_HANDLE hGic = g_hActiveHandle;

	BT_u32 ulStatus, ulIRQ;

	ulStatus = hGic->pGICC->IAR;
	ulIRQ = ulStatus & 0x03FF;

	while(ulIRQ < 1020) {
		g_oVectorStats[ulIRQ] += 1;
		ulIRQ 	   += hGic->ulBaseIRQ;		// Remap the IRQn into logical IRQ# space.
		g_oVectorTable[ulIRQ].pfnHandler(ulIRQ, g_oVectorTable[ulIRQ].pParam);
		hGic->pGICC->EOIR = ulIRQ;
		ulStatus 	= hGic->pGICC->IAR;		// receive the first interrupt from the IAR.
		ulIRQ 		= ulStatus & 0x03FF;	// Get the IRQ number.
	}
}

static BT_ERROR gic_stubhandler(BT_u32 ulIRQ, void *pParam) {
	return BT_ERR_NONE;
}

static BT_ERROR gic_cleanup(BT_HANDLE hGic) {
	return BT_ERR_NONE;
}

//#ifdef BT_CONFIG_ARCH_ARM_GIC_INIT_DISTRIBUTOR
static void gic_dist_init(BT_HANDLE hGic) {
	BT_u32 i;
	BT_u32 ulGicIRQs = hGic->ulGicIRQs;

	hGic->pGICD->CTLR = 0;

	// Set all global interrupts to be level triggered (active low).
	for(i = 32; i < ulGicIRQs; i += 16) {
		hGic->pGICD->ICFGR[i/16] = 0;
	}

	BT_u32 ulCPUMask = 		(1 << BT_GetCoreID());
	ulCPUMask |= ulCPUMask 	<< 8;
	ulCPUMask |= ulCPUMask 	<< 16;

	for(i = 32; i < ulGicIRQs; i += 4) {
		hGic->pGICD->ITARGETSR[i/4] |= ulCPUMask;
		hGic->pGICD->IPRIORITYR[i/4] = 0xA0A0A0A0;
	}

	for(i = 32; i < ulGicIRQs; i += 32) {
		hGic->pGICD->ICENABLER[i/32] = 0xFFFFFFFF;
	}

	hGic->pGICD->CTLR = 1;
}
//#endif

static void gic_cpu_init(BT_HANDLE hGic) {

	BT_u32 i;

	hGic->pGICD->ICENABLER[0] = 0xFFFF0000;
	hGic->pGICD->ISENABLER[0] = 0x0000FFFF;

	for(i = 0; i < 32; i += 4) {
		hGic->pGICD->IPRIORITYR[i/4] = 0xA0A0A0A0;
	}

	hGic->pGICC->PMR 	= 0xF0;
	hGic->pGICC->CTLR 	= 1;
}

static BT_ERROR gic_register(BT_HANDLE hGic, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	if(g_oVectorTable[ulIRQ].pfnHandler != gic_stubhandler) {
		return BT_ERR_GENERIC;
	}

	g_oVectorTable[ulIRQ].pfnHandler 	= pfnHandler;
	g_oVectorTable[ulIRQ].pParam 		= pParam;

	return BT_ERR_NONE;
}

static BT_ERROR gic_set_label(BT_HANDLE hGic, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam, const BT_i8 *label) {
	if(g_oVectorTable[ulIRQ].pfnHandler != pfnHandler) {
		return BT_ERR_GENERIC;
	}

	strncpy(g_oVectorTable[ulIRQ].label, label, BT_INTERRUPT_MAX_LABEL);
	g_oVectorTable[ulIRQ].label[BT_INTERRUPT_MAX_LABEL-1] = '\0';

	return BT_ERR_NONE;
}

static const BT_i8 *gic_get_label(BT_HANDLE hGic, BT_u32 ulIRQ) {
	return g_oVectorTable[ulIRQ].label;
}

static BT_BOOL gic_registered(BT_HANDLE hGic, BT_u32 ulIRQ) {
	if(g_oVectorTable[ulIRQ].pfnHandler != gic_stubhandler) {
		return BT_TRUE;
	}

	return BT_FALSE;
}

static BT_ERROR gic_unregister(BT_HANDLE hGic, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	if(g_oVectorTable[ulIRQ].pfnHandler != pfnHandler) {
		return BT_ERR_GENERIC;
	}

	g_oVectorTable[ulIRQ].pfnHandler 	= gic_stubhandler;
	g_oVectorTable[ulIRQ].pParam 		= NULL;

	return BT_ERR_NONE;
}

static BT_ERROR gic_setpriority(BT_HANDLE hGic, BT_u32 ulIRQ, BT_u32 ulPriority) {
	return BT_ERR_NONE;
}

static BT_u32 gic_getpriority(BT_HANDLE hGic, BT_u32 ulIRQ, BT_ERROR *pError) {
	return 0;
}

static BT_ERROR gic_enable(BT_HANDLE hGic, BT_u32 ulIRQ) {
	BT_u32 ulMask = 1 << (ulIRQ %32);
	hGic->pGICD->ISENABLER[ulIRQ/32] = ulMask;
	return BT_ERR_NONE;
}

static BT_ERROR gic_disable(BT_HANDLE hGic, BT_u32 ulIRQ) {
	BT_u32 ulMask = 1 << (ulIRQ % 32);
	hGic->pGICD->ICENABLER[ulIRQ/32] = ulMask;
	return BT_ERR_NONE;
}

static BT_ERROR gic_setaffinity(BT_HANDLE hGic, BT_u32 ulIRQ, BT_u32 ulCPU, BT_BOOL bReceive) {

	BT_u32 ulBank = ulIRQ / 4;
	BT_u32 ulLine = ulIRQ % 4;

	if(ulCPU > 7) {
		return BT_ERR_GENERIC;	// Only accept CPU ID's in range(0..7).
	}

	if(bReceive) {
		bReceive = 1;
	}

	hGic->pGICD->ITARGETSR[ulBank] &= ~(1 << ulCPU) << (8 * ulLine);
	hGic->pGICD->ITARGETSR[ulBank] |= (bReceive << ulCPU) << (8 * ulLine);

	return BT_ERR_NONE;
}

#define portUNMASK_VALUE				( 0xFFUL )
#define configMAX_API_CALL_INTERRUPT_PRIORITY	18

/*#if configUNIQUE_INTERRUPT_PRIORITIES == 16
	#define portPRIORITY_SHIFT 4
	#define portMAX_BINARY_POINT_VALUE	3
#elif configUNIQUE_INTERRUPT_PRIORITIES == 32*/
	#define portPRIORITY_SHIFT 3
	#define portMAX_BINARY_POINT_VALUE	2
/*#elif configUNIQUE_INTERRUPT_PRIORITIES == 64
	#define portPRIORITY_SHIFT 2
	#define portMAX_BINARY_POINT_VALUE	1
#elif configUNIQUE_INTERRUPT_PRIORITIES == 128
	#define portPRIORITY_SHIFT 1
	#define portMAX_BINARY_POINT_VALUE	0
#elif configUNIQUE_INTERRUPT_PRIORITIES == 256
	#define portPRIORITY_SHIFT 0
	#define portMAX_BINARY_POINT_VALUE	0
#else
	#error Invalid configUNIQUE_INTERRUPT_PRIORITIES setting.  configUNIQUE_INTERRUPT_PRIORITIES must be set to the number of unique priorities implemented by the target hardware
#endif*/

#define portCPU_IRQ_DISABLE()												\
	__asm volatile("cpsid i");												\
	__asm volatile("dsb");													\
	__asm volatile("isb");													\

#define portCPU_IRQ_ENABLE()												\
	__asm volatile("cpsie i");												\
	__asm volatile("dsb");													\
	__asm volatile("isb");													\


static BT_ERROR gic_enable_interrupts(BT_HANDLE hGic) {

	portCPU_IRQ_ENABLE();

	/*hGic->pGICD->CTLR |= 1;
	hGic->pGICC->CTLR |= 1;*/
	return BT_ERR_NONE;
}


static BT_ERROR gic_disable_interrupts(BT_HANDLE hGic) {
	/*hGic->pGICC->CTLR &= ~1;*/

	portCPU_IRQ_DISABLE();

	return BT_ERR_NONE;
}

static BT_u32 gic_mask_interrupts(BT_HANDLE hGic) {
	BT_u32 ulReturn;

	portCPU_IRQ_DISABLE();
	{
		if(hGic->pGICC->PMR == (BT_u32) (configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT)) {
			ulReturn = 1;
		} else {
			ulReturn = 0;
			hGic->pGICC->PMR = (BT_u32) (configMAX_API_CALL_INTERRUPT_PRIORITY << portPRIORITY_SHIFT);
			__asm volatile("dsb");
			__asm volatile("isb");
		}
	}
	portCPU_IRQ_ENABLE();

	return ulReturn;
}

static BT_ERROR gic_unmask_interrupts(BT_HANDLE hGic, BT_u32 ulNewMaskValue) {
	if(ulNewMaskValue == 0) {
		portCPU_IRQ_DISABLE();
		{
			hGic->pGICC->PMR = portUNMASK_VALUE;
			__asm volatile("dsb");
			__asm volatile("isb");
		}
		portCPU_IRQ_ENABLE();
	}
}

static BT_u32 gic_get_count(BT_HANDLE hGic, BT_u32 ulIRQ) {
	return g_oVectorStats[ulIRQ];
}

static const BT_DEV_IF_IRQ oDeviceOps = {
	.pfnRegister			= gic_register,
	.pfnSetLabel			= gic_set_label,
	.pfnGetLabel			= gic_get_label,
	.pfnRegistered			= gic_registered,
	.pfnUnregister			= gic_unregister,
	.pfnSetPriority			= gic_setpriority,
	.pfnGetPriority			= gic_getpriority,
	.pfnEnable				= gic_enable,
	.pfnDisable				= gic_disable,
	.pfnSetAffinity			= gic_setaffinity,						///< An option interface, GIC could implement this.
	.pfnEnableInterrupts	= gic_enable_interrupts,
	.pfnDisableInterrupts	= gic_disable_interrupts,
	.pfnMaskInterrupts		= gic_mask_interrupts,
	.pfnUnmaskInterrupts 	= gic_unmask_interrupts,
	.pfnGetCount			= gic_get_count,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_INTC,
	.unConfigIfs = {
		.pIRQIF = &oDeviceOps,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = gic_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

static const BT_INTEGRATED_DRIVER oIntegratedDriver;

static BT_HANDLE gic_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	if(g_hActiveHandle) {
		if(pError) {
			*pError = BT_ERR_GENERIC;
		}
		return NULL;
	}

	BT_HANDLE hGic = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hGic) {
		goto err_out;
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto  err_free_chip;
	}

	hGic->pGICC = (GICC_REGS *) bt_ioremap((void *)pResource->ulStart, BT_SIZE_4K);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 1);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_chip;
	}

	hGic->pGICD = (GICD_REGS *) bt_ioremap((void *)pResource->ulStart, BT_SIZE_4K);

	// Set private members.

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_chip;
	}

	BT_u32 base 	= pResource->ulStart;
	BT_u32 total 	= (pResource->ulEnd - pResource->ulStart) + 1;

	hGic->ulBaseIRQ = base;

	BT_u32 i;
	for(i = 0; i < total; i++) {
		g_oVectorTable[i].pfnHandler 	= gic_stubhandler;
		g_oVectorTable[i].pParam 	 	= NULL;
	}

	Error = BT_RegisterInterruptController(base, total, hGic);
	if(Error) {
		goto err_free_chip;
	}

	g_hActiveHandle = hGic;

	// Enable the Distributor and CPU interfaces.

	BT_u32 ulGicIRQs = (hGic->pGICD->TYPER & 0x1F);
	ulGicIRQs = (ulGicIRQs + 1) * 32;

	/*
	 *	GIC can support a mximum 1020 IRQs.
	 */
	if(ulGicIRQs > 1020) {
		ulGicIRQs = 1020;
	}

	hGic->ulGicIRQs = ulGicIRQs;

//#ifdef BT_CONFIG_ARCH_ARM_GIC_INIT_DISTRIBUTOR
	gic_dist_init(hGic);
//#endif
	gic_cpu_init(hGic);

	return hGic;

err_free_chip:
	BT_DestroyHandle(hGic);

	if(pError) {
		*pError = Error;
	}

err_out:
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oIntegratedDriver = {
	.name		= "arm,common,gic",
	.pfnProbe 	= gic_probe,
};
