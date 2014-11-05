/**
 *	SD Host Controller Interface Driver Implementation for BitThunder.
 *
 *	This allows commands to be sent and received, as well as large data blocks
 *	to be transferred.
 *
 *	This is a generic layer, on-top of which the SD-Card driver sits, but
 *	its possible for a Bluetooth/Wifi/GPS drivers to sit above this, to
 *	provide support for such cards.
 **/

#include <bitthunder.h>
#include "../core.h"
#include "../host.h"
#include "../sdcard.h"

BT_DEF_MODULE_NAME				("Generic SPI Controller Driver")
BT_DEF_MODULE_DESCRIPTION		("Implements a driver for SDCARD abstraction for SPI controllers")
BT_DEF_MODULE_AUTHOR			("Robert Steinbauer")
BT_DEF_MODULE_EMAIL				("rsteinbauer@riegl.co.at")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	BT_SPI_DEVICE 			   *pSpi;
	BT_MMC_CARD_EVENTRECEIVER 	pfnEventReceiver;
	MMC_HOST				   *pHost;
	BT_MMC_HOST_OPS			   *pHostOps;
	BT_u32						ulFlags;
};

static const BT_IF_HANDLE oHandleInterface;

#if (BT_CONFIG_DRIVERS_SDCARD_SPI_CS_GPIO != -1)
	#define		SELECT()				BT_GpioSet(BT_CONFIG_DRIVERS_SDCARD_SPI_CS_GPIO, BT_FALSE);
	#define		DESELECT()				BT_GpioSet(BT_CONFIG_DRIVERS_SDCARD_SPI_CS_GPIO, BT_TRUE);
#else
	#define		SELECT()
	#define		DESELECT()
#endif

static BT_ERROR spi_cleanup(BT_HANDLE hSDIO) {

	// Dont't forget the handle will itself be cleanup up auto-magically!

	return BT_ERR_NONE;
}

static BT_ERROR spi_enable_clock(BT_HANDLE hSDIO) {
	return BT_ERR_NONE;
}

static BT_ERROR spi_disable_clock(BT_HANDLE hSDIO) {
	return BT_ERR_NONE;
}

static BT_BOOL spi_ready(BT_HANDLE hSDIO) {
	BT_u8 ucRx;

   	BT_SpiRead(hSDIO->pSpi, &ucRx, 1);

    return (ucRx == 0xFF);
}


static BT_ERROR spi_request(BT_HANDLE hSDIO, MMC_COMMAND *pCommand) {

	BT_u8 ucRes;
    BT_u8 ucStatus[16], i;

    BT_u8 *cp = ucStatus;

	while(!spi_ready(hSDIO)) {
		//BT_ThreadYield();
	}

    // Send command packet
	*cp++ = pCommand->opcode | 0x40;
	*cp++ = (pCommand->arg >> 24) & 0xFF;
	*cp++ = (pCommand->arg >> 16) & 0xFF;
	*cp++ = (pCommand->arg >>  8) & 0xFF;
	*cp++ = (pCommand->arg >>  0) & 0xFF;
	*cp = 0;
    if (pCommand->opcode == 0) *cp = 0x95;            // CRC for CMD0(0)
    if (pCommand->opcode == 8) *cp = 0x87;            // CRC for CMD8(0x1AA)
    cp++;

    BT_SpiWrite(hSDIO->pSpi, (const void*)ucStatus, 6);

    BT_u8 n = 10;                                // Wait for a valid response in timeout of 10 attempts

    for (i=0;i<16;i++)
    	ucStatus[i] = 0;
    do {
    	BT_SpiRead(hSDIO->pSpi, (void*)&ucStatus[15], 1);
    } while ((ucStatus[15] == 0xFF) && (--n));
    switch (pCommand->ulResponseType) {
		case BT_SDCARD_RESPONSE_TYPE_R1b: {
			do {
				BT_SpiRead(hSDIO->pSpi, (void*)&ucRes, 1);
			} while (ucRes == 0x00);
			break;
		}
		case BT_SDCARD_RESPONSE_TYPE_R2:
		case BT_SDCARD_RESPONSE_TYPE_R5: {
			BT_SpiRead(hSDIO->pSpi, (void*)&ucRes, 1);
			break;
		}

		case BT_SDCARD_RESPONSE_TYPE_R3:
		case BT_SDCARD_RESPONSE_TYPE_R4:
		case BT_SDCARD_RESPONSE_TYPE_R7: {
			BT_SpiRead(hSDIO->pSpi, (void*)&ucStatus[12], 4);
			break;
		}
		case BT_SDCARD_RESPONSE_TYPE_R1_DATA: {
			do {
				BT_SpiRead(hSDIO->pSpi, (void*)&ucRes, 1);
			} while (ucRes != 0xFE);
			BT_SpiRead(hSDIO->pSpi, (void*)&ucStatus[0], 16);
			BT_SpiRead(hSDIO->pSpi, (void*)&ucRes, 1);
			BT_SpiRead(hSDIO->pSpi, (void*)&ucRes, 1);
			break;
		}
		default:
			break;
	}

   	pCommand->response[0] = ((BT_u32)ucStatus[12]<<24) + ((BT_u32)ucStatus[13]<<16) + ((BT_u32)ucStatus[14]<<8) + ((BT_u32)ucStatus[15]);
   	pCommand->response[1] = ((BT_u32)ucStatus[8] <<24) + ((BT_u32)ucStatus[9] <<16) + ((BT_u32)ucStatus[10]<<8) + ((BT_u32)ucStatus[11]);
   	pCommand->response[2] = ((BT_u32)ucStatus[4] <<24) + ((BT_u32)ucStatus[5] <<16) + ((BT_u32)ucStatus[6] <<8) + ((BT_u32)ucStatus[7] );
   	pCommand->response[3] = ((BT_u32)ucStatus[0] <<24) + ((BT_u32)ucStatus[1] <<16) + ((BT_u32)ucStatus[2] <<8) + ((BT_u32)ucStatus[3] );

	return BT_ERR_NONE;
}

