/**
 *	LPC11xx Hal for BitThunder
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
BT_DEF_MODULE_NAME						("LPC11xx-SPI")
BT_DEF_MODULE_DESCRIPTION				("Simple SPI device for the LPC11xx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a SPI driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LPC11xx_SPI_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_SPI_OPERATING_MODE	eMode;		///< Operational mode, i.e. buffered/polling mode.
	BT_u32					id;
	BT_HANDLE		   		hRxFifo;		///< RX fifo - ring buffer.
	BT_HANDLE		   		hTxFifo;		///< TX fifo - ring buffer.
};

static BT_HANDLE g_SPI_HANDLES[2] = {
	NULL,
	NULL,
};

static const BT_IF_HANDLE oHandleInterface;	// Prototype for the spiOpen function.
static void disableSpiPeripheralClock(BT_HANDLE hSpi);


BT_ERROR BT_NVIC_IRQ_30(void) {
	return 0;
}

BT_ERROR BT_NVIC_IRQ_36(void) {
	return 0;
}

static BT_ERROR spiDisable(BT_HANDLE hSpi);
static BT_ERROR spiEnable(BT_HANDLE hSpi);


static void ResetSpi(BT_HANDLE hSpi)
{
	volatile LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	if (hSpi->id == 0) {
		pRCC->PRESETCTRL &= LPC11xx_RCC_PRESETCTRL_SSP0_DEASSERT;
		pRCC->PRESETCTRL |= LPC11xx_RCC_PRESETCTRL_SSP0_DEASSERT;
	}
	if (hSpi->id == 1) {
		pRCC->PRESETCTRL &= LPC11xx_RCC_PRESETCTRL_SSP1_DEASSERT;
		pRCC->PRESETCTRL |= LPC11xx_RCC_PRESETCTRL_SSP1_DEASSERT;
	}
}

/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR spiCleanup(BT_HANDLE hSpi) {
	ResetSpi(hSpi);

	// Disable peripheral clock.
	disableSpiPeripheralClock(hSpi);

	// Free any buffers if used.
	if(hSpi->eMode == BT_SPI_MODE_BUFFERED) {
		BT_CloseHandle(hSpi->hTxFifo);
		BT_CloseHandle(hSpi->hRxFifo);
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	g_SPI_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

#define MAX_BAUD_ERROR_RATE	3	/* max % error allowed */

static BT_ERROR spiSetBaudrate(BT_HANDLE hSpi, BT_u32 ulBaudrate) {
	volatile LPC11xx_SPI_REGS *pRegs = hSpi->pRegs;

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	BT_u32 ulInputClk = BT_LPC11xx_GetMainFrequency();

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	switch (pResource->ulStart) {
	case 0: {
		if (pRCC->SSP0CLKDIV == 0)
			pRCC->SSP0CLKDIV = 1;

		ulInputClk /= pRCC->SSP0CLKDIV;
		break;
	}
	case 1: {
		if (pRCC->SSP1CLKDIV == 0)
			pRCC->SSP1CLKDIV = 1;

		ulInputClk /= pRCC->SSP1CLKDIV;
		break;
	}
	default: {
		break;
	}
	}
	pRegs->CR0 &= ~LPC11xx_SPI_CR0_SCR_MASK;

	BT_u32 ulDivider = (ulInputClk / ulBaudrate) / 256;

	if (ulDivider < 2) ulDivider = 2;
	if (ulDivider % 2) ulDivider++;

	pRegs->CPSR = ulDivider;
	pRegs->CR0 |= ((ulInputClk / (ulBaudrate * ulDivider)-1) << 8) & LPC11xx_SPI_CR0_SCR_MASK;

	spiEnable(hSpi);
	return BT_ERR_NONE;
}

/**
 *	This actually allows the SPIS to be clocked!
 **/
