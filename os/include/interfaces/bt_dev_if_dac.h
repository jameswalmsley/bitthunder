#ifndef _BT_DEV_IF_DAC_H_
#define _BT_DEV_IF_DAC_H_

#include <fs/bt_devfs.h>
#include <collections/bt_list.h>

typedef struct _BT_DAC_INFO {
	const BT_DEVICE *pDevice;
	BT_HANDLE hDac;
	struct bt_list_head item;
	struct bt_devfs_node node;
	BT_u32 ulReferenceCount;
} BT_DAC_INFO;

typedef void 	 	(*BT_DAC_CALLBACK)					(BT_HANDLE hDAC, void *pParam);

typedef enum {
	BT_DAC_MODE_POLLED = 0,	///< A really simple, pure polling mode, with thread-yielding.
	BT_DAC_MODE_BUFFERED,		///< A fully buffered interrupt driven mode.
} BT_DAC_OPERATING_MODE;


typedef struct {
	BT_DAC_OPERATING_MODE	eMode;
	BT_u32					ulResolution;			//	Resolution for DAC
	BT_u32					ulUpdateInterval;		//  Reload Innterval
	BT_u32					ulBufferSize;			//	Buffersize for BUFFERED MODE
	BT_BOOL					bInternalReference;		//  use an internal/external reference
	BT_u32					ulGain;					//  Output Buffer gain
} BT_DAC_CONFIG;


typedef struct _BT_DEV_IF_DAC {
	BT_ERROR 	(*pfnSetConfig)				(BT_HANDLE hDAC, BT_DAC_CONFIG *pConfig);
	BT_ERROR 	(*pfnGetConfig)				(BT_HANDLE hDAC, BT_DAC_CONFIG *pConfig);
	BT_ERROR 	(*pfnStart)					(BT_HANDLE hDAC);
	BT_ERROR	(*pfnStop)					(BT_HANDLE hDAC);
	BT_HANDLE	(*pfnRegisterCallback)		(BT_HANDLE hDAC, BT_DAC_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
	BT_ERROR	(*pfnUnregisterCallback)	(BT_HANDLE hDAC, BT_HANDLE hCallback);
	BT_ERROR 	(*pfnWrite)					(BT_HANDLE hDAC, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pSrc);
} BT_DEV_IF_DAC;

BT_ERROR 	BT_DacStart					(BT_HANDLE hDAC);
BT_ERROR 	BT_DacStop					(BT_HANDLE hDAC);

BT_ERROR 	BT_DacSetConfiguration		(BT_HANDLE hDAC, BT_DAC_CONFIG *pConfig);
BT_ERROR 	BT_DacGetConfiguration		(BT_HANDLE hDAC, BT_DAC_CONFIG *pConfig);

BT_HANDLE	BT_DacRegisterCallback		(BT_HANDLE hDAC, BT_ADC_CALLBACK pfnCallback, void *pParam, BT_ERROR *pError);
BT_ERROR	BT_DacUnregisterCallback 	(BT_HANDLE hDAC, BT_HANDLE hCallback);

BT_ERROR	BT_DacWrite					(BT_HANDLE hDAC, BT_u32 ulChannel, BT_u32 ulSize, BT_u32 *pSrc);

BT_ERROR	BT_DacRegisterDevice		(BT_HANDLE hDevice, BT_DAC_INFO *dac);

#endif