BT_ERROR spi_select(BT_HANDLE hSDIO) {
	SELECT();
	return BT_ERR_NONE;
}

BT_ERROR spi_deselect(BT_HANDLE hSDIO) {
	DESELECT();
	return BT_ERR_NONE;
}


static BT_s32 spi_read(BT_HANDLE hSDIO, BT_u32 ulBlocks, void *pBuffer) {
	register BT_u8 *p = (BT_u8 *) pBuffer;

	BT_s32 slRead = 0;
	BT_u8 ucData[2];

	BT_SPI_MESSAGE m;
	BT_SpiMessageInit(&m);

	BT_SPI_TRANSFER r = {
			.rx_buf	= p,
			.len	= 512,
	};

	BT_SpiMessageAddTail(&r, &m);

	BT_SPI_TRANSFER r1 = {
			.rx_buf	= ucData,
			.len	= 2,
	};

	BT_SpiMessageAddTail(&r1, &m);

	while(slRead < ulBlocks) {

		do {
			BT_SpiRead(hSDIO->pSpi, (void*)&ucData[0], 1);
		} while (ucData[0] != 0xFE);

		r.rx_buf = p;

		BT_SpiSync(hSDIO->pSpi, &m);

		p += r.len;

		slRead++;
	}


	if(slRead != ulBlocks) {
		// software reset on data line if an error occurred ...
	}

	return slRead;
}

static BT_s32 spi_write(BT_HANDLE hSDIO, BT_u32 ulSize, void *pBuffer) {
	register BT_u8 *p = (BT_u8 *) pBuffer;

	BT_u32 slWritten = 0;
	BT_u8 ucData[3];
	BT_u32 ulBlockSize = 512;

	BT_SPI_MESSAGE m;
	BT_SpiMessageInit(&m);

	BT_SPI_TRANSFER t = {
			.tx_buf	= ucData,
			.len	= 1,
	};
	BT_SpiMessageAddTail(&t, &m);

	BT_SPI_TRANSFER t1 = {
			.tx_buf	= p,
			.len	= ulBlockSize,
			};
	BT_SpiMessageAddTail(&t1, &m);

	BT_SPI_TRANSFER r = {
			.rx_buf	= ucData,
			.len	= 3,
			};
	BT_SpiMessageAddTail(&r, &m);

	while(slWritten < ulSize) {

		while(!spi_ready(hSDIO)) {
		}

		ucData[0] = 0xFC;
		t1.tx_buf = p;

		BT_SpiSync(hSDIO->pSpi, &m);

		p += ulBlockSize;

		slWritten++;

		if ((ucData[2] & 0x05) != 0x05)
			break;
	}

	while(!spi_ready(hSDIO)) {
		//BT_ThreadYield();
	}

	ucData[0] = 0xFD;				// Stop Token
	BT_SpiWrite(hSDIO->pSpi, &ucData[0], 1);

	return slWritten;
}

