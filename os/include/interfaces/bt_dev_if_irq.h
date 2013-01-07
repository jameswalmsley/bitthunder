#ifndef _BT_DEV_IF_IRQ_H_
#define _BT_DEV_IF_IRQ_H_

#include "bt_types.h"
#include "interrupts/bt_interrupts.h"

typedef struct _BT_DEV_IF_IRQ {
	BT_ERROR		(*pfnRegister)			(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
	BT_ERROR		(*pfnUnregister)		(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
	BT_ERROR		(*pfnSetPriority)		(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_u32 ulPriority);
	BT_u32			(*pfnGetPriority)		(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_ERROR *pError);
	BT_ERROR		(*pfnEnable)			(BT_HANDLE hIRQ, BT_u32 ulIRQ);
	BT_ERROR		(*pfnDisable)			(BT_HANDLE hIRQ, BT_u32 ulIRQ);
	BT_ERROR		(*pfnSetAffinity)		(BT_HANDLE hIRQ, BT_u32 ulIRQ, BT_u32 ulCPU);
} BT_DEV_IF_IRQ;



#endif
