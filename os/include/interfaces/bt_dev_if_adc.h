#ifndef _BT_DEV_IF_ADC_H_
#define _BT_DEV_IF_ADC_H_

#include "bt_types.h"
#include <interrupts/bt_interrupts.h>

typedef void 	 	(*BT_ADC_CALLBACK)					(BT_HANDLE hAdc, void *pParam);

typedef enum {
	BT_ADC_MODE_POLLED = 0,	///< A really simple, pure polling mode, with thread-yielding.
	BT_ADC_MODE_BUFFERED,		///< A fully buffered interrupt driven mode.
} BT_ADC_OPERATING_MODE;

typedef enum {
	BT_ADC_INPUT_MODE_SINGLE_END = 0,
	BT_ADC_INPUT_MODE_DIFFERENTIAL,
} BT_ADC_INPUT_MODE;

typedef struct {
	BT_ADC_OPERATING_MODE	eMode;
	BT_u32					ulSampleRate;			//	Samplerate for ADC
	BT_u32					ulResolution;			//	Resolution for ADC
	BT_ADC_INPUT_MODE		eInputMode;				//
	BT_u32					ulHWAverageCount;		//	Use internal hardware averaging over count
	BT_u32					ulSWAverageCount;		//	do software averaging over count
	BT_u32					ulActiveChannels;		//	BitMask for every channel
	BT_u32					ulBufferSize;			//	Buffersize for BUFFERED MODE
	BT_BOOL					bUseIntReference;		//  Use internal reference;
} BT_ADC_CONFIG;


typedef struct _BT_DEV_IF_ADC {
	BT_ERROR 	(*pfnSetConfig)				(BT_HANDLE hUart, BT_ADC_CONFIG *pConfig);
	BT_ERROR 	(*pfnGetConfig)				(BT_HANDLE hUart, BT_ADC_CONFIG *pConfig);
	BT_ERROR 	(*pfnStart)					(BT_HANDLE hAdc);
	BT_ERROR	(*pfnStop)					(BT_HANDLE hAdc);
	BT_HANDLE	(*pfnRegisterCallback)		(BT_HANDLE hAdc, BT_ADC_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
	BT_ERROR	(*pfnUnregisterCallback)	(BT_HANDLE hAdc, BT_HANDLE hCallback);
	BT_ERROR 	(*pfnRead)					(BT_HANDLE hAdc, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pucDest);
} BT_DEV_IF_ADC;

BT_ERROR 	BT_AdcStart					(BT_HANDLE hAdc);
BT_ERROR 	BT_AdcStop					(BT_HANDLE hAdc);

BT_ERROR 	BT_AdcSetConfiguration		(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig);
BT_ERROR 	BT_AdcGetConfiguration		(BT_HANDLE hAdc, BT_ADC_CONFIG *pConfig);

BT_HANDLE	BT_AdcRegisterCallback		(BT_HANDLE hAdc, BT_ADC_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
BT_ERROR	BT_AdcUnregisterCallback 	(BT_HANDLE hAdc, BT_HANDLE hCallback);

BT_ERROR	BT_AdcRead					(BT_HANDLE hAdc, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pucDest);

#endif
