/**
 *	LM3Sxx Hal for BitThunder
 *	UART Driver Implementation.
 *
 *	This driver serves as robust example as to how implement a fully functional UART device driver
 *	for BitThunder.
 *
 *	This driver should be easily ported to UART peripherals on other processors with little effort.
 *
 *	@author		Robert Steinbauer <rsteinbauer@riegl.com>
 *	@copyright	(c)2012 Robert Steinbauer
 *	@copyright	(c)2012 Riegl Laser Measurement Systems GmbH
 *
 **/
#include <bitthunder.h>	 				// Include all those wonderful BitThunder APIs.
#include "uart.h"						// Includes a full hardware definition for the integrated uarts.
#include "rcc.h"						// Used for getting access to rcc regs, to determine the real Clock Freq.
#include "ioconfig.h"						// Used for getting access to IOCON regs.
#include <collections/bt_fifo.h>

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME						("LM3Sxx-USART")
BT_DEF_MODULE_DESCRIPTION				("Simple Uart device for the LM3Sxx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a UART driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LM3Sxx_UART_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_UART_OPERATING_MODE	eMode;		///< Operational mode, i.e. buffered/polling mode.
	BT_HANDLE		   		hRxFifo;		///< RX fifo - ring buffer.
	BT_HANDLE		   		hTxFifo;		///< TX fifo - ring buffer.
};

static BT_HANDLE g_USART_HANDLES[3] = {
	NULL,
	NULL,
	NULL,
};


static const BT_IF_HANDLE oHandleInterface;	// Protoype for the uartOpen function.
static void disableUartPeripheralClock(BT_HANDLE hUart);

static void usartRxHandler(BT_HANDLE hUart) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;
	BT_u8 ucData;

	BT_ERROR Error = BT_ERR_NONE;

	while (!(pRegs->FR & LM3Sxx_UART_FR_RXFE))	{
		ucData = pRegs->DR & 0xFF;
		BT_FifoWrite(hUart->hRxFifo, 1, &ucData, &Error);
	}
}

static void usartTxHandler(BT_HANDLE hUart) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;
	BT_u8 ucData;

	BT_ERROR Error = BT_ERR_NONE;

	while (!BT_FifoIsEmpty(hUart->hTxFifo, &Error) && (!(pRegs->FR & LM3Sxx_UART_FR_TXFF))) {
		BT_FifoRead(hUart->hTxFifo, 1, &ucData, &Error);
		pRegs->DR = ucData;
	}
	if (BT_FifoIsEmpty(hUart->hTxFifo, &Error)) {
		pRegs->IM &= ~LM3Sxx_UART_INT_TX;	// Disable the interrupt
	}
}

BT_ERROR BT_NVIC_IRQ_21(void) {
	BT_HANDLE hUart = g_USART_HANDLES[0];
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	// Get and clear the current interrupt source(s)
	BT_u32 ulIntStatus = pRegs->MIS;
	pRegs->ICR = ulIntStatus;

	if(ulIntStatus & (LM3Sxx_UART_INT_RX | LM3Sxx_UART_INT_RT)) {
		usartRxHandler(hUart);
	}
	if(ulIntStatus & LM3Sxx_UART_INT_TX) {
		usartTxHandler(hUart);
	}
	return 0;
}

BT_ERROR BT_NVIC_IRQ_22(void) {
	BT_HANDLE hUart = g_USART_HANDLES[1];
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	// Get and clear the current interrupt source(s)
	BT_u32 ulIntStatus = pRegs->MIS;
	pRegs->ICR = ulIntStatus;

	if(ulIntStatus & (LM3Sxx_UART_INT_RX | LM3Sxx_UART_INT_RT)) {
		usartRxHandler(hUart);
	}
	if(ulIntStatus & LM3Sxx_UART_INT_TX) {
		usartTxHandler(hUart);
	}
	return 0;
}
BT_ERROR BT_NVIC_IRQ_49(void) {
	BT_HANDLE hUart = g_USART_HANDLES[2];
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	// Get and clear the current interrupt source(s)
	BT_u32 ulIntStatus = pRegs->MIS;
	pRegs->ICR = ulIntStatus;

	if(ulIntStatus & (LM3Sxx_UART_INT_RX | LM3Sxx_UART_INT_RT)) {
		usartRxHandler(hUart);
	}
	if(ulIntStatus & LM3Sxx_UART_INT_TX) {
		usartTxHandler(hUart);
	}
	return 0;
}


