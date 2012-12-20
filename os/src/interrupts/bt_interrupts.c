#include <bitthunder.h>
#include <interrupts/bt_interrupts.h>

typedef struct _BT_INTERRUPT_CONTROLLERS {
	const BT_INTERRUPT_CONTROLLER  *pIntc;			///< Interrupt controller's device interface.
	BT_u32							ulBaseIRQ;		///< Base IRQ number.
	BT_u32							ulTotalIRQs;	///< Total IRQ lines.
} BT_INTERRUPT_CONTROLLERS;

static BT_INTERRUPT_CONTROLLERS g_oControllers[BT_CONFIG_MAX_INTERRUPT_CONTROLLERS];
static BT_u32 g_ulRegistered = 0;

BT_ERROR BT_RegisterInterruptController(BT_u32 ulBaseIRQ, BT_u32 ulTotalIRQs, const BT_INTERRUPT_CONTROLLER *pIntc) {

	BT_ERROR Error;

	if(g_ulRegistered >= BT_CONFIG_MAX_INTERRUPT_CONTROLLERS) {
		return -1;	/// Maximum controllers already registered.
	}

	g_oControllers[g_ulRegistered].pIntc 		= pIntc;
	g_oControllers[g_ulRegistered].ulBaseIRQ 	= ulBaseIRQ;
	g_oControllers[g_ulRegistered].ulTotalIRQs 	= ulTotalIRQs;

	g_ulRegistered++;

	Error = pIntc->pfnInitialise(ulTotalIRQs);
	if(Error) {
		g_ulRegistered--;
	}

	return Error;
}

static const BT_INTERRUPT_CONTROLLERS *getInterruptController(BT_u32 ulIRQ) {
	BT_u32 i;

	for(i=0; i < g_ulRegistered; i++) {
		BT_u32 min, max;
		min = g_oControllers[i].ulBaseIRQ;
		max = g_oControllers[i].ulBaseIRQ + g_oControllers[i].ulTotalIRQs;

		if(ulIRQ >= min && ulIRQ <= max) {
			return &g_oControllers[i];
		}
	}

	return NULL;
}

BT_ERROR BT_RegisterInterrupt(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	const BT_INTERRUPT_CONTROLLERS *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	return pIntc->pIntc->pfnRegister(ulIRQ-pIntc->ulBaseIRQ, pfnHandler, pParam);
}

BT_ERROR BT_UnregisterInterrupt(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {
	const BT_INTERRUPT_CONTROLLERS *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	return pIntc->pIntc->pfnUnregister(ulIRQ-pIntc->ulBaseIRQ, pfnHandler, pParam);
}

BT_ERROR BT_SetInterruptPriority(BT_u32 ulIRQ, BT_u32 ulPriority) {
	const BT_INTERRUPT_CONTROLLERS *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	return pIntc->pIntc->pfnSetPriority(ulIRQ-pIntc->ulBaseIRQ, ulPriority);
}

BT_u32 BT_GetInterruptPriority(BT_u32 ulIRQ, BT_ERROR *pError) {
	const BT_INTERRUPT_CONTROLLERS *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		if(pError) {
			*pError = -1;
		}
		return 0;
	}

	return pIntc->pIntc->pfnGetPriority(ulIRQ-pIntc->ulBaseIRQ, pError);
}

BT_ERROR BT_EnableInterrupt(BT_u32 ulIRQ) {
	const BT_INTERRUPT_CONTROLLERS *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}
	return pIntc->pIntc->pfnEnable(ulIRQ-pIntc->ulBaseIRQ);
}

BT_ERROR BT_DisableInterrupt(BT_u32 ulIRQ) {
	const BT_INTERRUPT_CONTROLLERS *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}
	return pIntc->pIntc->pfnDisable(ulIRQ-pIntc->ulBaseIRQ);
}

BT_ERROR BT_SetInterruptAffinity(BT_u32 ulIRQ, BT_u32 ulCPU) {
	const BT_INTERRUPT_CONTROLLERS *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	if(pIntc->pIntc->pfnSetAffinity) {
		return pIntc->pIntc->pfnSetAffinity(ulIRQ, ulCPU);
	}

	return -1;
}
