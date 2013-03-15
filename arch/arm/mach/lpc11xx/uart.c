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

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME						("LPC11xx-USART")
BT_DEF_MODULE_DESCRIPTION				("Simple Uart device for the LPC11xx Embedded Platform")
BT_DEF_MODULE_AUTHOR					("Robert Steinbauer")
BT_DEF_MODULE_EMAIL						("rsteinbauer@riegl.com")

/**
 *	This is a simple FIFO implementation for the UART driver.
 *	BT will eventually create optimised implementations of such common structures
 *	making it possible to make even simpler drivers.
 **/
typedef struct {
	BT_u8	*pBuf;						///< Pointer to the start of the ring-buffer.
	BT_u8	*pIn;						///< Input pointer.
	BT_u8	*pOut;						///< Output pointer.
	BT_u8	*pEnd;						///< Pointer to end of buffer.
} BT_UART_BUFFER;

/**
 *	We can define how a handle should look in a UART driver, probably we only need a
 *	hardware-ID number. (Remember, try to keep HANDLES as low-cost as possible).
 **/
struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 		h;			///< All handles must include a handle header.
	BT_u32					ulUartID;	///< UART device ID.
	BT_UART_OPERATING_MODE	eMode;		///< Operational mode, i.e. buffered/polling mode.
	BT_UART_BUFFER		   oRxBuf;		///< RX fifo - ring buffer.
	BT_UART_BUFFER		   oTxBuf;		///< TX fifo - ring buffer.
};

/**
 *	Now let's define an array of pointer's to the BASE_ADDRESS of each UART register block:
 *
 *	Such a declaration makes the pointers constant, i.e. in ROM, but allows the de-referenced
 *	values to be modified.
 *
 *	This is important, because we want this table to be placed in ROM by the linker, not our
 *	limited RAM! -- Remember RAM is for the USER application where possible!
 *
 *	The reason for having a table like this, is that its better for code density overall
 *	if we had to do a switch or if..else statement for detecting which base address to use
 *	we would require more code.
 *
 *	Using such a table allows use to quick index using the uartID number,
 *	and requires few instructions to achieve.
 *
 **/
static volatile LPC11xx_UART_REGS * const g_UARTS[] = {
	UART0,		// UART_0	-- These register definitions come from uart.h
};

static BT_HANDLE g_USART_HANDLES[sizeof(g_UARTS)/sizeof(LPC11xx_UART_REGS *)] = {
	NULL,
};

static const BT_IF_HANDLE oHandleInterface;	// Protoype for the uartOpen function.
static void disableUartPeripheralClock(BT_u32 nUartID);


static void usartRxHandler(int id) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[id];
	BT_HANDLE hUart = g_USART_HANDLES[id];

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
		*hUart->oRxBuf.pIn++ = pRegs->FIFO & 0xFF;
		if(hUart->oRxBuf.pIn >= hUart->oRxBuf.pEnd)
		{
			hUart->oRxBuf.pIn = hUart->oRxBuf.pBuf;
		}
	}
}

static void usartTxHandler(int id) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[id];
	BT_HANDLE hUart = g_USART_HANDLES[id];


	if(hUart->oTxBuf.pOut != hUart->oTxBuf.pIn) {
		pRegs->FIFO = *hUart->oTxBuf.pOut++;
		if(hUart->oTxBuf.pOut >= hUart->oTxBuf.pEnd) {
			hUart->oTxBuf.pOut = hUart->oTxBuf.pBuf;
		}
	} else {
		pRegs->IER &= ~LPC11xx_UART_IER_THREIE;	// Disable the interrupt
	}
}

BT_ERROR BT_NVIC_IRQ_37(void) {
	BT_u32 IIRValue = UART0->IIR;

	if(IIRValue & (LPC11xx_UART_IIR_RDA_INT | LPC11xx_UART_IIR_RLS_INT)) {
		usartRxHandler(0);
	}
	if(IIRValue & LPC11xx_UART_IIR_THRE_INT) {
		usartTxHandler(0);
	}
	return 0;
}


