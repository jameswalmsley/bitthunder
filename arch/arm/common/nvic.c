/**
 *	ARM NVIC Driver for BitThunder
 *
 *
 **/
#include <bitthunder.h>
#include <arch/common/nvic.h>
#include <arch/common/scb.h>

BT_DEF_MODULE_NAME			("ARM NVIC")
BT_DEF_MODULE_DESCRIPTION	("Driver for ARM NVIC Interrupt Controller")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	NVIC_REGS *pRegs;
};

static BT_HANDLE 	g_hActiveHandle = NULL;

static BT_ERROR nvic_cleanup(BT_HANDLE hNVIC) {
	return BT_ERR_NONE;
}

static BT_ERROR nvic_register(BT_HANDLE hNVIC, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	return BT_ERR_NONE;
}

static BT_ERROR nvic_unregister(BT_HANDLE hNVIC, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	return BT_ERR_NONE;
}

static BT_ERROR nvic_setpriority(BT_HANDLE hNVIC, BT_u32 ulIRQ, BT_u32 ulPriority) {
	SCB_REGS * pSCB = SCB;
	NVIC_REGS * pNVIC = hNVIC->pRegs;

	if(ulIRQ < 16) {
		pSCB->SHPR[(ulIRQ & 0xF)-4] = ((ulPriority << (8 - 5)) & 0xff); } /* set Priority for Cortex-M3 System Interrupts */
	else {
	    pNVIC->IP[ulIRQ-16] = ((ulPriority << (8 - 5)) & 0xff);    }        /* set Priority for device specific Interrupts  */
	return BT_ERR_NONE;
}

static BT_u32 nvic_getpriority(BT_HANDLE hNVIC, BT_u32 ulIRQ, BT_ERROR *pError) {

	return BT_ERR_NONE;
}

static BT_ERROR nvic_enable(BT_HANDLE hNVIC, BT_u32 ulIRQ) {
	BT_u32 ulBank = (ulIRQ-16) / 32;
	BT_u32 ulBit = (ulIRQ-16) % 32;

	hNVIC->pRegs->ISE[ulBank] = 1 << ulBit;

	return BT_ERR_NONE;
}

static BT_ERROR nvic_disable(BT_HANDLE hNVIC, BT_u32 ulIRQ) {
	BT_u32 ulBank = (ulIRQ-16) / 32;
	BT_u32 ulBit = (ulIRQ-16) % 32;

	hNVIC->pRegs->ICE[ulBank] = (1 << ulBit);

	return BT_ERR_NONE;
}

static const BT_DEV_IF_IRQ oDeviceOps = {
	.pfnRegister 	= nvic_register,
	.pfnUnregister	= nvic_unregister,
	.pfnSetPriority	= nvic_setpriority,
	.pfnGetPriority = nvic_getpriority,
	.pfnEnable 		= nvic_enable,
	.pfnDisable		= nvic_disable,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_INTC,
	.unConfigIfs = {
		.pIRQIF = &oDeviceOps,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = nvic_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

static const BT_INTEGRATED_DRIVER oIntegratedDriver;

static BT_HANDLE nvic_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	if(g_hActiveHandle) {
		if(pError) {
			*pError = BT_ERR_GENERIC;
		}
		return NULL;
	}

	BT_HANDLE hNVIC = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hNVIC) {
		goto err_out;
	}

	const BT_RESOURCE *pResource;
	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto  err_free_chip;
	}

	hNVIC->pRegs = (NVIC_REGS *) pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_chip;
	}

	BT_u32 base, total;
 	base = pResource->ulStart;
	total = (pResource->ulEnd - pResource->ulStart) + 1;

	Error = BT_RegisterInterruptController(base, total, hNVIC);
	if(Error) {
		goto err_free_chip;
	}

	return hNVIC;

err_free_chip:
	BT_DestroyHandle(hNVIC);

	if(pError) {
		*pError = Error;
	}

err_out:
	return hNVIC;
}

BT_INTEGRATED_DRIVER_DEF oIntegratedDriver = {
	.name 		= "arm,common,nvic",
	.pfnProbe 	= nvic_probe,
};