static BT_BOOL spi_is_card_present(BT_HANDLE hSDIO, BT_ERROR *pError) {

	if (BT_CONFIG_DRIVERS_SDCARD_SPI_DETECT_GPIO != -1)
		return (BT_GpioGet(BT_CONFIG_DRIVERS_SDCARD_SPI_DETECT_GPIO, pError) || (hSDIO->ulFlags & SDCARD_FLAGS_ALWAYS_PRESENT));
	else
		return (hSDIO->ulFlags & SDCARD_FLAGS_ALWAYS_PRESENT);
}

static BT_ERROR spi_event_subscribe(BT_HANDLE hSDIO, BT_MMC_CARD_EVENTRECEIVER pfnReceiver, MMC_HOST *pHost) {
	BT_ERROR Error = BT_ERR_NONE;

	hSDIO->pfnEventReceiver = pfnReceiver;
	hSDIO->pHost 			= pHost;

	// Test for card presence, and generate detection signal.

	if(spi_is_card_present(hSDIO, &Error)) {
		if(hSDIO->pfnEventReceiver) {
			hSDIO->pfnEventReceiver(hSDIO->pHost, BT_MMC_CARD_DETECTED, BT_FALSE);
		}
	}

	return Error;
}

static BT_ERROR spi_initialise(BT_HANDLE hSDIO) {
	#if (BT_CONFIG_DRIVERS_SDCARD_SPI_CS_GPIO != -1)
	BT_GpioSet(BT_CONFIG_DRIVERS_SDCARD_SPI_CS_GPIO, BT_TRUE);
	BT_GpioSetDirection(BT_CONFIG_DRIVERS_SDCARD_SPI_CS_GPIO, BT_GPIO_DIR_OUTPUT);
	#endif

	BT_u8 ucData[10], i;

	for (i=0;i<10;i++)
		ucData[i] = 0xFF;

	BT_SpiWrite(hSDIO->pSpi, ucData, 10);

	return BT_ERR_NONE;
}

static const BT_MMC_OPS spi_mmc_ops = {
	.ulCapabilites1 	 = BT_MMC_SPI_MODE,
	.pfnRequest			 = spi_request,
	.pfnRead			 = spi_read,
	.pfnWrite			 = spi_write,
	.pfnEventSubscribe 	 = spi_event_subscribe,
	.pfnIsCardPresent	 = spi_is_card_present,
	.pfnSetInterruptMask = NULL,
	.pfnSetDataWidth     = NULL,
	.pfnSetBlockSize     = NULL,
	.pfnInitialise 		 = spi_initialise,
	.pfnEnableClock		 = spi_enable_clock,
	.pfnDisableClock	 = spi_disable_clock,
	.pfnSelect			 = spi_select,
	.pfnDeselect		 = spi_deselect,
};

static BT_HANDLE spi_probe(BT_SPI_DEVICE *spi, const BT_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	spi->bits_per_word = 8;
	spi->chip_select = 0;
	spi->mode = spi->pMaster->mode_bits;
	spi->max_speed_hz = 20000000;

	Error = BT_SpiSetup(spi);

	BT_HANDLE hSDIO = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hSDIO) {
		goto err_out;
	}

	//hSDIO->pDevice = pDevice;
	hSDIO->pSpi    = spi;	// Provide access to parameters on cleanup.

	spi_initialise(hSDIO);

	// We could check controller version to ensure compatibility, but
	// in all likely-hood (clutching at straws) newer versions would remain mostly
	// compatible.

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_PARAM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_irq;
	}

	hSDIO->pHostOps = (BT_MMC_HOST_OPS *) pResource->pParam;

	// Register with the SD Host controller.
	Error = BT_RegisterSDHostController(hSDIO, &spi_mmc_ops);
	if(Error) {
		goto err_free_irq;
	}

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_FLAGS, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_irq;
	}

	hSDIO->ulFlags = pResource->ulConfigFlags;

	hSDIO->pfnEventReceiver(hSDIO->pHost, BT_MMC_CARD_DETECTED, BT_TRUE);

	return hSDIO;

err_free_irq:

err_free_out:
	BT_DestroyHandle(hSDIO);

	if(pError) {
		*pError = Error;
	}

err_out:
	return NULL;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = spi_cleanup,
};

BT_INTEGRATED_DRIVER_DEF spi_driver = {
	.name			= "mmc,spi",
	.eType			= BT_DRIVER_SPI,
	.pfnSPIProbe	= spi_probe,
};
