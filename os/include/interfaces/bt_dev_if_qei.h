#ifndef _BT_DEV_IF_QEI_H_
#define _BT_DEV_IF_QEI_H_

#include "bt_types.h"
#include <interrupts/bt_interrupts.h>

typedef void 	 	(*BT_QEI_CALLBACK)					(BT_HANDLE hQEI, void *pParam);

typedef enum {
	BT_QEI_CAPTURE_2_EDGES = 0,
	BT_QEI_CAPTURE_4_EDGES,
} BT_QEI_CAPTURE_MODE;

typedef enum {
	BT_QEI_SIGNAL_QEI = 0,
	BT_QEI_SIGNAL_DIR_CLK,
} BT_QEI_SIGNAL_MODE;

typedef enum {
	BT_QEI_DIRECTION_POS = 0,
	BT_QEI_DIRECTION_NEG,
} BT_QEI_DIRECTION;

typedef struct {
	BT_QEI_CAPTURE_MODE	eCaptureMode;
	BT_QEI_SIGNAL_MODE	eSignalMode;
	BT_QEI_DIRECTION	eDirection;
	BT_u32				ulVelocityUpdateRate;
} BT_QEI_CONFIG;


typedef struct _BT_DEV_IF_QEI {
	BT_ERROR 	(*pfnSetConfig)				(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig);
	BT_ERROR 	(*pfnGetConfig)				(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig);
	BT_u32		(*pfnGetIndexCount)			(BT_HANDLE hQEI, BT_ERROR *pError);
	BT_u32		(*pfnGetPosition)			(BT_HANDLE hQEI, BT_ERROR *pError);
	BT_ERROR	(*pfnSetMaximumPosition)	(BT_HANDLE hQEI, BT_u32 ulValue);
	BT_ERROR	(*pfnSetPositionComparator)	(BT_HANDLE hQEI, BT_u32 ulChannel, BT_u32 ulValue);
	BT_u32 		(*pfnGetVelocity)			(BT_HANDLE hQEI, BT_ERROR *pError);
	BT_ERROR	(*pfnEnableInterrupt)		(BT_HANDLE hQEI, BT_u32 ulType);
	BT_ERROR	(*pfnDisableInterrupt)		(BT_HANDLE hQEI, BT_u32 ulType);
	BT_ERROR	(*pfnClearInterrupt)		(BT_HANDLE hQEI, BT_u32 ulType);
	BT_HANDLE	(*pfnRegisterCallback)		(BT_HANDLE hQEI, BT_QEI_CALLBACK pfnCallback, void *pParam, BT_u32 ulInterruptID, BT_ERROR *pError);
	BT_ERROR	(*pfnUnregisterCallback)	(BT_HANDLE hQEI, BT_HANDLE hCallback);
} BT_DEV_IF_QEI;

BT_ERROR 	BT_QEISetConfiguration		(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig);
BT_ERROR 	BT_QEIGetConfiguration		(BT_HANDLE hQEI, BT_QEI_CONFIG *pConfig);

BT_u32 		BT_QEIGetIndexCount			(BT_HANDLE hQEI, BT_ERROR *pError);
BT_u32		BT_QEIGetPosition			(BT_HANDLE hQEI, BT_ERROR *pError);
BT_ERROR	BT_QEISetMaximumPosition	(BT_HANDLE hQEI, BT_u32 ulValue);
BT_ERROR	BT_QEISetPositionComparator	(BT_HANDLE hQEI, BT_u32 ulChannel, BT_u32 ulValue);

BT_u32		BT_QEIGetVelocity			(BT_HANDLE hQEI, BT_ERROR *pError);

BT_ERROR	BT_QEIEnableInterrupt		(BT_HANDLE hQEI, BT_u32 ulType);
BT_ERROR	BT_QEIDisableInterrupt		(BT_HANDLE hQEI, BT_u32 ulType);
BT_ERROR	BT_QEIClearInterrupt		(BT_HANDLE hQEI, BT_u32 ulType);
BT_HANDLE	BT_QEIRegisterCallback		(BT_HANDLE hQEI, BT_QEI_CALLBACK pfnCallback, void *pParam, BT_u32 ulInterruptID, BT_ERROR *pError);
BT_ERROR	BT_QEIUnregisterCallback 	(BT_HANDLE hQEI, BT_HANDLE hCallback);


#endif
