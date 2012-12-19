#ifndef _BT_DEV_IF_TIMER_H_
#define _BT_DEV_IF_TIMER_H_

#include "bt_types.h"

typedef void 	 	(*BT_TIMER_CALLBACK)					(BT_HANDLE hTimer, void *pParam);

typedef BT_u32		(*BT_DEV_IF_TIMER_GETINPUTCLOCK)		(BT_HANDLE hTimer, BT_ERROR *pError);
typedef BT_ERROR 	(*BT_DEV_IF_TIMER_START) 				(BT_HANDLE hTimer);
typedef BT_ERROR 	(*BT_DEV_IF_TIMER_STOP)  				(BT_HANDLE hTimer);
typedef BT_ERROR 	(*BT_DEV_IF_TIMER_ENABLE_INTERRUPT)		(BT_HANDLE hTimer);
typedef BT_ERROR 	(*BT_DEV_IF_TIMER_DISABLE_INTERRUPT)	(BT_HANDLE hTimer);
typedef BT_HANDLE 	(*BT_DEV_IF_TIMER_REGISTER_CALLBACK)	(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
typedef BT_ERROR	(*BT_DEV_IF_TIMER_UNREGISTER_CALLBACK)	(BT_HANDLE hTimer, BT_HANDLE hCallback);
typedef BT_u32		(*BT_DEV_IF_TIMER_GETPRESCALER)			(BT_HANDLE hTimer, BT_ERROR *pError);
typedef BT_ERROR	(*BT_DEV_IF_TIMER_SETPRESCALER)			(BT_HANDLE hTimer, BT_u32 ulPrescaler);
typedef BT_u32		(*BT_DEV_IF_TIMER_GETPERIODCOUNT)		(BT_HANDLE hTimer, BT_ERROR *pError);
typedef BT_ERROR	(*BT_DEV_IF_TIMER_SETPERIODCOUNT)		(BT_HANDLE hTimer, BT_u32 ulValue);
typedef BT_ERROR	(*BT_DEV_IF_TIMER_ENABLE_RELOAD)		(BT_HANDLE hTimer);
typedef BT_ERROR	(*BT_DEV_IF_TIMER_DISABLE_RELOAD)		(BT_HANDLE hTimer);
typedef BT_u32		(*BT_DEV_IF_TIMER_GETVALUE)				(BT_HANDLE hTimer, BT_ERROR *pError);
typedef BT_ERROR	(*BT_DEV_IF_TIMER_SETVALUE)				(BT_HANDLE hTimer, BT_u32 ulValue);
typedef BT_u32		(*BT_DEV_IF_TIMER_GETMAXVALUE)			(BT_HANDLE hTimer, BT_ERROR *pError);

typedef struct _BT_DEV_IF_TIMER {
	BT_DEV_IF_TIMER_GETINPUTCLOCK 		pfnGetInputClock;
	BT_DEV_IF_TIMER_START				pfnStart;
	BT_DEV_IF_TIMER_STOP				pfnStop;
	BT_DEV_IF_TIMER_ENABLE_INTERRUPT	pfnEnableInterrupt;
	BT_DEV_IF_TIMER_DISABLE_INTERRUPT	pfnDisableInterrupt;
	BT_DEV_IF_TIMER_REGISTER_CALLBACK 	pfnRegisterCallback;
	BT_DEV_IF_TIMER_UNREGISTER_CALLBACK	pfnUnregisterCallback;
	BT_DEV_IF_TIMER_GETPRESCALER 		pfnGetPrescaler;
	BT_DEV_IF_TIMER_SETPRESCALER		pfnSetPrescaler;
	BT_DEV_IF_TIMER_GETPERIODCOUNT		pfnGetPeriodCount;
	BT_DEV_IF_TIMER_SETPERIODCOUNT 		pfnSetPeriodCount;
	BT_DEV_IF_TIMER_ENABLE_RELOAD 		pfnEnableReload;
	BT_DEV_IF_TIMER_DISABLE_RELOAD 		pfnDisableReload;
	BT_DEV_IF_TIMER_GETVALUE 			pfnGetValue;
	BT_DEV_IF_TIMER_SETVALUE 			pfnSetValue;
} BT_DEV_IF_TIMER;

BT_ERROR 	BT_TimerStart				(BT_HANDLE hTimer);
BT_ERROR 	BT_TimerStop				(BT_HANDLE hTimer);

BT_ERROR	BT_TimerEnableInterrupt		(BT_HANDLE hTimer);
BT_ERROR	BT_TimerDisableInterrupt	(BT_HANDLE hTimer);
BT_HANDLE	BT_TimerRegisterCallback	(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
BT_ERROR	BT_TimerUnregisterCallback 	(BT_HANDLE hTimer, BT_HANDLE hCallback);

BT_u32		BT_TimerGetPrescaler		(BT_HANDLE hTimer, BT_ERROR *pError);
BT_ERROR	BT_TimerSetPrescaler 		(BT_HANDLE hTimer, BT_u32 ulPrescaler);
BT_u32 		BT_TimerGetPeriodCount		(BT_HANDLE hTimer, BT_ERROR *pError);
BT_ERROR	BT_TimerSetPeriodCount		(BT_HANDLE hTimer, BT_u32 ulPeriodCount);
BT_ERROR	BT_TimerSetFrequency		(BT_HANDLE hTimer, BT_u32 ulFrequencyHz);
BT_u32		BT_TimerGetFrequency		(BT_HANDLE hTimer, BT_ERROR *pError);

BT_ERROR	BT_TimerEnableReload 		(BT_HANDLE hTimer);
BT_ERROR	BT_TimerDisableReload		(BT_HANDLE hTimer);

BT_u32		BT_TimerGetValue			(BT_HANDLE hTimer, BT_ERROR *pError);
BT_ERROR	BT_TimerSetValue			(BT_HANDLE hTimer, BT_u32 ulValue);
BT_BOOL		BT_TimerExpired				(BT_HANDLE hTimer, BT_ERROR *pError);

#endif
