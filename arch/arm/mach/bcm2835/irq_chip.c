/**
 *	Implementation of the Interrupt controller on the BCM2835 SOC.
 **/

/**
 *	Raspberry Pi Interrupt Controller Driver for BitThunder
 *
 *	@author	James Walmsley
 **/
#include <bitthunder.h>
#include <interrupts/bt_interrupts.h>
#include <bt_module.h>
#include <mm/bt_heap.h>
#include "irq_chip.h"

BT_DEF_MODULE_NAME				("BCM2835 Interrupt Controller")
BT_DEF_MODULE_DESCRIPTION		("Provides a complete interrupt controller interface for BitThunder")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

static BT_HANDLE				g_hActiveHandle = NULL;
static BT_INTERRUPT_VECTOR 		g_oVectorTable[BT_CONFIG_MACH_BCM2835_TOTAL_IRQ];

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	BT_u32						ulBaseIRQ;
	BCM2835_INTC_REGS		   *pRegs;
};

#define clz(a) \
 ({ BT_u32 __value, __arg = (a); \
     asm ("clz\t%0, %1": "=r" (__value): "r" (__arg)); \
     __value; })


void BT_ARCH_ARM_MACH_BCM2835_IRQHandler() {

	BT_HANDLE hIRQ = g_hActiveHandle;

	register BT_u32 ulMaskedStatus;
	register BT_u32 irqNumber;
	register BT_u32 tmp;

	ulMaskedStatus = hIRQ->pRegs->IRQBasic;
	tmp = ulMaskedStatus & 0x00000300;			// Check if anything pending in pr1/pr2.

	if(ulMaskedStatus & ~0xFFFFF300) {			// Note how we mask out the GPU interrupt Aliases.
		irqNumber = 64 + 31;						// Shifting the basic ARM IRQs to be IRQ# 64 +
		goto emit_interrupt;
	}

	if(tmp & 0x100) {
		ulMaskedStatus = hIRQ->pRegs->Pending1;
		irqNumber = 0 + 31;
		// Clear the interrupts also available in basic IRQ pending reg.
		//ulMaskedStatus &= ~((1 << 7) | (1 << 9) | (1 << 10) | (1 << 18) | (1 << 19));
		if(ulMaskedStatus) {
			goto emit_interrupt;
		}
	}

	if(tmp & 0x200) {
		ulMaskedStatus + hIRQ->pRegs->Pending2;
		irqNumber = 32 + 31;
		// Don't clear the interrupts in the basic pending, simply allow them to processed here!
		if(ulMaskedStatus) {
			goto emit_interrupt;
		}
	}

	return;

emit_interrupt:

	tmp = ulMaskedStatus - 1;
	ulMaskedStatus = ulMaskedStatus ^ tmp;

	unsigned long lz = clz(ulMaskedStatus);

	if(g_oVectorTable[irqNumber-lz].pfnHandler) {
		g_oVectorTable[irqNumber-lz].pfnHandler(irqNumber-lz, g_oVectorTable[irqNumber-lz].pParam);
	}
}

static BT_ERROR irq_chip_stubhandler(BT_u32 ulIRQ, void *pParam) {
	return BT_ERR_NONE;
}

static BT_ERROR irq_chip_cleanup(BT_HANDLE hIRQ) {
	return BT_ERR_NONE;
}

static BT_ERROR irq_chip_register(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	if(g_oVectorTable[ulIRQ].pfnHandler != irq_chip_stubhandler) {
		return BT_ERR_GENERIC;
	}

	g_oVectorTable[ulIRQ].pfnHandler 	= pfnHandler;
	g_oVectorTable[ulIRQ].pParam 		= pParam;

	return BT_ERR_NONE;
}

static BT_ERROR irq_chip_unregister(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	if(g_oVectorTable[ulIRQ].pfnHandler != pfnHandler) {
		return BT_ERR_GENERIC;
	}

	g_oVectorTable[ulIRQ].pfnHandler 	= irq_chip_stubhandler;
	g_oVectorTable[ulIRQ].pParam 		= NULL;

	return BT_ERR_NONE;
}

static BT_ERROR irq_chip_setpriority(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_u32 ulPriority) {
	return BT_ERR_NONE;
}

static BT_u32 irq_chip_getpriority(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_ERROR *pError) {
	return 0;
}

static BT_ERROR irq_chip_enable(BT_HANDLE hIRQ, BT_u32 ulIRQ) {

	BT_u32	ulTMP;

	ulTMP = hIRQ->pRegs->EnableBasic;

	if(ulIRQ >= 64 && ulIRQ <= 72) {	// Basic IRQ enables
		hIRQ->pRegs->EnableBasic = 1 << (ulIRQ - 64);
	}

	ulTMP = hIRQ->pRegs->EnableBasic;

	// Otherwise its a GPU interrupt, and we're not supporting those...yet!

	return BT_ERR_NONE;
}

static BT_ERROR irq_chip_disable(BT_HANDLE hIRQ, BT_u32 ulIRQ) {
	if(ulIRQ >= 64 && ulIRQ <= 72) {
		hIRQ->pRegs->DisableBasic = 1 << (ulIRQ - 64);
	}

	// I'm currently only supporting the basic IRQs.
	return BT_ERR_NONE;
}

static const BT_DEV_IF_IRQ oDeviceOps = {
	.pfnRegister		= irq_chip_register,
	.pfnUnregister		= irq_chip_unregister,
	.pfnSetPriority		= irq_chip_setpriority,
	.pfnGetPriority		= irq_chip_getpriority,
	.pfnEnable			= irq_chip_enable,
	.pfnDisable			= irq_chip_disable,
	.pfnSetAffinity		= NULL,						///< An optional interface,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_INTC,
	.unConfigIfs = {
		.pIRQIF = &oDeviceOps,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = irq_chip_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

static const BT_INTEGRATED_DRIVER oIntegratedDriver;

static BT_HANDLE irq_chip_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	if(g_hActiveHandle) {
		if(pError) {
			*pError = BT_ERR_GENERIC;
		}
		return NULL;
	}

	BT_HANDLE hIRQ = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hIRQ) {
		goto err_out;
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto  err_free_chip;
	}

	hIRQ->pRegs = (BCM2835_INTC_REGS *) pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_chip;
	}

	BT_u32 base 	= pResource->ulStart;
	BT_u32 total 	= (pResource->ulEnd - pResource->ulStart) + 1;

	hIRQ->ulBaseIRQ = base;

	BT_u32 i;
	for(i = 0; i < total; i++) {
		g_oVectorTable[i].pfnHandler 	= irq_chip_stubhandler;
		g_oVectorTable[i].pParam 	 	= NULL;
	}

	hIRQ->pRegs->DisableBasic = 0xFFFFFFFF;

	Error = BT_RegisterInterruptController(base, total, hIRQ);
	if(Error) {
		goto err_free_chip;
	}

	g_hActiveHandle = hIRQ;

	return hIRQ;

err_free_chip:
	BT_DestroyHandle(hIRQ);

	if(pError) {
		*pError = Error;
	}

err_out:
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF oIntegratedDriver = {
	.name		= "bcm2835,intc",
	.pfnProbe 	= irq_chip_probe,
};
