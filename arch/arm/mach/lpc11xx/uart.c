/**
 *	LPC11xx Hal for BitThunder
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
BT_DEF_MODULE_NAME						("LPC11xx-USART")
BT_DEF_MODULE_DESCRIPTION				("Simple Uart device for the LPC11xx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/**
 *	We can define how a handle should look in a UART driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	LPC11xx_UART_REGS	   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;
	BT_UART_OPERATING_MODE	eMode;		///< Operational mode, i.e. buffered/polling mode.
	BT_HANDLE		   		hRxFifo;		///< RX fifo - ring buffer.
	BT_HANDLE		   		hTxFifo;		///< TX fifo - ring buffer.
};

static BT_HANDLE g_USART_HANDLES[1] = {
	NULL,
};

static BT_u8 TX_FIFO_LVL = 0;

static const BT_IF_HANDLE oHandleInterface;	// Protoype for the uartOpen function.
static void disableUartPeripheralClock(BT_HANDLE hUart);


static void usartRxHandler(BT_HANDLE hUart) {
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;
	BT_u8 ucData;

	BT_ERROR Error = BT_ERR_NONE;

	/* Receive Line Status */
	if (pRegs->LSR & (LPC11xx_UART_LSR_OE | LPC11xx_UART_LSR_PE | LPC11xx_UART_LSR_FE | LPC11xx_UART_LSR_RXFE | LPC11xx_UART_LSR_BI))
    {
		/* There are errors or break interrupt */
		/* Read LSR will clear the interrupt */
		volatile BT_u32 Dummy = pRegs->FIFO;	/* Dummy read on RX to clear
										interrupt, then bail out */
      return;
    }

	while (pRegs->LSR & LPC11xx_UART_LSR_RDR)
	{
		ucData = pRegs->FIFO & 0xFF;
		BT_FifoWrite(hUart->hRxFifo, 1, &ucData, &Error);
	}
}

static void usartTxHandler(BT_HANDLE hUart) {
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;
	BT_u8 ucData;

	BT_ERROR Error = BT_ERR_NONE;

	TX_FIFO_LVL = 0;

	while (!BT_FifoIsEmpty(hUart->hTxFifo, &Error) && (TX_FIFO_LVL < 16)) {
		BT_FifoRead(hUart->hTxFifo, 1, &ucData, &Error);
		pRegs->FIFO = ucData;
		TX_FIFO_LVL++;
	}
	if (BT_FifoIsEmpty(hUart->hTxFifo, &Error)) {
		pRegs->IER &= ~LPC11xx_UART_IER_THREIE;	// Disable the interrupt
	}
}

BT_ERROR BT_NVIC_IRQ_37(void) {
	BT_u32 IIRValue = UART0->IIR;

	if(IIRValue & (LPC11xx_UART_IIR_RDA_INT | LPC11xx_UART_IIR_RLS_INT)) {
		usartRxHandler(g_USART_HANDLES[0]);
	}
	if(IIRValue & LPC11xx_UART_IIR_THRE_INT) {
		usartTxHandler(g_USART_HANDLES[0]);
	}
	return 0;
}


static BT_ERROR uartDisable(BT_HANDLE hUart);
static BT_ERROR uartEnable(BT_HANDLE hUart);


