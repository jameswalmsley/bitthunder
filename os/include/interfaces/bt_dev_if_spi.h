#ifndef _BT_DEV_IF_SPI_H_
#define _BT_DEV_IF_SPI_H_

#include "bt_types.h"

typedef enum {
	BT_SPI_MODE_POLLED = 0,	///< A really simple, pure polling mode, with thread-yielding.
	//BT_SPI_MODE_INTERRUPT,		///< A non-buffered interrupt driven mode.
	BT_SPI_MODE_BUFFERED,		///< A fully buffered interrupt driven mode.
} BT_SPI_OPERATING_MODE;

typedef enum {
	BT_SSP_SPI_FRAME_FORMAT = 0,
	BT_SSP_TI_FRAME_FORMAT,
	BT_SSP_MICROWIRE_FRAME_FORMAT,
} BT_SSP_FRAME_FORMAT;

typedef enum {
	BT_SPI_CLKMODE00 = 0,
	BT_SPI_CLKMODE01,
	BT_SPI_CLKMODE10,
	BT_SPI_CLKMODE11,
} BT_SPI_CLOCK_MODE;

#define	BT_SPI_CPOL		0x01
#define	BT_SPI_CPHA		0x10



typedef struct {
	BT_SPI_OPERATING_MODE	eMode;
	BT_SSP_FRAME_FORMAT		eFrameFormat;

	BT_u32 					ulBaudrate;

	BT_u8					ucDataBits;
	BT_SPI_CLOCK_MODE		eSPIClockMode;

	BT_u16					ulRxBufferSize;		///<
	BT_u16					ulTxBufferSize;		///<
} BT_SPI_CONFIG;

typedef struct {
	BT_ERROR (*pfnSetBaudrate)	(BT_HANDLE hSpi, BT_u32 ulBaudrate);
	BT_ERROR (*pfnSetConfig)	(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig);
	BT_ERROR (*pfnGetConfig)	(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig);
	BT_ERROR (*pfnEnable)		(BT_HANDLE hSpi);
	BT_ERROR (*pfnDisable)		(BT_HANDLE hSpi);
	BT_ERROR (*pfnWrite)		(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucSource, BT_u32 ulSize);
	BT_ERROR (*pfnRead)			(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucDest  , BT_u32 ulSize);
} BT_DEV_IF_SPI;

/*
 *	Define the unified API for SPI devices in BlueThunder
 */
BT_ERROR BT_SpiSetBaudrate			(BT_HANDLE hSpi, BT_u32 ulBaudrate);
BT_ERROR BT_SpiSetConfiguration		(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig);
BT_ERROR BT_SpiGetConfiguration		(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig);
BT_ERROR BT_SpiEnable				(BT_HANDLE hSpi);
BT_ERROR BT_SpiDisable				(BT_HANDLE hSpi);
BT_ERROR BT_SpiWrite				(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucSource, BT_u32 ulSize);
BT_ERROR BT_SpiRead					(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucDest  , BT_u32 ulSize);

#endif
