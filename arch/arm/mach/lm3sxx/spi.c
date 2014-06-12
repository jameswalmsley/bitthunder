/**
 *	LM3Sxx Hal for BitThunder
 *	SPI Driver Implementation.
 *
 *	This driver serves as robust example as to how implement a fully functional SPI device driver
 *	for BitThunder.
 *
 *	This driver should be easily ported to SPI peripherals on other processors with little effort.
 *
 *	@author		Robert Steinbauer <rsteinbauer@riegl.com>
 *	@copyright	(c)2012 Robert Steinbauer
 *	@copyright	(c)2012 Riegl Laser Measurement Systems GmbH
 *
 **/
#include <bitthunder.h>	 				// Include all those wonderful BitThunder APIs.
#include "spi.h"						// Includes a full hardware definition for the integrated spis.
#include "rcc.h"						// Used for getting access to rcc regs, to determine the real Clock Freq.
#include "ioconfig.h"					// Used for getting access to IOCON regs.
#include <collections/bt_fifo.h>

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME						("LM3Sxx-SPI")
BT_DEF_MODULE_DESCRIPTION				("Simple SPI device for the LM3Sxx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/*
 * The modebits configurable by the driver to make the SPI support different
 * data formats
 */
#define MODEBITS                        (SPI_CPOL | SPI_CPHA)

/**
 *	We can define how a handle should look in a SPI driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;				///< All handles must include a handle header.
	LM3Sxx_SPI_REGS	   		   *pRegs;

	BT_SPI_MASTER				spi_master;

	const BT_INTEGRATED_DEVICE *pDevice;

	BT_u32 						speed_hz;
};

static BT_HANDLE g_SPI_HANDLES[2] = {
	NULL,
	NULL,
};

static const BT_IF_HANDLE oHandleInterface;	// Prototype for the spiOpen function.
static void disableSpiPeripheralClock(BT_HANDLE hSpi);


BT_ERROR BT_NVIC_IRQ_23(void) {
	return 0;
}

BT_ERROR BT_NVIC_IRQ_50(void) {
	return 0;
}


/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR spiCleanup(BT_HANDLE hSpi) {
	// Disable peripheral clock.
	disableSpiPeripheralClock(hSpi);

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	g_SPI_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

#define MAX_BAUD_ERROR_RATE	3	/* max % error allowed */

static BT_ERROR spiSetBaudrate(BT_HANDLE hSpi, BT_u32 ulBaudrate) {
	volatile LM3Sxx_SPI_REGS *pRegs = hSpi->pRegs;

	BT_u32 ulInputClk = BT_LM3Sxx_GetMainFrequency();

	pRegs->CR0 &= ~LM3Sxx_SPI_CR0_SCR_MASK;

	BT_u32 ulDivider = (ulInputClk / ulBaudrate) / 256;

	if (ulDivider < 2) ulDivider = 2;
	if (ulDivider % 2) ulDivider++;

	pRegs->CPSR = ulDivider;
	pRegs->CR0 |= ((ulInputClk / (ulBaudrate * ulDivider)-1) << 8) & LM3Sxx_SPI_CR0_SCR_MASK;

	return BT_ERR_NONE;
}

/**
 *	This actually allows the SPIS to be clocked!
 **/
static void enableSpiPeripheralClock(BT_HANDLE hSpi) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_SPI0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_SPI1EN;
		break;
	}
	default: {
		break;
	}
	}
}

/**
 *	If the serial port is not in use, we can make it sleep!
 **/
static void disableSpiPeripheralClock(BT_HANDLE hSpi) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_SPI0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_SPI1EN;
		break;
	}
	default: {
		break;
	}
	}
}

/**
 *	Function to test the current peripheral clock gate status of the devices.
 **/
static BT_BOOL isSpiPeripheralClockEnabled(BT_HANDLE hSpi) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_SPI0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_SPI1EN) {
			return BT_TRUE;
		}
		break;
	}
	default: {
		break;
	}
	}

	return BT_FALSE;
}