static BT_ERROR uartDisable(BT_HANDLE hUart);
static BT_ERROR uartEnable(BT_HANDLE hUart);


static void ResetUart(BT_HANDLE hUart)
{
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];

	pRegs->LCR 		|= LPC11xx_UART_LCR_DLAB;
	pRegs->DLL		= 0;
	pRegs->DLM		= 0;
	pRegs->LCR 		|= LPC11xx_UART_LCR_DLAB;
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
}

/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR uartCleanup(BT_HANDLE hUart) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];
	//BT_u32 ulSize;
	// Disable interrupts of USART module.
	//@@BT_DisableInterrupt()
	pRegs->IER &= ~LPC11xx_UART_IER_THREIE;	// Disable the interrupt

	ResetUart(hUart);

	// Disable peripheral clock.
	disableUartPeripheralClock(hUart->ulUartID);

	// Free any buffers if used.
	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		if(hUart->oRxBuf.pBuf) {
		 	BT_kFree(hUart->oRxBuf.pBuf);
		 	hUart->oRxBuf.pBuf = NULL;
		 	hUart->oRxBuf.pEnd = NULL;
		}
		if(hUart->oTxBuf.pBuf) {
		 	BT_kFree(hUart->oTxBuf.pBuf);
		 	hUart->oTxBuf.pBuf = NULL;
		 	hUart->oTxBuf.pEnd = NULL;
		}
	}

	g_USART_HANDLES[hUart->ulUartID] = NULL;	// Finally mark the hardware as not used.

	return BT_ERR_NONE;
}

/**
 *	Open doesn't do much, it simply creates the handle, after ensuring we are allowed
 *	to actually create a handle to the device.
 *
 *	i.e. its not already in use!
 **/
static BT_HANDLE uartOpen(BT_u32 nDeviceID, BT_ERROR *pError) {
	BT_HANDLE hUart;

	if(nDeviceID >= oHandleInterface.oIfs.pDevIF->ulTotalDevices) {	// Ensure we're not out of range!
		// ERR -- Invalid Device ID.							// BT should ensure this doesn't happen anyway!
		return NULL;
	}

	//lock();
	{
		if(g_USART_HANDLES[nDeviceID]) {							// This needs to be locked!
			// ERR -- Device In USE.
			return NULL;
		}

		hUart = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
		if(!hUart) {
			return NULL;
		}

		g_USART_HANDLES[nDeviceID] = hUart;							// Reserve the hardware in the handle table.
	}
	//unlock();

	hUart->ulUartID = nDeviceID;								// Set the device ID for further use in the API.

	// Reset all registers to their defaults!
	ResetUart(hUart);

	return hUart;
}

/**
 *	Now we come to the interesting part of the driver!
 *	Here we actually configure device registers appropriately e.g. set baudrates.
 *
 *	For cases like in STM32 where all peripherals a clock gated (NICE!! :D) we should return an
 *	error in case the device is not powered.
 *
 *	This should be managed by the device internally, (FOR now!) Later we should provide
 *	a Clock management API, which can be used internally by devices.
 *
 **/

#define MAX_BAUD_ERROR_RATE	3	/* max % error allowed */

