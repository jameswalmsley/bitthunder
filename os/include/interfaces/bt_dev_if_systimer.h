#ifndef _BT_DEV_IF_SYSTIMER_H_
#define _BT_DEV_IF_SYSTIMER_H_

#include "bt_types.h"
#include <interrupts/bt_interrupts.h>

typedef struct _BT_DEV_IF_SYSTIMER {
	BT_ERROR	(*pfnGetClockRate)		(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_ERROR	(*pfnRegisterInterrupt)	(BT_HANDLE hTimer, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
	BT_ERROR	(*pfnEnableInterrupt)	(BT_HANDLE hTimer);
	BT_ERROR	(*pfnDisableInterrupt)	(BT_HANDLE hTimer);
	BT_u32		(*pfnGetFrequency)		(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_ERROR	(*pfnSetFrequency)		(BT_HANDLE hTimer, BT_u32 	 ulFrequencyHz);
	BT_u32		(*pfnGetOffset)			(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_ERROR	(*pfnStart)				(BT_HANDLE hTimer);
	BT_ERROR	(*pfnStop)				(BT_HANDLE hTimer);
} BT_DEV_IF_SYSTIMER;

#endif