/**
 *	This implements the SPI power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR spiSetPowerState(BT_HANDLE hSpi, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableSpiPeripheralClock(hSpi);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableSpiPeripheralClock(hSpi);
		break;
	}

	default: {
		//return BT_ERR_POWER_STATE_UNSUPPORTED;
		return (BT_ERROR) -1;
	}
	}

	return BT_ERR_NONE;
}

/**
 *	This implements the SPI power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR spiGetPowerState(BT_HANDLE hSpi, BT_POWER_STATE *pePowerState) {
	if(isSpiPeripheralClockEnabled(hSpi)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

static void spi_chipselect(BT_HANDLE qspi, BT_SPI_DEVICE *pDevice, int is_on)
{
}

static BT_ERROR spiRead(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucDest, BT_u32 ulSize) {
	volatile LM3Sxx_SPI_REGS *pRegs = hSpi->pRegs;

	BT_ERROR Error = BT_ERR_NONE;
	volatile BT_u32 ulCount = 0;
	volatile BT_u32 ulLen = ulSize;

	while(ulSize) {
		if (pRegs->SR & LM3Sxx_SPI_SR_TNF) {
			pRegs->DR = 0xFF;
			ulSize--;
		}
		while ((pRegs->SR & LM3Sxx_SPI_SR_RNE)) {
			*pucDest++ = (BT_u8)(pRegs->DR & 0xFF);
			ulCount++;
		}
	}
	ulLen -= ulCount;
	while (ulLen) {
		if ((pRegs->SR & LM3Sxx_SPI_SR_RNE)) {
			*pucDest++ = (BT_u8)(pRegs->DR & 0xFF);
			ulLen--;
		}
	}

	return Error;
}

/**
 *	Implementing the CHAR dev write API.
 *
 *	Note, this doesn't implement ulFlags specific options yet!
 **/
static BT_ERROR spiWrite(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucSource, BT_u32 ulSize) {
	volatile LM3Sxx_SPI_REGS *pRegs = hSpi->pRegs;

	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 ulDummy;
	volatile BT_u32 ulCount = 0;
	volatile BT_u32 ulLen = ulSize;

	while(ulSize) {
		if (pRegs->SR & LM3Sxx_SPI_SR_TNF) {
			pRegs->DR = *pucSource++;
			ulSize--;
		}
		while ((pRegs->SR & LM3Sxx_SPI_SR_RNE)) {
			ulDummy = pRegs->DR;
			ulCount++;
		}
	}
	ulLen -= ulCount;
	while (ulLen) {
		if ((pRegs->SR & LM3Sxx_SPI_SR_RNE)) {
			ulDummy = pRegs->DR;
			ulLen--;
		}
	}

	return Error;
}


static BT_i32 spi_start_transfer(BT_HANDLE hSpi, BT_SPI_TRANSFER *transfer)
{
	if (transfer->tx_buf)
		spiWrite(hSpi, 0, (BT_u8*)transfer->tx_buf, transfer->len);
	if (transfer->rx_buf)
		spiRead(hSpi, 0, transfer->rx_buf, transfer->len);

	return (transfer->len);
}

/**
 *	Complete a full configuration of the SPI.
 **/
static BT_ERROR spi_setup_transfer(BT_HANDLE hSpi, BT_SPI_DEVICE *pDevice, BT_SPI_TRANSFER * transfer) {
	volatile LM3Sxx_SPI_REGS *pRegs = hSpi->pRegs;

	BT_u32 req_hz, bits;

	req_hz = (transfer) ? transfer->speed_hz : pDevice->max_speed_hz;

	if(pDevice->mode & ~MODEBITS) {
		BT_kPrint("spi: unsupported mode bits %x\n",pDevice->mode & ~MODEBITS);
		return BT_ERR_UNSUPPORTED_FLAG;
	}

	if(transfer && (transfer->speed_hz == 0)) {
		req_hz = pDevice->max_speed_hz;
	}

	bits = (transfer) ? transfer->bits_per_word : pDevice->bits_per_word;

	if(transfer && (transfer->bits_per_word == 0)) {
		bits = pDevice->bits_per_word;
	}

	/* Set the clock frequency */
	if(hSpi->speed_hz != req_hz) {
		hSpi->speed_hz = req_hz;
		spiSetBaudrate(hSpi, transfer->speed_hz);
	}

	/* Set the QSPI clock phase and clock polarity */
	pRegs->CR0 &= ~LM3Sxx_SPI_CR0_DSS_MASK;
	pRegs->CR0 |= (bits - 1);
	if (pDevice->mode & SPI_CPHA)
		pRegs->CR0 |= LM3Sxx_SPI_CR0_CPHA;
	if (pDevice->mode & SPI_CPOL)
		pRegs->CR0 |= LM3Sxx_SPI_CR0_CPOL;



	return BT_ERR_NONE;
}

