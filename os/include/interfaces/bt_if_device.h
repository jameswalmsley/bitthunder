#ifndef _BT_IF_DEVICE_H_
#define _BT_IF_DEVICE_H_

#include "bt_types.h"
#include "bt_if_chardev.h"
#include "interfaces/bt_dev_if_timer.h"
#include "interfaces/bt_if_power.h"
#include "interfaces/bt_dev_if_uart.h"

typedef enum _BT_DEV_IF_TYPE {
	BT_DEV_IF_T_NONE=0,
	BT_DEV_IF_T_SYSTEM,
	BT_DEV_IF_T_INTC,
	BT_DEV_IF_T_TIMER,
	BT_DEV_IF_T_GPIO,
	BT_DEV_IF_T_UART,
	BT_DEV_IF_T_I2C,
	BT_DEV_IF_T_CAN,
	BT_DEV_IF_T_SPI,
	BT_DEV_IF_T_EMAC,
} BT_DEV_IF_TYPE;

typedef struct _BT_DEV_INTERFACE *BT_DEV_INTERFACE;

typedef union {
	const BT_DEV_INTERFACE		p;
	const BT_DEV_IF_TIMER	   *pTimerIF;
	const BT_DEV_IF_UART 	   *pUartIF;

} BT_DEV_IFS;


typedef BT_HANDLE	(*BT_DEVICE_HANDLE_OPEN)	(BT_u32 nDeviceID, BT_ERROR *pError);

typedef struct _BT_IF_DEVICE {
	BT_u32					ulTotalDevices;
	BT_DEVICE_HANDLE_OPEN	pfnOpen;

	const BT_IF_POWER	   *pPowerIF;
	BT_DEV_IF_TYPE 			eConfigType;
	const BT_DEV_IFS 	   *unConfigIfs;

	const BT_IF_CHARDEV	   *pCharDevIf;
//	const BT_IF_BLOCKDEV   *pBlockDevIf;

} BT_IF_DEVICE;


#endif
