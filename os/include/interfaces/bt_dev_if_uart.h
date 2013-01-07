#ifndef _BT_DEV_IF_UART_H_
#define _BT_DEV_IF_UART_H_

#include "bt_types.h"

typedef enum {
	BT_UART_MODE_POLLED = 0,	///< A really simple, pure polling mode, with thread-yielding.
	//BT_UART_MODE_INTERRUPT,		///< A non-buffered interrupt driven mode.
	BT_UART_MODE_BUFFERED,		///< A fully buffered interrupt driven mode.
} BT_UART_OPERATING_MODE;

typedef enum {
	BT_UART_5_DATABITS=5,
	BT_UART_6_DATABITS,
	BT_UART_7_DATABITS,
	BT_UART_8_DATABITS,
} BT_UART_DATABITS;

typedef enum {
	BT_UART_PARITY_NONE = 0,	///< No PARITY Checking is enabled and added to data frame
	BT_UART_PARITY_ODD,			///< Odd PARITY Checking is enabled and added to data frame
	BT_UART_PARITY_EVEN,		///< Even PARITY Checking is enabled and added to data frame
	BT_UART_PARITY_MARK,		///< Mark PARITY Checking is enabled and added to data frame
	BT_UART_PARITY_SPACE,		///< Space PARITY Checking is enabled and added to data frame
} BT_UART_PARITY_MODE;

typedef enum {
	BT_UART_ONE_STOP_BIT = 0,	///< One Stop bit
	BT_UART_TWO_STOP_BITS,		///< two stop bits
} BT_UART_STOPBIT_MODE;

typedef struct {
	BT_UART_OPERATING_MODE	eMode;
	BT_u32 					ulBaudrate;

	BT_u8					ucDataBits;
	BT_u8					ucStopBits;
	BT_u8					ucParity;

	BT_u16					ulRxBufferSize;		///<
	BT_u16					ulTxBufferSize;		///<
} BT_UART_CONFIG;

typedef struct {
	BT_ERROR (*pfnSetBaudrate)	(BT_HANDLE hUart, BT_u32 ulBaudrate);
	BT_ERROR (*pfnSetConfig)	(BT_HANDLE hUart, BT_UART_CONFIG *pConfig);
	BT_ERROR (*pfnGetConfig)	(BT_HANDLE hUart, BT_UART_CONFIG *pConfig);
	BT_ERROR (*pfnEnable)		(BT_HANDLE hUart);
	BT_ERROR (*pfnDisable)		(BT_HANDLE hUart);
} BT_DEV_IF_UART;

/*
 *	Define the unified API for UART devices in BlueThunder
 */
BT_ERROR BT_UartSetBaudrate			(BT_HANDLE hUart, BT_u32 ulBaudrate);
BT_ERROR BT_UartSetConfiguration	(BT_HANDLE hUart, BT_UART_CONFIG *pConfig);
BT_ERROR BT_UartGetConfiguration	(BT_HANDLE hUart, BT_UART_CONFIG *pConfig);
BT_ERROR BT_UartEnable				(BT_HANDLE hUart);
BT_ERROR BT_UartDisable				(BT_HANDLE hUart);

#endif