BT_ERROR spi_transfer(BT_HANDLE hSpi, BT_SPI_MESSAGE * message) {

	BT_SPI_TRANSFER * transfer;
	int status = 0;
	unsigned			cs_change = 1;

	message->actual_length = 0;
	message->status = -1;

	/* Check each transfer's parameters */
	bt_list_for_each_entry(transfer, &message->transfers, transfer_list) {
		if (!transfer->tx_buf && !transfer->rx_buf && transfer->len)
			return -BT_ERR_INVALID_RESOURCE;
	}

	bt_list_for_each_entry(transfer, &message->transfers, transfer_list) {
		if (transfer->bits_per_word || transfer->speed_hz) {
			status = spi_setup_transfer(hSpi, message->spi_device, transfer);
			if (status != BT_ERR_NONE)
				break;
		}

		/* Select the chip if required */
		if (cs_change) {
			spi_chipselect(hSpi, message->spi_device, 1);
		}

		cs_change = transfer->cs_change;

		if(!transfer->tx_buf && !transfer->rx_buf && transfer->len) {
			status = BT_ERR_INVALID_VALUE;
			break;
		}

		/* Request the transfer */
		if (transfer->len) {
			status = spi_start_transfer(hSpi, transfer);
		}

		if (status != transfer->len) {
			//if (status > 0)
			//	status = -1;
			break;
		}
		message->actual_length += status;
		status = 0;

		//if (transfer->delay_usecs)
		//{/* FIXME: udelay(transfer->delay_usecs); */ }

		if (cs_change)
			/* Deselect the chip */
			spi_chipselect(hSpi, message->spi_device, 0);

		if (transfer->transfer_list.next == &message->transfers)
			break;
	}

	message->status = status;
	message->complete(message->context);

	spi_setup_transfer(hSpi, message->spi_device, NULL);

	if(!(status == 0 && cs_change))
		spi_chipselect(hSpi, message->spi_device, 0);

	return BT_ERR_NONE;
}

static BT_ERROR spi_setup(BT_HANDLE hspi, BT_SPI_DEVICE *pDevice)
{
	if (pDevice->mode & SPI_LSB_FIRST)
		return BT_ERR_INVALID_VALUE;

	if (!pDevice->max_speed_hz)
		return BT_ERR_INVALID_VALUE;

	if (!pDevice->bits_per_word)
		pDevice->bits_per_word = 8;

	return spi_setup_transfer(hspi, pDevice, NULL);
}

static BT_ERROR spi_setup(BT_HANDLE hspi, BT_SPI_DEVICE *pDevice)
{
	if (pDevice->mode & SPI_LSB_FIRST)
		return BT_ERR_INVALID_VALUE;

	if (!pDevice->max_speed_hz)
		return BT_ERR_INVALID_VALUE;

	if (!pDevice->bits_per_word)
		pDevice->bits_per_word = 8;

	return spi_setup_transfer(hspi, pDevice, NULL);
}


/**
 *	Make the SPI active (Set the Enable bit).
 **/
static BT_ERROR spi_init_hw(BT_HANDLE hSpi) {
	volatile LM3Sxx_SPI_REGS *pRegs = hSpi->pRegs;

	pRegs->CR1 |= LM3Sxx_SPI_CR1_SSP_ENABLE;

	return BT_ERR_NONE;
}

