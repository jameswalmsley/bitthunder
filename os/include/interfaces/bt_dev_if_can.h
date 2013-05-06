#ifndef _BT_DEV_IF_CAN_H_
#define _BT_DEV_IF_CAN_H_

#include "bt_types.h"


#define	BT_CAN_EFF_FLAG		0x80000000			// Extended Frame Format
#define	BT_CAN_RTR_FLAG		0x40000000			// Remote Transmission Request

typedef enum {
	BT_CAN_MODE_POLLED = 0,	///< A really simple, pure polling mode, with thread-yielding.
	BT_CAN_MODE_BUFFERED,		///< A fully buffered interrupt driven mode.
} BT_CAN_OPERATING_MODE;


typedef struct {
	BT_u32	ulID;
	BT_u8	ucLength;
	BT_u8 	ucdata[8];
} BT_CAN_MESSAGE;

typedef struct {
	BT_UART_OPERATING_MODE	eMode;
	BT_u32 					ulBaudrate;
	BT_u16					usRxBufferSize;		///<
	BT_u16					usTxBufferSize;		///<
} BT_CAN_CONFIG;


typedef struct {
	BT_ERROR (*pfnSetBaudrate)	(BT_HANDLE hCAN, BT_u32 ulBaudrate);
	BT_ERROR (*pfnSetConfig)	(BT_HANDLE hCAN, BT_CAN_CONFIG *pConfig);
	BT_ERROR (*pfnGetConfig)	(BT_HANDLE hCAN, BT_CAN_CONFIG *pConfig);
	BT_ERROR (*pfnEnable)		(BT_HANDLE hCAN);
	BT_ERROR (*pfnDisable)		(BT_HANDLE hCAN);
	BT_ERROR (*pfnSendMessage)	(BT_HANDLE hCAN, BT_CAN_MESSAGE *pCanMessage);
	BT_ERROR (*pfnReadMessage)	(BT_HANDLE hCAN, BT_CAN_MESSAGE *pCanMessage);
} BT_DEV_IF_CAN;

/*
 *	Define the unified API for CAN devices in BlueThunder
 */
BT_ERROR BT_CanSetBaudrate			(BT_HANDLE hCAN, BT_u32 ulBaudrate);
BT_ERROR BT_CanSetConfiguration		(BT_HANDLE hCAN, BT_CAN_CONFIG *pConfig);
BT_ERROR BT_CanGetConfiguration		(BT_HANDLE hCAN, BT_CAN_CONFIG *pConfig);
BT_ERROR BT_CanEnable				(BT_HANDLE hCAN);
BT_ERROR BT_CanDisable				(BT_HANDLE hCAN);
BT_ERROR BT_CanSendMessage			(BT_HANDLE hCAN, BT_CAN_MESSAGE *pCanMessage);
BT_ERROR BT_CanReadMessage			(BT_HANDLE hCAN, BT_CAN_MESSAGE *pCanMessage);


#endif