static BT_ERROR uartSetBaudrate(BT_HANDLE hUart, BT_u32 ulBaudrate) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];

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
static void enableUartPeripheralClock(BT_u32 nUartID) {
	switch(nUartID) {
	case 0: {
		LPC11xx_RCC->SYSAHBCLKCTRL |= RCC_SYSAHBCLKCTRL_UARTEN;
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
static void disableUartPeripheralClock(BT_u32 nUartID) {
	switch(nUartID) {
	case 0: {
		LPC11xx_RCC->SYSAHBCLKCTRL &= ~RCC_SYSAHBCLKCTRL_UARTEN;
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
static BT_BOOL isUartPeripheralClockEnabled(BT_u32 nUartID) {
	switch(nUartID) {
	case 0: {
		if(LPC11xx_RCC->SYSAHBCLKCTRL & RCC_SYSAHBCLKCTRL_UARTEN) {
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
		disableUartPeripheralClock(hUart->ulUartID);
		break;
	}
	case BT_POWER_STATE_AWAKE: {
		enableUartPeripheralClock(hUart->ulUartID);
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
	if(isUartPeripheralClockEnabled(hUart->ulUartID)) {
		return BT_POWER_STATE_AWAKE;
	}
	return BT_POWER_STATE_ASLEEP;
}

/**
 *	Complete a full configuration of the UART.
 **/
static BT_ERROR uartSetConfig(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];

	pRegs->LCR = (pConfig->ucDataBits - 5);
	pRegs->LCR |= (pConfig->ucStopBits - 1) << 2;
	pRegs->LCR |= (pConfig->ucParity << 3);

	uartSetBaudrate(hUart, pConfig->ulBaudrate);

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hUart->eMode !=  BT_UART_MODE_POLLED) {
			if(hUart->oRxBuf.pBuf) {
				BT_kFree(hUart->oRxBuf.pBuf);
				hUart->oRxBuf.pBuf = NULL;
				hUart->oRxBuf.pEnd = NULL;
			}
			if(hUart->oTxBuf.pBuf) {
				BT_kFree(hUart->oTxBuf.pBuf);
				hUart->oTxBuf.pBuf = NULL;
				hUart->oTxBuf.pEnd = NULL;
			}

			// Disable TX and RX interrupts
			//@@BT_DisableInterrupt(hUart->)
			pRegs->IER &= ~LPC11xx_UART_IER_RBRIE;	// Disable the interrupt

			hUart->eMode = BT_UART_MODE_POLLED;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		if(hUart->eMode != BT_UART_MODE_BUFFERED) {
			if(!hUart->oRxBuf.pBuf && !hUart->oTxBuf.pBuf) {
				hUart->oRxBuf.pBuf 	= BT_kMalloc(pConfig->ulRxBufferSize);
				// Check malloc succeeded!
				hUart->oRxBuf.pIn 	= hUart->oRxBuf.pBuf;
				hUart->oRxBuf.pOut 	= hUart->oRxBuf.pBuf;
				hUart->oRxBuf.pEnd  = hUart->oRxBuf.pBuf + pConfig->ulRxBufferSize;

				hUart->oTxBuf.pBuf 	= BT_kMalloc(pConfig->ulTxBufferSize);
				// Check malloc succeeded!
				hUart->oTxBuf.pIn	= hUart->oTxBuf.pBuf;
				hUart->oTxBuf.pOut	= hUart->oTxBuf.pBuf;
				hUart->oTxBuf.pEnd 	= hUart->oTxBuf.pBuf + pConfig->ulTxBufferSize;

				pRegs->IER |= LPC11xx_UART_IER_RBRIE;	// Enable the interrupt
				hUart->eMode = BT_UART_MODE_BUFFERED;

				//@@BT_EnableInterrupt()
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
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];

	pConfig->eMode 			= hUart->eMode;

	BT_u32 InputClk;
	LPC11xx_RCC_REGS *pRCC = LPC11xx_RCC;
	/*
	 *	Determine the clock source!
	 */

	InputClk = BT_LPC11xx_GetMainFrequency() / pRCC->UARTCLKDIV;

	pRegs->LCR |= LPC11xx_UART_LCR_DLAB;
	BT_u32 Divider = (pRegs->DLM << 8) + pRegs->DLL;
	pRegs->LCR &= ~LPC11xx_UART_LCR_DLAB;

	BT_u32 MulVal 	 = (pRegs->FDR >> 4);
	BT_u32 DivAddVal = (pRegs->FDR & 0x0F);
	pConfig->ulBaudrate 	= ((InputClk * MulVal) / (16 * Divider * (MulVal + DivAddVal)));		// Clk / Divisor == ~Baudrate
	pConfig->ulTxBufferSize = (BT_u32) (hUart->oTxBuf.pEnd - hUart->oTxBuf.pBuf);
	pConfig->ulRxBufferSize = (BT_u32) (hUart->oRxBuf.pEnd - hUart->oRxBuf.pBuf);
	pConfig->ucDataBits 	= (pRegs->LCR & 0x00000003) + 5;
	pConfig->ucStopBits		= ((pRegs->LCR & 0x00000004) >> 2) + 1;
	pConfig->ucParity		= (BT_UART_PARITY_MODE)((pRegs->LCR & 0x00000380) >> 3);
	pConfig->eMode			= hUart->eMode;


	return BT_ERR_NONE;
}

/**
 *	Make the UART active (Set the Enable bit).
 **/
static BT_ERROR uartEnable(BT_HANDLE hUart) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];

	pRegs->TER |= LPC11xx_UART_TER_TXEN;		// Enable TX line.
	pRegs->FCR |= LPC11xx_UART_FCR_FIFO_ENB | LPC11xx_UART_FCR_RX_PURGE | LPC11xx_UART_FCR_TX_PURGE;		// Reset TX and RX FIFO's.

	return BT_ERR_NONE;
}

/**
 *	Make the UART inactive (Clear the Enable bit).
 **/
static BT_ERROR uartDisable(BT_HANDLE hUart) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];

	pRegs->TER &= ~LPC11xx_UART_TER_TXEN;
	pRegs->FCR &= ~LPC11xx_UART_FCR_FIFO_ENB;

	return BT_ERR_NONE;
}


static BT_ERROR uartRead(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest) {
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];
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
		while(ulSize) {
			if(hUart->oRxBuf.pOut != hUart->oRxBuf.pIn) {
				*pucDest++ = *hUart->oRxBuf.pOut++;
				if(hUart->oRxBuf.pOut >= hUart->oRxBuf.pEnd) {
					hUart->oRxBuf.pOut = hUart->oRxBuf.pBuf;
				}
				ulSize--;
			} else {
				BT_ThreadYield();
			}
		}
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
	volatile LPC11xx_UART_REGS *pRegs = g_UARTS[hUart->ulUartID];
	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while(pRegs->LSR & LPC11xx_UART_LSR_THRE) {
				BT_ThreadYield();
			}
			pRegs->FIFO = *pucSource++;
			ulSize--;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		while(ulSize) {
			// We should prevent overflow, and block!
			*hUart->oTxBuf.pIn++ = *pucSource++;
			if(hUart->oTxBuf.pIn >= hUart->oTxBuf.pEnd) {
				hUart->oTxBuf.pIn = hUart->oTxBuf.pBuf;
			}
			ulSize--;
			pRegs->IER |= LPC11xx_UART_IER_THREIE;	// Enable the interrupt
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
	sizeof(g_UARTS)/sizeof(LPC11xx_UART_REGS *),					///< Allow upto 3 uart instances!
	uartOpen,													///< Special Open interface.
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

/*BT_HANDLE uart_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;
	if(pError) {
		*pError = BT_ERR_NONE;
	}
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IO, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	BT_u32 base 	= pResource->ulStart;
	BT_u32 total 	= (pResource->ulEnd - pResource->ulStart) + 1;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	//Error = BT_EnableInterrupt(pResource->ulStart);

	Error = BT_RegisterGpioController(base, total, hGPIO);
	if(Error) {
		goto err_free_out;
	}

	return hGPIO;

err_free_out:
	BT_kFree(hGPIO);

	if(pError) {
		*pError = Error;
	}

err_out:
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF uart_driver = {
	.name 		= "LPC11xx,usart",
	.pfnProbe	= uart_probe,
};*/

static const BT_MODULE_ENTRY_DESCRIPTOR entryDescriptor = {
	(BT_s8 *) "usart",
	NULL,					///< No driver init function required!
	&oHandleInterface,
};

BT_MODULE_ENTRY(entryDescriptor);

