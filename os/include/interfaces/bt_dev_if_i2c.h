#ifndef _BT_DEV_IF_I2C_H_
#define _BT_DEV_IF_I2C_H_

#include "bt_types.h"

typedef enum {
	BT_I2C_MODE_POLLED = 0,	///< A really simple, pure polling mode, with thread-yielding.
	BT_I2C_MODE_BUFFERED,	///< A fully buffered interrupt driven mode.
} BT_I2C_OPERATING_MODE;

typedef enum {
	BT_I2C_CLOCKRATE_100kHz = 0,
	BT_I2C_CLOCKRATE_400kHz,
	BT_I2C_CLOCKRATE_1000kHz,
	BT_I2C_CLOCKRATE_3400kHz,
} BT_I2C_CLOCKRATE;

typedef enum {
	BT_I2C_WRITE_ACCESS = 0,
	BT_I2C_READ_ACCESS,
} BT_I2C_ACCESS_MODE;

typedef struct {
	BT_I2C_OPERATING_MODE	eMode;
	BT_I2C_CLOCKRATE		ulClockrate;
	BT_u16					ulRxBufferSize;		///<
	BT_u16					ulTxBufferSize;		///<
} BT_I2C_CONFIG;

typedef struct {
	BT_ERROR (*pfnSetClockrate)	(BT_HANDLE hI2C, BT_I2C_CLOCKRATE eClockrate);
	BT_ERROR (*pfnSetConfig)	(BT_HANDLE hI2C, BT_I2C_CONFIG *pConfig);
	BT_ERROR (*pfnGetConfig)	(BT_HANDLE hI2C, BT_I2C_CONFIG *pConfig);
	BT_ERROR (*pfnEnable)		(BT_HANDLE hI2C);
	BT_ERROR (*pfnDisable)		(BT_HANDLE hI2C);
	BT_ERROR (*pfnStart)		(BT_HANDLE hI2C);
	BT_ERROR (*pfnSendAddress)	(BT_HANDLE hI2C, BT_u32 ulAddress, BT_I2C_ACCESS_MODE eAccessMode);
	BT_ERROR (*pfnGetData)		(BT_HANDLE hI2C, BT_u8 *pDest, BT_u32 ulLength);
	BT_ERROR (*pfnSendData)		(BT_HANDLE hI2C, BT_u8 *pSrc, BT_u32 ulLength);
	BT_ERROR (*pfnSendNack)		(BT_HANDLE hI2C);
	BT_ERROR (*pfnSendAck)		(BT_HANDLE hI2C);
	BT_BOOL  (*pfnGetAck)		(BT_HANDLE hI2C, BT_ERROR *pError);
	BT_ERROR (*pfnStop)			(BT_HANDLE hI2C);
	BT_ERROR (*pfnWrite)		(BT_HANDLE hI2C, BT_u8 ucDevice, BT_u8 *pucSource, BT_u32 ulLength);
	BT_ERROR (*pfnRead)			(BT_HANDLE hI2C, BT_u8 ucDevice, BT_u8 *pucDest  , BT_u32 ulLength);
} BT_DEV_IF_I2C;

/*
 *	Define the unified API for I2C devices in BlueThunder
 */
BT_ERROR BT_I2CSetClockrate			(BT_HANDLE hI2C, BT_I2C_CLOCKRATE eClockrate);
BT_ERROR BT_I2CSetConfiguration		(BT_HANDLE hI2C, BT_I2C_CONFIG *pConfig);
BT_ERROR BT_I2CGetConfiguration		(BT_HANDLE hI2C, BT_I2C_CONFIG *pConfig);
BT_ERROR BT_I2CEnable				(BT_HANDLE hI2C);
BT_ERROR BT_I2CDisable				(BT_HANDLE hI2C);
BT_ERROR BT_I2CStart				(BT_HANDLE hI2C);
BT_ERROR BT_I2CSendAddress			(BT_HANDLE hI2C, BT_u32 ulAddress, BT_I2C_ACCESS_MODE eAccessMode);
BT_ERROR BT_I2CGetData				(BT_HANDLE hI2C, BT_u8 *pDest, BT_u32 ulLength);
BT_ERROR BT_I2CSendData				(BT_HANDLE hI2C, BT_u8 *pSrc, BT_u32 ulLength);
BT_ERROR BT_I2CSendNack				(BT_HANDLE hI2C);
BT_ERROR BT_I2CSendAck				(BT_HANDLE hI2C);
BT_BOOL  BT_I2CGetAck				(BT_HANDLE hI2C, BT_ERROR *pError);
BT_ERROR BT_I2CStop					(BT_HANDLE hI2C);
BT_ERROR BT_I2CWrite(BT_HANDLE hI2C, BT_u8 ucDevice, BT_u8 *pucSource, BT_u32 ulLength);
BT_ERROR BT_I2CRead(BT_HANDLE hI2C, BT_u8 ucDevice, BT_u8 *pucDest, BT_u32 ulLength);



#endif