static void enableSpiPeripheralClock(BT_HANDLE hSpi) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LPC11xx_RCC->SYSAHBCLKCTRL |= LPC11xx_RCC_SYSAHBCLKCTRL_SSP0EN;
		break;
	}
	case 1: {
		LPC11xx_RCC->SYSAHBCLKCTRL |= LPC11xx_RCC_SYSAHBCLKCTRL_SSP1EN;
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
		LPC11xx_RCC->SYSAHBCLKCTRL &= ~LPC11xx_RCC_SYSAHBCLKCTRL_SSP0EN;
		break;
	}
	case 1: {
		LPC11xx_RCC->SYSAHBCLKCTRL &= ~LPC11xx_RCC_SYSAHBCLKCTRL_SSP1EN;
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
		if(LPC11xx_RCC->SYSAHBCLKCTRL & LPC11xx_RCC_SYSAHBCLKCTRL_SSP0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LPC11xx_RCC->SYSAHBCLKCTRL & LPC11xx_RCC_SYSAHBCLKCTRL_SSP1EN) {
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

/**
 *	Complete a full configuration of the SPI.
 **/
static BT_ERROR spiSetConfig(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig) {
	volatile LPC11xx_SPI_REGS *pRegs = hSpi->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	pRegs->CR0 = (pConfig->ucDataBits - 1) & LPC11xx_SPI_CR0_DSS_MASK;
	pRegs->CR0 |= pConfig->eCPOL << 6;
	pRegs->CR0 |= pConfig->eCPHA << 7;

	spiSetBaudrate(hSpi, pConfig->ulBaudrate);

	switch(pConfig->eMode) {
	case BT_SPI_MODE_POLLED: {
		if(hSpi->eMode !=  BT_SPI_MODE_POLLED) {

			if(hSpi->hTxFifo) {
				BT_CloseHandle(hSpi->hTxFifo);
				hSpi->hTxFifo = NULL;
			}
			if(hSpi->hRxFifo) {
				BT_CloseHandle(hSpi->hRxFifo);
				hSpi->hRxFifo = NULL;
			}

			// Disable TX and RX interrupts
			//@@pRegs->IER &= ~LPC11xx_SPI_IER_RBRIE;	// Disable the interrupt

			hSpi->eMode = BT_SPI_MODE_POLLED;
		}
		break;
	}

	case BT_SPI_MODE_BUFFERED:
	{
		if(hSpi->eMode != BT_SPI_MODE_BUFFERED) {
			if(!hSpi->hRxFifo && !hSpi->hTxFifo) {
				hSpi->hRxFifo = BT_FifoCreate(pConfig->ulRxBufferSize, 1, 0, &Error);
				hSpi->hTxFifo = BT_FifoCreate(pConfig->ulTxBufferSize, 1, 0, &Error);

				//@@pRegs->IER |= LPC11xx_SPI_IER_RBRIE;	// Enable the interrupt
				hSpi->eMode = BT_SPI_MODE_BUFFERED;
			}
		}
		break;
	}

	default:
		// Unsupported operating mode!
		break;
	}

	return BT_ERR_NONE;
}

/**
 *	Get a full configuration of the SPI.
 **/
static BT_ERROR spiGetConfig(BT_HANDLE hSpi, BT_SPI_CONFIG *pConfig) {
	volatile LPC11xx_SPI_REGS *pRegs = hSpi->pRegs;

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	BT_ERROR Error = BT_ERR_NONE;

	BT_u32 ulInputClk = BT_LPC11xx_GetMainFrequency();

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSpi->pDevice, BT_RESOURCE_ENUM, 0);

	switch (pResource->ulStart) {
	case 0: {
		if (pRCC->SSP0CLKDIV == 0)
			pRCC->SSP0CLKDIV = 1;

		ulInputClk /= pRCC->SSP0CLKDIV;
		break;
	}
	case 1: {
		if (pRCC->SSP1CLKDIV == 0)
			pRCC->SSP1CLKDIV = 1;

		ulInputClk /= pRCC->SSP1CLKDIV;
		break;
	}
	default: {
		break;
	}
	}

	pConfig->ulBaudrate 	= ulInputClk / (pRegs->CPSR * ((pRegs->CR0 & LPC11xx_SPI_CR0_SCR_MASK) >> 8));		// Clk / Divisor == ~Baudrate
	pConfig->ulTxBufferSize = BT_FifoSize(hSpi->hTxFifo);
	pConfig->ulRxBufferSize = BT_FifoSize(hSpi->hRxFifo);
	pConfig->ucDataBits 	= (pRegs->CR0 & LPC11xx_SPI_CR0_DSS_MASK) + 1;
	pConfig->eCPOL			= (pRegs->CR0 & LPC11xx_SPI_CR0_CPOL);
	pConfig->eCPHA			= (pRegs->CR0 & LPC11xx_SPI_CR0_CPHA);

	pConfig->eMode			= hSpi->eMode;


	return Error;
}

/**
 *	Make the SPI active (Set the Enable bit).
 **/
static BT_ERROR spiEnable(BT_HANDLE hSpi) {
	volatile LPC11xx_SPI_REGS *pRegs = hSpi->pRegs;

	pRegs->CR1 |= LPC11xx_SPI_CR1_SSP_ENABLE;		// Enable TX line.

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;
	if (hSpi->id == 0)
		pRCC->SSP0CLKDIV = 1;
	else if (hSpi->id == 1)
		pRCC->SSP1CLKDIV = 1;

	return BT_ERR_NONE;
}

/**
 *	Make the SPI inactive (Clear the Enable bit).
 **/
static BT_ERROR spiDisable(BT_HANDLE hSpi) {
	volatile LPC11xx_SPI_REGS *pRegs = hSpi->pRegs;

	pRegs->CR1 &= ~LPC11xx_SPI_CR1_SSP_ENABLE;		// Enable TX line.

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;
	if (hSpi->id == 0)
		pRCC->SSP0CLKDIV = 0;
	else if (hSpi->id == 1)
		pRCC->SSP1CLKDIV = 0;

	return BT_ERR_NONE;
}


static BT_ERROR spiRead(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucDest, BT_u32 ulSize) {
	volatile LPC11xx_SPI_REGS *pRegs = hSpi->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hSpi->eMode) {
	case BT_SPI_MODE_POLLED:
	{
		BT_u32 ulSend = ulSize;
		while(ulSize) {
			while((pRegs->SR & LPC11xx_SPI_SR_TNF) && (ulSend)) {
				pRegs->DR = 0;
				ulSend--;
			}
			while(!(pRegs->SR & LPC11xx_SPI_SR_RNE)) {
				BT_ThreadYield();
			}
			*pucDest++ = pRegs->DR & 0x0000FFFF;
			ulSize--;
		}
		break;
	}

	case BT_SPI_MODE_BUFFERED:
	{
		// Get bytes from RX buffer very quickly.
		//@@BT_FifoRead(hSpi->hRxFifo, ulSize, pucDest, 0);
		break;
	}

	default:
		// ERR, invalid handle configuration.
		break;
	}
	return Error;
}