static BT_ERROR uartDisable(BT_HANDLE hUart);
static BT_ERROR uartEnable(BT_HANDLE hUart);


static void ResetUart(BT_HANDLE hUart)
{
}

/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR uartCleanup(BT_HANDLE hUart) {
	ResetUart(hUart);

	// Disable peripheral clock.
	disableUartPeripheralClock(hUart);

	// Free any buffers if used.
	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		BT_CloseHandle(hUart->hTxFifo);
		BT_CloseHandle(hUart->hRxFifo);
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);

	pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_ENUM, 0);

	g_USART_HANDLES[pResource->ulStart] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

#define MAX_BAUD_ERROR_RATE	3	/* max % error allowed */

static BT_ERROR uartSetBaudrate(BT_HANDLE hUart, BT_u32 ulBaudrate) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	BT_u32	ulInputClk;
	BT_u32	BaudRate = ulBaudrate;

	/*
	 *	We must determine the input clock frequency to the UART peripheral.
	 */

	ulInputClk = BT_LM3Sxx_GetSystemFrequency();

	uartDisable(hUart);

	/*
	 * Determine the Baud divider. It can be 4to 254.
	 * Loop through all possible combinations
	 */

    // Is the required baud rate greater than the maximum rate supported
    // without the use of high speed mode?
    if((BaudRate * 16) > ulInputClk) {
        // Enable high speed mode.
        pRegs->CTL |= LM3Sxx_UART_CTL_HSE;

        // Half the supplied baud rate to compensate for enabling high speed
        // mode.  This allows the following code to be common to both cases.
        BaudRate /= 2;
    }
    else {
        // Disable high speed mode.
    	pRegs->CTL &= ~(LM3Sxx_UART_CTL_HSE);
    }

    // Compute the fractional baud rate divider.
    BT_u32 ulDiv = (((ulInputClk * 8) / BaudRate) + 1) / 2;

    // Set the baud rate.
    pRegs->IBRD = ulDiv / 64;
    pRegs->FBRD = ulDiv % 64;

    // Start the UART.
	uartEnable(hUart);
	return BT_ERR_NONE;
}

/**
 *	This actually allows the UARTS to be clocked!
 **/
