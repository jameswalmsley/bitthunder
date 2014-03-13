#ifndef _BT_GPIO_H_
#define _BT_GPIO_H_

typedef enum _BT_GPIO_DIRECTION {
	BT_GPIO_DIR_UNKNOWN=0,
	BT_GPIO_DIR_HIGH_Z,
	BT_GPIO_DIR_INPUT,
	BT_GPIO_DIR_OUTPUT,
	BT_GPIO_DIR_OPEN_DRAIN,
	BT_GPIO_DIR_OPEN_SOURCE,
} BT_GPIO_DIRECTION;


BT_ERROR			BT_RegisterGpioController	(BT_u32 ulBaseGPIO, BT_u32 ulTotalGPIOs, BT_HANDLE hGPIO);

BT_ERROR   			BT_GpioSet			(BT_u32 ulGPIO, BT_BOOL bValue);
BT_BOOL				BT_GpioGet			(BT_u32 ulGPIO, BT_ERROR *pError);
BT_ERROR			BT_GpioSetDirection	(BT_u32 ulGPIO, BT_GPIO_DIRECTION eDirection);
BT_GPIO_DIRECTION 	BT_GpioGetDirection	(BT_u32 ulGPIO, BT_ERROR *pError);

BT_ERROR			BT_GpioEnableInterrupt	(BT_u32 ulGPIO);
BT_ERROR			BT_GpioDisableInterrupt	(BT_u32 ulGPIO);



#endif