/**
 *	Implementing the CHAR dev write API.
 *
 *	Note, this doesn't implement ulFlags specific options yet!
 **/
static BT_ERROR spiWrite(BT_HANDLE hSpi, BT_u32 ulFlags, BT_u8 *pucSource, BT_u32 ulSize) {
	volatile LPC11xx_SPI_REGS *pRegs = hSpi->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hSpi->eMode) {
	case BT_SPI_MODE_POLLED:
	{
		while(ulSize) {
			while(!(pRegs->SR & LPC11xx_SPI_SR_TNF)) {
				BT_ThreadYield();
			}
			pRegs->DR = *pucSource++;
			ulSize--;
		}
		break;
	}

	case BT_SPI_MODE_BUFFERED:
	{
		//@@BT_FifoWrite(hSpi->hTxFifo, ulSize, pSrc, 0);
		//@@pRegs->IER |= LPC11xx_SPI_IER_THREIE;	// Enable the interrupt

		break;
	}

	default:
		break;
	}
	return Error;
}

static const BT_DEV_IF_SPI oSpiConfigInterface = {
	.pfnSetBaudrate		= spiSetBaudrate,											///< SPI setBaudrate implementation.
	.pfnSetConfig		= spiSetConfig,												///< SPI set config imple.
	.pfnGetConfig		= spiGetConfig,
	.pfnEnable			= spiEnable,													///< Enable/disable the device.
	.pfnDisable			= spiDisable,
	.pfnRead			= spiRead,
	.pfnWrite			= spiWrite,
};

static const BT_IF_POWER oPowerInterface = {
	spiSetPowerState,											///< Pointers to the power state API implementations.
	spiGetPowerState,											///< This gets the current power state.
};

static const BT_DEV_IFS oConfigInterface = {
	(BT_DEV_INTERFACE) &oSpiConfigInterface,
};

const BT_IF_DEVICE BT_LPC11xx_SPI_oDeviceInterface = {
	&oPowerInterface,											///< Device does not support powerstate functionality.
	BT_DEV_IF_T_SPI,											///< Allow configuration through the SPI api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oSpiConfigInterface,
	},
	NULL,											///< Provide a Character device interface implementation.
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LPC11xx_SPI_oDeviceInterface,
	},
	.eType		= BT_HANDLE_T_DEVICE,											///< Handle Type!
	.pfnCleanup	= spiCleanup,												///< Handle's cleanup routine.
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

	hSpi = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hSpi) {
		goto err_out;
	}

	hSpi->id = pResource->ulStart;

	g_SPI_HANDLES[pResource->ulStart] = hSpi;

	hSpi->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hSpi->pRegs = (LPC11xx_SPI_REGS *) pResource->ulStart;

	spiSetPowerState(hSpi, BT_POWER_STATE_AWAKE);

	// Reset all registers to their defaults!

	ResetSpi(hSpi);

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
	.name 		= "LPC11xx,spi",
	.pfnProbe	= spi_probe,
};

#ifdef BT_CONFIG_MACH_LPC11xx_SPI_0
static const BT_RESOURCE oLPC11xx_spi0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC11xx_SPI0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC11xx_SPI0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 36,
		.ulEnd				= 36,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC11xx_spi0_device = {
	.name 					= "LPC11xx,spi",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC11xx_spi0_resources),
	.pResources 			= oLPC11xx_spi0_resources,
};

const BT_DEVFS_INODE_DEF oLPC11xx_spi0_inode = {
	.szpName = "spi0",
	.pDevice = &oLPC11xx_spi0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC11xx_SPI_1
static const BT_RESOURCE oLPC11xx_spi1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC11xx_SPI1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC11xx_SPI1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 30,
		.ulEnd				= 30,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC11xx_spi1_device = {
	.name 					= "LPC11xx,spi",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC11xx_spi1_resources),
	.pResources 			= oLPC11xx_spi1_resources,
};

const BT_DEVFS_INODE_DEF oLPC11xx_spi1_inode = {
	.szpName = "spi1",
	.pDevice = &oLPC11xx_spi1_device,
};
#endif
