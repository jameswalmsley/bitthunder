#include <bitthunder.h>
#include <interrupts/bt_interrupts.h>

typedef struct _BT_INTERRUPT_CONTROLLER {
	BT_HANDLE						hIRQ;
	BT_u32							ulBaseIRQ;		///< Base IRQ number.
	BT_u32							ulTotalIRQs;	///< Total IRQ lines.
} BT_INTERRUPT_CONTROLLER;

static BT_INTERRUPT_CONTROLLER g_oControllers[BT_CONFIG_MAX_INTERRUPT_CONTROLLERS];
static BT_u32 g_ulRegistered = 0;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

void BT_IRQHandler(BT_u32 ulIRQ) {

}

BT_ERROR BT_RegisterInterruptController(BT_u32 ulBaseIRQ, BT_u32 ulTotalIRQs, BT_HANDLE hIRQ) {

	BT_ERROR Error = BT_ERR_NONE;

	if(g_ulRegistered >= BT_CONFIG_MAX_INTERRUPT_CONTROLLERS) {
		return -1;	/// Maximum controllers already registered.
	}

	g_oControllers[g_ulRegistered].hIRQ       	= hIRQ;

	// Ensure there is no conflict.
	g_oControllers[g_ulRegistered].ulBaseIRQ 	= ulBaseIRQ;
	g_oControllers[g_ulRegistered].ulTotalIRQs 	= ulTotalIRQs;

	g_ulRegistered++;

	return Error;
}

BT_ERROR BT_CleanupInterruptControllers() {
	BT_u32 i;
	BT_ERROR Error;

	for(i=0; i < g_ulRegistered; i++) {

		BT_CloseHandle(g_oControllers[i].hIRQ);
		if(Error) {
			//BT_kPrintf("Error cleaning %s", );
		}
	}

	return BT_ERR_NONE;
}


static BT_INTERRUPT_CONTROLLER *getInterruptController(BT_u32 ulIRQ) {
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
	BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	return pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnRegister(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, pfnHandler, pParam);
}

BT_ERROR BT_UnregisterInterrupt(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam) {

	BT_ERROR Error;

	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	BT_DisableInterrupt(ulIRQ);

	Error = pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnUnregister(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, pfnHandler, pParam);

	return Error;
}

BT_ERROR BT_SetInterruptPriority(BT_u32 ulIRQ, BT_u32 ulPriority) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	return pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnSetPriority(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, ulPriority);
}

BT_u32 BT_GetInterruptPriority(BT_u32 ulIRQ, BT_ERROR *pError) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		if(pError) {
			*pError = -1;
		}
		return 0;
	}

	return pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnGetPriority(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ, pError);
}

BT_ERROR BT_EnableInterrupt(BT_u32 ulIRQ) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}
	return pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnEnable(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ);
}

BT_ERROR BT_DisableInterrupt(BT_u32 ulIRQ) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}
	return pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnDisable(pIntc->hIRQ, ulIRQ-pIntc->ulBaseIRQ);
}

BT_ERROR BT_SetInterruptAffinity(BT_u32 ulIRQ, BT_u32 ulCPU) {
	const BT_INTERRUPT_CONTROLLER *pIntc = getInterruptController(ulIRQ);
	if(!pIntc) {
		return -1;
	}

	if(pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnSetAffinity) {
		return pIntc->hIRQ->h.pIf->oIfs.pDevIF->unConfigIfs.pIRQIF->pfnSetAffinity(pIntc->hIRQ, ulIRQ, ulCPU);
	}

	return -1;
}
