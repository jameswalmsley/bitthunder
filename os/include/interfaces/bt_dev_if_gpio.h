#ifndef _BT_DEV_IF_GPIO_H_
#define _BT_DEV_IF_GPIO_H_

#include "gpio/bt_gpio.h"

typedef struct _BT_DEV_IF_GPIO {
	BT_ERROR		(*pfnSet)				(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_BOOL bValue);
	BT_BOOL			(*pfnGet)				(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError);
	BT_ERROR		(*pfnSetDirection)		(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_GPIO_DIRECTION eDirection);
	BT_GPIO_DIRECTION (*pfnGetDirection)	(BT_HANDLE hGPIO, BT_u32 ulGPIO, BT_ERROR *pError);
	BT_ERROR		(*pfnEnableInterrupt)	(BT_HANDLE hGPIO, BT_u32 ulGPIO);
	BT_ERROR		(*pfnDisableInterrupt)	(BT_HANDLE hGPIO, BT_u32 ulGPIO);
} BT_DEV_IF_GPIO;



#endif
