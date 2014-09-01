#ifndef _BT_DEV_IF_TIMER_H_
#define _BT_DEV_IF_TIMER_H_

#include "bt_types.h"
#include <interrupts/bt_interrupts.h>

typedef void 	 	(*BT_TIMER_CALLBACK)					(BT_HANDLE hTimer, void *pParam);

typedef enum {
	BT_TIMER_CONFIG_32BIT_TMR = 0,
	BT_TIMER_CONFIG_32BIT_RTC,
	BT_TIMER_CONFIG_16BIT_TMR,
} BT_TIMER_CONFIG_MODE;

typedef enum {
	BT_TIMER_DIRECTION_DOWN = 0,
	BT_TIMER_DIRECTION_UP,
} BT_TIMER_DIRECTION;

typedef enum {
	BT_TIMER_MODE_ONESHOT = 0,
	BT_TIMER_MODE_PERIODIC,
	BT_TIMER_MODE_CAPTURE_COUNT,
	BT_TIMER_MODE_CAPTURE_TIME,
	BT_TIMER_MODE_PWM,
} BT_TIMER_MODE;

typedef enum {
	BT_TIMER_EDGE_POSITIVE = 0,
	BT_TIMER_EDGE_NEGATIVE,
	BT_TIMER_EDGE_BOTH,
} BT_TIMER_EDGE;

typedef struct _BT_TIMER_CONFIG{
	BT_TIMER_CONFIG_MODE	eConfig;
	BT_TIMER_DIRECTION		eDirection;
	BT_BOOL					bWaitOnTrigger;
	BT_BOOL					bInterruptOnMatch;
	BT_TIMER_MODE			eMode;
	BT_TIMER_EDGE			eEdge;
	BT_u32					ulFrequency;
} BT_TIMER_CONFIG;

typedef struct _BT_DEV_IF_TIMER {
	BT_u32		(*pfnGetInputClock)			(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_ERROR 	(*pfnSetConfig)				(BT_HANDLE hUart, void *pConfig);
	BT_ERROR 	(*pfnGetConfig)				(BT_HANDLE hUart, void *pConfig);
	BT_ERROR 	(*pfnStart)					(BT_HANDLE hTimer);
	BT_ERROR	(*pfnStop)					(BT_HANDLE hTimer);
	BT_ERROR	(*pfnEnableInterrupt)		(BT_HANDLE hTimer);
	BT_ERROR	(*pfnDisableInterrupt)		(BT_HANDLE hTimer);
	BT_ERROR	(*pfnOverrideInterrupt)		(BT_HANDLE hTimer, BT_FN_INTERRUPT_HANDLER pfnHandler, void *pParam);
	BT_HANDLE	(*pfnRegisterCallback)		(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
	BT_ERROR	(*pfnUnregisterCallback)	(BT_HANDLE hTimer, BT_HANDLE hCallback);
	BT_u32 		(*pfnGetPrescaler)			(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_ERROR	(*pfnSetPrescaler)			(BT_HANDLE hTimer, BT_u32 ulPrescaler);
	BT_u32		(*pfnGetPeriodCount)		(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_ERROR 	(*pfnSetPeriodCount)		(BT_HANDLE hTimer, BT_u32 ulValue);
	BT_u32		(*pfnGetMatch)				(BT_HANDLE hTimer, BT_u32 ulChannel, BT_ERROR *pError);
	BT_ERROR 	(*pfnSetMatch)				(BT_HANDLE hTimer, BT_u32 ulChannel, BT_u32 ulValue);
	BT_ERROR 	(*pfnEnableReload)			(BT_HANDLE hTimer);
	BT_ERROR	(*pfnDisableReload)			(BT_HANDLE hTimer);
	BT_u32 		(*pfnGetValue)				(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_ERROR	(*pfnSetValue)				(BT_HANDLE hTimer, BT_u32 ulValue);
	BT_u32		(*pfnGetMaxValue)			(BT_HANDLE hTimer, BT_ERROR *pError);
} BT_DEV_IF_TIMER;

BT_ERROR 	BT_TimerStart				(BT_HANDLE hTimer);
BT_ERROR 	BT_TimerStop				(BT_HANDLE hTimer);

BT_ERROR 	BT_TimerSetConfiguration	(BT_HANDLE hTimer, void *pConfig);
BT_ERROR 	BT_TimerGetConfiguration	(BT_HANDLE hTimer, void *pConfig);

BT_ERROR	BT_TimerEnableInterrupt		(BT_HANDLE hTimer);
BT_ERROR	BT_TimerDisableInterrupt	(BT_HANDLE hTimer);
BT_HANDLE	BT_TimerRegisterCallback	(BT_HANDLE hTimer, BT_TIMER_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
BT_ERROR	BT_TimerUnregisterCallback 	(BT_HANDLE hTimer, BT_HANDLE hCallback);

BT_u32		BT_TimerGetPrescaler		(BT_HANDLE hTimer, BT_ERROR *pError);
BT_ERROR	BT_TimerSetPrescaler 		(BT_HANDLE hTimer, BT_u32 ulPrescaler);
BT_u32 		BT_TimerGetPeriodCount		(BT_HANDLE hTimer, BT_ERROR *pError);
BT_ERROR	BT_TimerSetPeriodCount		(BT_HANDLE hTimer, BT_u32 ulPeriodCount);
BT_u32 		BT_TimerGetMatch			(BT_HANDLE hTimer, BT_u32 ulChannel, BT_ERROR *pError);
BT_ERROR	BT_TimerSetMatch			(BT_HANDLE hTimer, BT_u32 ulChannel, BT_u32 ulValue);
BT_ERROR	BT_TimerSetFrequency		(BT_HANDLE hTimer, BT_u32 ulFrequencyHz);
BT_u32		BT_TimerGetFrequency		(BT_HANDLE hTimer, BT_ERROR *pError);

BT_ERROR	BT_TimerEnableReload 		(BT_HANDLE hTimer);
BT_ERROR	BT_TimerDisableReload		(BT_HANDLE hTimer);

BT_u32		BT_TimerGetValue			(BT_HANDLE hTimer, BT_ERROR *pError);
BT_ERROR	BT_TimerSetValue			(BT_HANDLE hTimer, BT_u32 ulValue);
BT_BOOL		BT_TimerExpired				(BT_HANDLE hTimer, BT_ERROR *pError);

#endif