static void enableUartPeripheralClock(BT_HANDLE hUart) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_UART0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_UART1EN;
		break;
	}
	case 2: {
		LM3Sxx_RCC->RCGC[1] |= LM3Sxx_RCC_RCGC_UART2EN;
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
static void disableUartPeripheralClock(BT_HANDLE hUart) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_UART0EN;
		break;
	}
	case 1: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_UART1EN;
		break;
	}
	case 2: {
		LM3Sxx_RCC->RCGC[1] &= ~LM3Sxx_RCC_RCGC_UART2EN;
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
static BT_BOOL isUartPeripheralClockEnabled(BT_HANDLE hUart) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_ENUM, 0);

	switch(pResource->ulStart) {
	case 0: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_UART0EN) {
			return BT_TRUE;
		}
		break;
	}
	case 1: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_UART1EN) {
			return BT_TRUE;
		}
		break;
	}
	case 2: {
		if(LM3Sxx_RCC->RCGC[1] & LM3Sxx_RCC_RCGC_UART2EN) {
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
 *	This implements the UART power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR uartSetPowerState(BT_HANDLE hUart, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		disableUartPeripheralClock(hUart);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableUartPeripheralClock(hUart);
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
 *	This implements the UART power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR uartGetPowerState(BT_HANDLE hUart, BT_POWER_STATE *pePowerState) {
	if(isUartPeripheralClockEnabled(hUart)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/**
 *	Complete a full configuration of the UART.
 **/
static BT_ERROR uartSetConfig(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	uartSetBaudrate(hUart, pConfig->ulBaudrate);

	uartDisable(hUart);

    // Set parity, data length, and number of stop bits.
    pRegs->LCRH = (pConfig->ucDataBits - 5) << 5;
	pRegs->LCRH |= (pConfig->ucStopBits) << 3;
	if (pConfig->ucParity == BT_UART_PARITY_ODD)   pRegs->LCRH |= LM3Sxx_UART_LCRH_ODD;
	if (pConfig->ucParity == BT_UART_PARITY_EVEN)  pRegs->LCRH |= LM3Sxx_UART_LCRH_EVEN;
	if (pConfig->ucParity == BT_UART_PARITY_MARK)  pRegs->LCRH |= LM3Sxx_UART_LCRH_MARK;
	if (pConfig->ucParity == BT_UART_PARITY_SPACE) pRegs->LCRH |= LM3Sxx_UART_LCRH_SPACE;

    // Clear the flags register.
    pRegs->FR = 0;

    uartEnable(hUart);

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hUart->eMode !=  BT_UART_MODE_POLLED) {

			if(hUart->hTxFifo) {
				BT_CloseHandle(hUart->hTxFifo);
				hUart->hTxFifo = NULL;
			}
			if(hUart->hRxFifo) {
				BT_CloseHandle(hUart->hRxFifo);
				hUart->hRxFifo = NULL;
			}

			// Disable TX and RX interrupts
			pRegs->IM &= ~(LM3Sxx_UART_INT_RX | LM3Sxx_UART_INT_RT);	// Disable the interrupt

			hUart->eMode = BT_UART_MODE_POLLED;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		if(hUart->eMode != BT_UART_MODE_BUFFERED) {
			if(!hUart->hRxFifo && !hUart->hTxFifo) {
				hUart->hRxFifo = BT_FifoCreate(pConfig->ulRxBufferSize, 1, 0, &Error);
				hUart->hTxFifo = BT_FifoCreate(pConfig->ulTxBufferSize, 1, 0, &Error);

				pRegs->IM |= LM3Sxx_UART_INT_RX | LM3Sxx_UART_INT_RT;	// Enable the interrupt
				hUart->eMode = BT_UART_MODE_BUFFERED;
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
 *	Get a full configuration of the UART.
 **/
static BT_ERROR uartGetConfig(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	pConfig->eMode 			= hUart->eMode;

	BT_ERROR Error = BT_ERR_NONE;

	BT_u32 ulInputClk = BT_LM3Sxx_GetSystemFrequency();

    // Compute the baud rate.
    pConfig->ulBaudrate = (ulInputClk * 4) / ((64 * pRegs->IBRD) + pRegs->FBRD);
	pConfig->ulTxBufferSize = BT_FifoSize(hUart->hTxFifo, &Error);
	pConfig->ulRxBufferSize = BT_FifoSize(hUart->hRxFifo, &Error);
	pConfig->ucDataBits 	= ((pRegs->LCRH >> 5) & 0x00000003) + 5;
	pConfig->ucStopBits		= ((pRegs->LCRH >> 3) & 0x00000001) + 1;
	if ((pRegs->LCRH & 0x00000086) == LM3Sxx_UART_LCRH_EVEN) pConfig->ucParity = BT_UART_PARITY_EVEN;
	else if ((pRegs->LCRH & 0x00000086) == LM3Sxx_UART_LCRH_MARK) pConfig->ucParity = BT_UART_PARITY_MARK;
	else if ((pRegs->LCRH & 0x00000086) == LM3Sxx_UART_LCRH_SPACE) pConfig->ucParity = BT_UART_PARITY_SPACE;
	else if ((pRegs->LCRH & 0x00000086) == LM3Sxx_UART_LCRH_ODD) pConfig->ucParity = BT_UART_PARITY_ODD;
	else pConfig->ucParity = BT_UART_PARITY_NONE;
	pConfig->eMode			= hUart->eMode;


	return Error;
}

/**
 *	Make the UART active (Set the Enable bit).
 **/
static BT_ERROR uartEnable(BT_HANDLE hUart) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;


    // Enable the FIFO.
	pRegs->LCRH |= LM3Sxx_UART_LCRH_FEN;

    // Enable RX, TX, and the UART.
	pRegs->CTL |= (LM3Sxx_UART_CTL_UARTEN | LM3Sxx_UART_CTL_TXE | LM3Sxx_UART_CTL_RXE);

	return BT_ERR_NONE;
}

/**
 *	Make the UART inactive (Clear the Enable bit).
 **/
static BT_ERROR uartDisable(BT_HANDLE hUart) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

    // Wait for end of TX.
    while(pRegs->FR & LM3Sxx_UART_FR_BUSY) {
    	BT_ThreadYield();
    }

    // Disable the FIFO.
    pRegs->LCRH &= ~(LM3Sxx_UART_LCRH_FEN);

    // Disable the UART.
    pRegs->CTL &= ~(LM3Sxx_UART_CTL_UARTEN | LM3Sxx_UART_CTL_TXE | LM3Sxx_UART_CTL_RXE);

    return BT_ERR_NONE;
}


static BT_ERROR uartRead(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while((pRegs->FR & LM3Sxx_UART_FR_RXFE)) {
				BT_ThreadYield();
			}

			*pucDest++ = pRegs->DR & 0x000000FF;
			ulSize--;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		// Get bytes from RX buffer very quickly.
		BT_FifoRead(hUart->hRxFifo, ulSize, pucDest, &Error);
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
static BT_ERROR uartWrite(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource) {
	volatile LM3Sxx_UART_REGS *pRegs = hUart->pRegs;

	BT_ERROR Error = BT_ERR_NONE;
	BT_u8 ucData;
	BT_u8 *pSrc = (BT_u8*)pucSource;

	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while((pRegs->FR & LM3Sxx_UART_FR_TXFF)) {
				BT_ThreadYield();
			}
			pRegs->DR = *pucSource++;
			ulSize--;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		BT_FifoWrite(hUart->hTxFifo, ulSize, pSrc, &Error);

		while (!BT_FifoIsEmpty(hUart->hTxFifo, &Error) && (!(pRegs->FR & LM3Sxx_UART_FR_TXFF))) {
			BT_FifoRead(hUart->hTxFifo, 1, &ucData, &Error);
			pRegs->DR = ucData;
		}
		pRegs->IM |= LM3Sxx_UART_INT_TX;	// Enable the interrupt
		break;
	}

	default:
		break;
	}
	return Error;
}

/**
 *	A driver doesn't have to implement all API's all at once, therefore we left the boring
 *	GETCH/PUTCH interfaces.
 **/

static BT_ERROR uartGetch(BT_HANDLE hUart, BT_u32 ulFlags) {
	return BT_ERR_NONE;
}

static BT_ERROR uartPutch(BT_HANDLE hUart, BT_u32 ulFlags, BT_u8 ucData) {
	return BT_ERR_NONE;
}


static const BT_DEV_IF_UART oUartConfigInterface = {
	uartSetBaudrate,											///< UART setBaudrate implementation.
	uartSetConfig,												///< UART set config imple.
	uartGetConfig,
	uartEnable,													///< Enable/disable the device.
	uartDisable,
};

static const BT_IF_POWER oPowerInterface = {
	uartSetPowerState,											///< Pointers to the power state API implementations.
	uartGetPowerState,											///< This gets the current power state.
};

static const BT_IF_CHARDEV oCharDevInterface = {
	uartRead,													///< CH device read function.
	uartWrite,													///< CH device write function.
	uartGetch,													///< This API wasn't implemented in this driver.
	uartPutch,													///< :: Therefore the pointer must be NULL for BT to handle.
};

static const BT_DEV_IFS oConfigInterface = {
	(BT_DEV_INTERFACE) &oUartConfigInterface,
};

const BT_IF_DEVICE BT_LM3Sxx_UART_oDeviceInterface = {
	&oPowerInterface,											///< Device does not support powerstate functionality.
	BT_DEV_IF_T_UART,											///< Allow configuration through the UART api.
	.unConfigIfs = {
		(BT_DEV_INTERFACE) &oUartConfigInterface,
	},
	&oCharDevInterface,											///< Provide a Character device interface implementation.
};


static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &BT_LM3Sxx_UART_oDeviceInterface,
	},
	BT_HANDLE_T_DEVICE,											///< Handle Type!
	uartCleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE uart_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hUart = NULL;


	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	if (g_USART_HANDLES[pResource->ulStart]){
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hUart = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hUart) {
		goto err_out;
	}

	g_USART_HANDLES[pResource->ulStart] = hUart;

	hUart->pDevice = pDevice;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_out;
	}

	hUart->pRegs = (LM3Sxx_UART_REGS *) pResource->ulStart;

	uartSetPowerState(hUart, BT_POWER_STATE_AWAKE);

	// Reset all registers to their defaults!

	ResetUart(hUart);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

/*	On NVIC we don't need to register interrupts, LINKER has patched vector for us
 * Error = BT_RegisterInterrupt(pResource->ulStart, uart_irq_handler, hUart);
	if(Error) {
		goto err_free_out;
	}*/


	Error = BT_EnableInterrupt(pResource->ulStart);

	return hUart;

err_free_out:
	BT_kFree(hUart);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF uart_driver = {
	.name 		= "LM3Sxx,usart",
	.pfnProbe	= uart_probe,
};

