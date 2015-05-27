#include <bitthunder.h>
#include <interrupts/bt_interrupts.h>

typedef struct _BT_INTERRUPT_CONTROLLER {
	BT_HANDLE						hIRQ;
	BT_u32							ulBaseIRQ;		///< Base IRQ number.
	BT_u32							ulTotalIRQs;	///< Total IRQ lines.
} BT_INTERRUPT_CONTROLLER;

static BT_INTERRUPT_CONTROLLER g_oController = {NULL};

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

BT_ERROR BT_RegisterInterruptController(BT_u32 ulBaseIRQ, BT_u32 ulTotalIRQs, BT_HANDLE hIRQ) {

	BT_ERROR Error = BT_ERR_NONE;

	if(!BT_IF_DEVICE(hIRQ) || BT_IF_DEVICE_TYPE(hIRQ) != BT_DEV_IF_T_INTC) {
		return BT_ERR_GENERIC;
	}

	if(g_oController.hIRQ) {
		return -1;	/// Maximum controllers already registered.
	}

	g_oController.hIRQ       	= hIRQ;

	// Ensure there is no conflict.
	g_oController.ulBaseIRQ 	= ulBaseIRQ;
	g_oController.ulTotalIRQs 	= ulTotalIRQs;

	return Error;
}
BT_EXPORT_SYMBOL(BT_RegisterInterruptController);

BT_ERROR BT_CleanupInterruptControllers() {
	BT_u32 i;
	BT_ERROR Error;

	BT_CloseHandle(g_oController.hIRQ);
	g_oController.hIRQ = NULL;

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_CleanupInterruptControllers);

static BT_INTERRUPT_CONTROLLER *getInterruptController(BT_u32 ulIRQ) {
	return &g_oController;
}

BT_ERROR BT_RegisterInterrupt(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnRegister(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, pfnHandler, pParam);
}
BT_EXPORT_SYMBOL(BT_RegisterInterrupt);

BT_ERROR BT_SetInterruptLabel(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam, const BT_i8 *label) {
	BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return BT_ERR_GENERIC;
	}

	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnSetLabel(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, pfnHandler, pParam, label);
}
BT_EXPORT_SYMBOL(BT_SetInterruptLabel);

const BT_i8 *BT_GetInterruptLabel(BT_u32 ulIRQ) {
	BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return NULL;
	}

	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnGetLabel(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ);
}
BT_EXPORT_SYMBOL(BT_GetInterruptLabel);

BT_BOOL BT_InterruptRegistered(BT_u32 ulIRQ) {
	BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return BT_FALSE;
	}

	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnRegistered(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ);
}
BT_EXPORT_SYMBOL(BT_InterruptRegistered);

BT_u32 BT_GetInterruptCount(BT_u32 ulIRQ) {
	BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return 0;
	}

	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnGetCount(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ);
}
BT_EXPORT_SYMBOL(BT_GetInterruptCount);

BT_ERROR BT_UnregisterInterrupt(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {

	BT_ERROR Error;

	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	BT_DisableInterrupt(ulIRQ);

	Error = pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnUnregister(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, pfnHandler, pParam);

	return Error;
}
BT_EXPORT_SYMBOL(BT_UnregisterInterrupt);

BT_ERROR BT_SetInterruptPriority(BT_u32 ulIRQ, BT_u32 ulPriority) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnSetPriority(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, ulPriority);
}
BT_EXPORT_SYMBOL(BT_SetInterruptPriority);

BT_s32 BT_GetActiveInterrupt(BT_ERROR *pError) {

	const BT_INTERRUPT_CONTROLLER *pIntc = &g_oController;

	BT_s32 slActive = pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnGetActiveInterrupt(pIntc->hIRQ, pError);
	if (slActive >= 0) {
		return g_oController.ulBaseIRQ + slActive;
	}
	return -1;
}
BT_EXPORT_SYMBOL(BT_GetActiveInterrupt);

BT_u32 BT_GetInterruptPriority(BT_u32 ulIRQ, BT_ERROR *pError) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		if(pError) {
			*pError = -1;
		}
		return 0;
	}

	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnGetPriority(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, pError);
}
BT_EXPORT_SYMBOL(BT_GetInterruptPriority);

BT_ERROR BT_EnableInterrupt(BT_u32 ulIRQ) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}
	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnEnable(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ);
}
BT_EXPORT_SYMBOL(BT_EnableInterrupt);

BT_ERROR BT_DisableInterrupt(BT_u32 ulIRQ) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}
	return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnDisable(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ);
}
BT_EXPORT_SYMBOL(BT_DisableInterrupt);

BT_ERROR BT_SetInterruptAffinity(BT_u32 ulIRQ, BT_u32 ulCPU, BT_BOOL bReceive) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	if(pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnSetAffinity) {
		return pIntc->BT_IF_IRQ_OPS(hIRQ)->pfnSetAffinity(pIntc->hIRQ, ulIRQ, ulCPU, bReceive);
	}

	return -1;
}
BT_EXPORT_SYMBOL(BT_SetInterruptAffinity);

BT_ERROR BT_EnableInterrupts() {
	if(g_oController.BT_IF_IRQ_OPS(hIRQ)->pfnEnableInterrupts) {
		g_oController.BT_IF_IRQ_OPS(hIRQ)->pfnEnableInterrupts(g_oController.hIRQ);
	}
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_EnableInterrupts);

BT_ERROR BT_DisableInterrupts() {
	g_oController.BT_IF_IRQ_OPS(hIRQ)->pfnDisableInterrupts(g_oController.hIRQ);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DisableInterrupts);

BT_u32 BT_MaskInterrupts() {
	if(g_oController.hIRQ && g_oController.BT_IF_IRQ_OPS(hIRQ)->pfnMaskInterrupts) {
		return g_oController.BT_IF_IRQ_OPS(hIRQ)->pfnMaskInterrupts(g_oController.hIRQ);
	}
	return 0;
}
BT_EXPORT_SYMBOL(BT_MaskInterrupts);

BT_ERROR BT_UnmaskInterrupts(BT_u32 ulNewMaskValue) {
	if(g_oController.hIRQ && g_oController.BT_IF_IRQ_OPS(hIRQ)->pfnUnmaskInterrupts) {
		g_oController.BT_IF_IRQ_OPS(hIRQ)->pfnUnmaskInterrupts(g_oController.hIRQ, ulNewMaskValue);
	}
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_UnmaskInterrupts);
