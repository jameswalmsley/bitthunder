#ifndef _BT_DEV_IF_PWM_H_
#define _BT_DEV_IF_PWM_H_

#include "bt_types.h"
#include <interrupts/bt_interrupts.h>

typedef struct {
	BT_u32					ulFrequency;			//	output frequency
} BT_PWM_CONFIG;


typedef struct _BT_DEV_IF_PWM {
	BT_ERROR 	(*pfnSetConfig)				(BT_HANDLE hUart, BT_PWM_CONFIG *pConfig);
	BT_ERROR 	(*pfnGetConfig)				(BT_HANDLE hUart, BT_PWM_CONFIG *pConfig);
	BT_ERROR 	(*pfnStart)					(BT_HANDLE hPwm);
	BT_ERROR	(*pfnStop)					(BT_HANDLE hPwm);
	BT_u32		(*pfnGetPeriodCount)		(BT_HANDLE hPwm, BT_ERROR *pError);
	BT_u32		(*pfnGetFrequency)			(BT_HANDLE hPwm, BT_ERROR *pError);
	BT_ERROR 	(*pfnSetFrequency)			(BT_HANDLE hPwm, BT_u32 ulValue);
	BT_u32 		(*pfnGetDutyCycle)			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_ERROR *pError);
	BT_ERROR	(*pfnSetDutyCycle)			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_u32 ulValue);
	BT_u32 		(*pfnGetDeadTime)			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_ERROR *pError);
	BT_ERROR	(*pfnSetDeadTime)			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_u32 ulValue);
} BT_DEV_IF_PWM;

BT_ERROR 	BT_PwmStart					(BT_HANDLE hPwm);
BT_ERROR 	BT_PwmStop					(BT_HANDLE hPwm);

BT_ERROR 	BT_PwmSetConfiguration		(BT_HANDLE hPwm, BT_PWM_CONFIG *pConfig);
BT_ERROR 	BT_PwmGetConfiguration		(BT_HANDLE hPwm, BT_PWM_CONFIG *pConfig);

BT_u32 		BT_PwmGetPeriodCount		(BT_HANDLE hPwm, BT_ERROR *pError);
BT_ERROR	BT_PwmSetFrequency			(BT_HANDLE hPwm, BT_u32 ulFrequencyHz);
BT_u32		BT_PwmGetFrequency			(BT_HANDLE hPwm, BT_ERROR *pError);

BT_u32		BT_PwmGetDutyCylce			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_ERROR *pError);
BT_ERROR	BT_PwmSetDutyCycle			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_u32 ulValue);

BT_u32		BT_PwmGetDeadTime			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_ERROR *pError);
BT_ERROR	BT_PwmSetDeadTime			(BT_HANDLE hPwm, BT_u32 ulChannel, BT_u32 ulValue);

#endif
