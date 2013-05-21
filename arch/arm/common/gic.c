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

BT_DEF_MODULE_NAME				("Arm GIC")
BT_DEF_MODULE_DESCRIPTION		("Provides a complete interrupt controller interface for BitThunder")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

#ifndef BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS
#error "BT_CONFIG_ARCH_GIC_TOTAL_IRQS not defined in platform abstraction layer."
#endif

static BT_HANDLE				g_hActiveHandle = NULL;
static BT_INTERRUPT_VECTOR 		g_oVectorTable[BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS];

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
	ulIRQ = ulStatus & 0x01FF;

	while(ulIRQ < 1020) {
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

static void gic_dist_init(BT_HANDLE hGic) {
	BT_u32 i;
	BT_u32 ulGicIRQs = hGic->ulGicIRQs;

	hGic->pGICD->CTLR = 0;

	// Set all global interrupts to be level triggered (active low).
	for(i = 32; i < ulGicIRQs; i += 16) {
		hGic->pGICD->ICFGR[i/16] = 0;
	}

	BT_u32 ulCPUMask = 1 	<< 0;
	ulCPUMask |= ulCPUMask 	<< 8;
	ulCPUMask |= ulCPUMask 	<< 16;

	for(i = 32; i < ulGicIRQs; i += 4) {
		hGic->pGICD->ITARGETSR[i/4] = ulCPUMask;
		hGic->pGICD->IPRIORITYR[i/4] = 0xA0A0A0A0;
	}

	for(i = 32; i < ulGicIRQs; i += 32) {
		hGic->pGICD->ICENABLER[i/32] = 0xFFFFFFFF;
	}

	hGic->pGICD->CTLR = 1;
}

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

static BT_ERROR gic_enable_interrupts(BT_HANDLE hGic) {

	__asm volatile (
				"STMDB	SP!, {R0}		\n\t"	/* Push R0.					*/
				"MRS	R0, CPSR		\n\t"	/* Get CPSR.				*/
				"BIC	R0, R0, #0xC0	\n\t"	/* Enable IRQ, FIQ.			*/
				"MSR	CPSR, R0		\n\t"	/* Write back modified value.*/
				"LDMIA	SP!, {R0}" );			/* Pop R0.					*/

	hGic->pGICC->CTLR |= 1;
	return BT_ERR_NONE;
}


static BT_ERROR gic_disable_interrupts(BT_HANDLE hGic) {
	hGic->pGICC->CTLR &= ~1;

	__asm volatile (
		"STMDB	SP!, {R0}			\n\t"	/* Push R0.						*/
		"MRS	R0, CPSR			\n\t"	/* Get CPSR.					*/
		"ORR	R0, R0, #0xC0		\n\t"	/* Disable IRQ, FIQ.			*/
		"MSR	CPSR, R0			\n\t"	/* Write back modified value.	*/
		"LDMIA	SP!, {R0}" );				/* Pop R0.						*/

	return BT_ERR_NONE;
}

static const BT_DEV_IF_IRQ oDeviceOps = {
	.pfnRegister			= gic_register,
	.pfnUnregister			= gic_unregister,
	.pfnSetPriority			= gic_setpriority,
	.pfnGetPriority			= gic_getpriority,
	.pfnEnable				= gic_enable,
	.pfnDisable				= gic_disable,
	.pfnSetAffinity			= gic_setaffinity,						///< An option interface, GIC could implement this.
	.pfnEnableInterrupts	= gic_enable_interrupts,
	.pfnDisableInterrupts	= gic_disable_interrupts,
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

	hGic->pGICC = (GICC_REGS *) pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 1);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_chip;
	}

	hGic->pGICD = (GICD_REGS *) pResource->ulStart;

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

	BT_u32 ulGicIRQs = hGic->pGICD->TYPER;
	ulGicIRQs = (ulGicIRQs + 1) * 32;

	/*
	 *	GIC can support a mximum 1020 IRQs.
	 */
	if(ulGicIRQs > 1020) {
		ulGicIRQs = 1020;
	}

	hGic->ulGicIRQs = ulGicIRQs;

	gic_dist_init(hGic);
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