/**
 *	Make the SPI active (Set the Enable bit).
 **/
static BT_ERROR spi_init_hw(BT_HANDLE hSpi) {
	volatile LM3Sxx_SPI_REGS *pRegs = hSpi->pRegs;

	pRegs->CR1 |= LM3Sxx_SPI_CR1_SSP_ENABLE;
	spiSetBaudrate(hSpi, 20000000);

	return BT_ERR_NONE;
}

static const BT_DEV_IF_SPI oSpiConfigInterface = {
	.pfnSetup			= spi_setup,
	.pfnTransfer		= spi_transfer,
};

static const BT_IF_POWER oPowerInterface = {
	.pfnSetPowerState	= spiSetPowerState,											///< Pointers to the power state API implementations.
	.pfnGetPowerState	= spiGetPowerState,											///< This gets the current power state.
};

static const BT_DEV_IFS oConfigInterface = {
	(BT_DEV_INTERFACE) &oSpiConfigInterface,
};

const BT_IF_DEVICE BT_LM3Sxx_SPI_oDeviceInterface = {
	.pPowerIF = &oPowerInterface,											///< Device does not support powerstate functionality.
	.eConfigType = BT_DEV_IF_T_SPI,
	.unConfigIfs = {
		.pSpiIF = &oSpiConfigInterface,
	},
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_SPI_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup = spiCleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE spi_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hSpi = NULL;


	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_SPI_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hSpi->spi_master.bus_num = pResource->ulStart;

	hSpi = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hSpi) {
		goto err_out;
	}

	g_SPI_HANDLES[pResource->ulStart] = hSpi;

	hSpi->spi_master.bus_num = pResource->ulStart;

	hSpi->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hSpi->pRegs = (LM3Sxx_SPI_REGS *) pResource->ulStart;

	spiSetPowerState(hSpi, BT_POWER_STATE_AWAKE);

	// Reset all registers to their defaults
	spi_init_hw(hSpi);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, spi_irq_handler, hSpi);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	hSpi->spi_master.pDevice = pDevice;

	hSpi->spi_master.num_chipselect = 0;

	hSpi->spi_master.flags = 0;

	hSpi->speed_hz = 20000000;

	Error = BT_SpiRegisterMaster(hSpi, &hSpi->spi_master);
	if(Error != BT_ERR_NONE) {
		goto err_free_out;
	}

/*	BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName("mmc,spi");
	if(!pDriver) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	return pDriver->pfnProbe(pDevice, pError);*/


	return hSpi;

err_free_out:
	BT_DestroyHandle(hSpi);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF spi_driver = {
	.name 		= "LM3Sxx,spi",
	.pfnProbe	= spi_probe,
};

#ifdef BT_CONFIG_MACH_LM3Sxx_SPI_0
static const BT_RESOURCE oLM3Sxx_spi0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_SPI0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_SPI0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 23,
		.ulEnd				= 23,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

BT_INTEGRATED_DEVICE_DEF oLM3Sxx_spi0_device = {
	.name 					= "LM3Sxx,spi",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_spi0_resources),
	.pResources 			= oLM3Sxx_spi0_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_spi0_inode = {
	.szpName = "spi0",
	.pDevice = &oLM3Sxx_spi0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LM3Sxx_SPI_1
BT_RESOURCE oLM3Sxx_spi1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LM3Sxx_SPI1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LM3Sxx_SPI1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 50,
		.ulEnd				= 50,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

BT_INTEGRATED_DEVICE_DEF oLM3Sxx_spi1_device = {
	.name 					= "LM3Sxx,spi",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLM3Sxx_spi1_resources),
	.pResources 			= oLM3Sxx_spi1_resources,
};

const BT_DEVFS_INODE_DEF oLM3Sxx_spi1_inode = {
	.szpName = "spi1",
	.pDevice = &oLM3Sxx_spi1_device,
};
#endif
