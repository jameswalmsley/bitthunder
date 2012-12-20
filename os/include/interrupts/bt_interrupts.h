#ifndef _BT_INTERRUPTS_H_
#define _BT_INTERRUPTS_H_

typedef BT_ERROR (*BT_FN_INTERRUPT_HANDLER)(BT_u32 ulIRQ, void *pParam);

/**
 *
 **/
typedef struct _BT_INTERRUPT_CONTROLLER {
	BT_MODULE_INFO 	oModuleInfo;
	BT_ERROR		(*pfnInitialise)		(BT_u32 ulTotalIRQs);
	BT_ERROR		(*pfnCleanup)			(void);
	BT_ERROR		(*pfnRegister)			(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
	BT_ERROR		(*pfnUnregister)		(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
	BT_ERROR		(*pfnSetPriority)		(BT_u32 ulIRQ, BT_u32 ulPriority);
	BT_u32			(*pfnGetPriority)		(BT_u32 ulIRQ, BT_ERROR *pError);
	BT_ERROR		(*pfnEnable)			(BT_u32 ulIRQ);
	BT_ERROR		(*pfnDisable)			(BT_u32 ulIRQ);
	BT_ERROR		(*pfnSetAffinity)		(BT_u32 ulIRQ, BT_u32 ulCPU);
	BT_u32			(*pfnGetTotalIRQs)		(BT_ERROR *pError);
} BT_INTERRUPT_CONTROLLER;

BT_ERROR 	BT_RegisterInterruptController(BT_u32 ulBaseIRQ, BT_u32 ulTotalIRQs, const BT_INTERRUPT_CONTROLLER *pIntc);

BT_ERROR 	BT_RegisterInterrupt(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
BT_ERROR 	BT_UnregisterInterrupt(BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
BT_ERROR 	BT_SetInterruptPriority(BT_u32 ulIRQ, BT_u32 ulPriority);
BT_u32 		BT_GetInterruptPriority(BT_u32 ulIRQ, BT_ERROR *pError);
BT_ERROR	BT_EnableInterrupt(BT_u32 ulIRQ);
BT_ERROR	BT_DisableInterrupt(BT_u32 ulIRQ);
BT_ERROR 	BT_SetInterruptAffinity(BT_u32 ulIRQ, BT_u32 ulCPU);

#endif