static void ResetUart(BT_HANDLE hUart)
{
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;

	pRegs->LCR 		|= LPC11xx_UART_LCR_DLAB;
	pRegs->DLL		= 0;
	pRegs->DLM		= 0;
	pRegs->LCR 		&= ~LPC11xx_UART_LCR_DLAB;
	pRegs->IER		= 0;
	pRegs->FCR		= 0;
	pRegs->LCR 		= 0;
	pRegs->MCR 		= 0;
	pRegs->MSR		= 0;
	pRegs->SCR   	= 0;
	pRegs->ABCR		= 0;
	pRegs->FDR 		= 0x00000010;
	pRegs->TER		= 0x00000080;
	pRegs->RS485CR 	= 0;
	pRegs->RS485AMR = 0;
	pRegs->RS485DLY	= 0;

	TX_FIFO_LVL = 0;
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
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;

	BT_u16	IterBAUDDIV;
	BT_u8	MulVal;
	BT_u8	DivAddVal;
	BT_u32	Divider_Value;
	BT_u32	CalcBaudRate;
	BT_u32	BaudError;
	BT_u32	Best_Fractional = 0x10;
	BT_u8	Best_BAUDDIV = 0;
	BT_u32	Best_Error = 0xFFFFFFFF;
	BT_u32	PercentError;
	BT_u32	InputClk;
	BT_u32	BaudRate = ulBaudrate;

	/*
	 *	We must determine the input clock frequency to the UART peripheral.
	 */

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;
	/*
	 *	Determine the clock source!
	 */

	if (pRCC->UARTCLKDIV == 0)
		pRCC->UARTCLKDIV = 1;

	InputClk = BT_LPC11xx_GetMainFrequency() / pRCC->UARTCLKDIV;

	/*
	 * Determine the Baud divider. It can be 4to 254.
	 * Loop through all possible combinations
	 */

	/*
	 * Calculate the exact divider incl. fractional part
	 */
	Divider_Value = InputClk / (16 * ulBaudrate);

	for (IterBAUDDIV = Divider_Value/2; IterBAUDDIV <= Divider_Value; IterBAUDDIV++)
	{
		for (MulVal = 1; MulVal < 16; MulVal++)
		{
			for (DivAddVal = 0; DivAddVal < MulVal; DivAddVal++)
			{
				/*
				 * Calculate the baud rate from the BRGR value
				 */
				CalcBaudRate = ((InputClk * MulVal) / (16 * IterBAUDDIV * (MulVal + DivAddVal)));

				/*
				 * Avoid unsigned integer underflow
				 */
				if (BaudRate > CalcBaudRate) {
					BaudError = BaudRate - CalcBaudRate;
				} else {
					BaudError = CalcBaudRate - BaudRate;
				}

				/*
				 * Find the calculated baud rate closest to requested baud rate.
				 */
				if (Best_Error > BaudError)
				{
					Best_Fractional = (MulVal << 4) + DivAddVal;
					Best_BAUDDIV = IterBAUDDIV;
					Best_Error = BaudError;
				}
			}
		}
	}

	PercentError = (Best_Error * 100) / BaudRate;
	if (MAX_BAUD_ERROR_RATE < PercentError) {
		return -1;
	}

	uartDisable(hUart);

	pRegs->LCR |= LPC11xx_UART_LCR_DLAB;             /* Enable DLAB bit for setting Baudrate divider */
	pRegs->DLM = Best_BAUDDIV / 256;
	pRegs->DLL = Best_BAUDDIV % 256;
	pRegs->LCR &= ~LPC11xx_UART_LCR_DLAB;		/* DLAB = 0 */
	pRegs->FDR = Best_Fractional;

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
		LPC11xx_RCC->SYSAHBCLKCTRL |= LPC11xx_RCC_SYSAHBCLKCTRL_UARTEN;
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
		LPC11xx_RCC->SYSAHBCLKCTRL &= ~LPC11xx_RCC_SYSAHBCLKCTRL_UARTEN;
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
		if(LPC11xx_RCC->SYSAHBCLKCTRL & LPC11xx_RCC_SYSAHBCLKCTRL_UARTEN) {
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
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;
	BT_ERROR Error = BT_ERR_NONE;

	pRegs->LCR = (pConfig->ucDataBits - 5);
	pRegs->LCR |= (pConfig->ucStopBits) << 2;
	pRegs->LCR |= (pConfig->ucParity << 3);

	uartSetBaudrate(hUart, pConfig->ulBaudrate);

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
			pRegs->IER &= ~LPC11xx_UART_IER_RBRIE;	// Disable the interrupt

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

				pRegs->IER |= LPC11xx_UART_IER_RBRIE;	// Enable the interrupt
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
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;

	pConfig->eMode 			= hUart->eMode;

	BT_ERROR Error = BT_ERR_NONE;

	BT_u32 InputClk;

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	InputClk = BT_LPC11xx_GetMainFrequency() / pRCC->UARTCLKDIV;

	pRegs->LCR |= LPC11xx_UART_LCR_DLAB;
	BT_u32 Divider = (pRegs->DLM << 8) + pRegs->DLL;
	pRegs->LCR &= ~LPC11xx_UART_LCR_DLAB;

	BT_u32 MulVal 	 = (pRegs->FDR >> 4);
	BT_u32 DivAddVal = (pRegs->FDR & 0x0F);
	pConfig->ulBaudrate 	= ((InputClk * MulVal) / (16 * Divider * (MulVal + DivAddVal)));		// Clk / Divisor == ~Baudrate
	pConfig->ulTxBufferSize = BT_FifoSize(hUart->hTxFifo, &Error);
	pConfig->ulRxBufferSize = BT_FifoSize(hUart->hRxFifo, &Error);
	pConfig->ucDataBits 	= (pRegs->LCR & 0x00000003) + 5;
	pConfig->ucStopBits		= ((pRegs->LCR & 0x00000004) >> 2) + 1;
	pConfig->ucParity		= (BT_UART_PARITY_MODE)((pRegs->LCR & 0x00000380) >> 3);
	pConfig->eMode			= hUart->eMode;


	return Error;
}

/**
 *	Make the UART active (Set the Enable bit).
 **/
static BT_ERROR uartEnable(BT_HANDLE hUart) {
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;

	pRegs->TER |= LPC11xx_UART_TER_TXEN;		// Enable TX line.
	pRegs->FCR |= LPC11xx_UART_FCR_FIFO_ENB | LPC11xx_UART_FCR_RX_PURGE | LPC11xx_UART_FCR_TX_PURGE;		// Reset TX and RX FIFO's.

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	pRCC->UARTCLKDIV = 1;

	return BT_ERR_NONE;
}

/**
 *	Make the UART inactive (Clear the Enable bit).
 **/
static BT_ERROR uartDisable(BT_HANDLE hUart) {
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;

	pRegs->TER &= ~LPC11xx_UART_TER_TXEN;
	pRegs->FCR &= ~LPC11xx_UART_FCR_FIFO_ENB;

	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;

	pRCC->UARTCLKDIV = 0;

	return BT_ERR_NONE;
}


static BT_ERROR uartRead(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest) {
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;

	BT_ERROR Error = BT_ERR_NONE;

	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while((pRegs->LSR & LPC11xx_UART_LSR_RDR)) {
				BT_ThreadYield();
			}

			*pucDest++ = pRegs->FIFO & 0x000000FF;
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
	return BT_ERR_NONE;
}

/**
 *	Implementing the CHAR dev write API.
 *
 *	Note, this doesn't implement ulFlags specific options yet!
 **/
static BT_ERROR uartWrite(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, const BT_u8 *pucSource) {
	volatile LPC11xx_UART_REGS *pRegs = hUart->pRegs;

	BT_ERROR Error = BT_ERR_NONE;
	BT_u8 ucData;
	BT_u8 *pSrc = pucSource;

	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while(!(pRegs->LSR & LPC11xx_UART_LSR_THRE)) {
				BT_ThreadYield();
			}
			pRegs->FIFO = *pucSource++;
			ulSize--;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		BT_FifoWrite(hUart->hTxFifo, ulSize, pSrc, &Error);
		pRegs->IER |= LPC11xx_UART_IER_THREIE;	// Enable the interrupt

		while (!BT_FifoIsEmpty(hUart->hTxFifo, &Error) && (TX_FIFO_LVL < 16)) {
			BT_FifoRead(hUart->hTxFifo, 1, &ucData, &Error);
			pRegs->FIFO = ucData;
			TX_FIFO_LVL++;
		}
		break;
	}

	default:
		break;
	}
	return BT_ERR_NONE;
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

const BT_IF_DEVICE BT_LPC11xx_UART_oDeviceInterface = {
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
		(BT_HANDLE_INTERFACE) &BT_LPC11xx_UART_oDeviceInterface,
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

	hUart->pRegs = (LPC11xx_UART_REGS *) pResource->ulStart;

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
	BT_DestroyHandle(hUart);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF uart_driver = {
	.name 		= "LPC11xx,usart",
	.pfnProbe	= uart_probe,
};

