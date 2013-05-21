/**
 *	ZYNQ Hal for BitThunder
 *	UART Driver Implementation.
 *
 *	This driver serves as robust example as to how implement a fully functional UART device driver
 *	for BitThunder.
 *
 *	This driver should be easily ported to UART peripherals on other processors with little effort.
 *
 *	@author		James Walmsley <jwalmsley@riegl.com>
 *	@copyright	(c)2012 James Walmsley
 *	@copyright	(c)2012 Riegl Laser Measurement Systems GmbH
 *
 **/
#include <bitthunder.h>	 				// Include all those wonderful BitThunder APIs.
#include "uart.h"						// Includes a full hardware definition for the integrated uarts.
#include "slcr.h"						// Used for getting access to slcr regs, to determine the real Clock Freq.

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME						("ZYNQ-USART")
BT_DEF_MODULE_DESCRIPTION				("Simple Uart device for the Zynq Embedded Platform")
BT_DEF_MODULE_AUTHOR					("James Walmsley")
BT_DEF_MODULE_EMAIL						("jwalmsley@riegl.com")

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
	ZYNQ_UART_REGS		   *pRegs;
	const BT_INTEGRATED_DEVICE   *pDevice;

	struct _BT_OPAQUE_HANDLE	*pNext;

	BT_UART_OPERATING_MODE	eMode;		///< Operational mode, i.e. buffered/polling mode.
	BT_UART_BUFFER		   oRxBuf;		///< RX fifo - ring buffer.
	BT_UART_BUFFER		   oTxBuf;		///< TX fifo - ring buffer.
};

static const BT_IF_HANDLE oHandleInterface;	// Protoype for the uartOpen function.

static BT_ERROR uart_irq_handler(BT_u32 ulIRQ, void *pParam) {

	return BT_ERR_NONE;
}
/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR uartCleanup(BT_HANDLE hUart) {

	// Free any buffers if used.
	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		if(hUart->oRxBuf.pBuf) {
		 	//ulSize = (BT_u32) (hUart->oRxBuf.pEnd - hUart->oRxBuf.pBuf);
		 	//BT_Free(ulSize, hUart->oRxBuf.pBuf);
		 	hUart->oRxBuf.pBuf = NULL;
		 	hUart->oRxBuf.pEnd = NULL;
		}
		if(hUart->oTxBuf.pBuf) {
		 	//ulSize = (BT_u32) (hUart->oTxBuf.pEnd - hUart->oTxBuf.pBuf);
		 	//BT_Free(ulSize, hUart->oTxBuf.pBuf);
		 	hUart->oTxBuf.pBuf = NULL;
		 	hUart->oTxBuf.pEnd = NULL;
		}
	}

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_IRQ, 0);

	BT_UnregisterInterrupt(pResource->ulStart, uart_irq_handler, hUart);

	return BT_ERR_NONE;
}


/**
 *	Now we come to the interersting part of the driver!
 *	Here we actually configure device registers appropriately e.g. set baudrates.
 *
 *	For cases like in STM32 where all peripherals a clock gated (NICE!! :D) we should return an
 *	error in case the device is not powered.
 *
 *	This should be managed by the device internally, (FOR now!) Later we should provide
 *	a Clock management API, which can be used internally by devices.
 *
 **/

static BT_ERROR uartDisable(BT_HANDLE hUart);
static BT_ERROR uartEnable(BT_HANDLE hUart);

#define MAX_BAUD_ERROR_RATE	3	/* max % error allowed */

static BT_ERROR uartSetBaudrate(BT_HANDLE hUart, BT_u32 ulBaudrate) {
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	BT_u8	IterBAUDDIV;
	BT_u32	BRGR_Value;
	BT_u32	CalcBaudRate;
	BT_u32	BaudError;
	BT_u32	Best_BRGR = 0;
	BT_u8	Best_BAUDDIV = 0;
	BT_u32	Best_Error = 0xFFFFFFFF;
	BT_u32	PercentError;
	BT_u32	InputClk;
	BT_u32	BaudRate = ulBaudrate;

	/*
	 *	We must determine the input clock frequency to the UART peripheral.
	 */

	volatile ZYNQ_SLCR_REGS *pSLCR = ZYNQ_SLCR;
	/*
	 *	Determine the clock source!
	 */

	BT_u32 clk_sel = ZYNQ_SLCR_CLK_CTRL_SRCSEL_VAL(pSLCR->UART_CLK_CTRL);
	switch(clk_sel) {
	case ZYNQ_SLCR_CLK_CTRL_SRCSEL_ARMPLL:
		InputClk = BT_ZYNQ_GetArmPLLFrequency();
		break;
	case ZYNQ_SLCR_CLK_CTRL_SRCSEL_IOPLL:
		InputClk = BT_ZYNQ_GetIOPLLFrequency();
		break;

	case ZYNQ_SLCR_CLK_CTRL_SRCSEL_DDRPLL:
		InputClk = BT_ZYNQ_GetDDRPLLFrequency();
		break;

	default:
		return -1;

	}

	InputClk /= ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(pSLCR->UART_CLK_CTRL);

	/*
	 * Determine the Baud divider. It can be 4to 254.
	 * Loop through all possible combinations
	 */
	for (IterBAUDDIV = 4; IterBAUDDIV < 255; IterBAUDDIV++) {

		/*
		 * Calculate the value for BRGR register
		 */
		BRGR_Value = InputClk / (BaudRate * (IterBAUDDIV + 1));

		/*
		 * Calculate the baud rate from the BRGR value
		 */
		CalcBaudRate = InputClk/ (BRGR_Value * (IterBAUDDIV + 1));

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
		if (Best_Error > BaudError) {

			Best_BRGR = BRGR_Value;
			Best_BAUDDIV = IterBAUDDIV;
			Best_Error = BaudError;
		}
	}

	PercentError = (Best_Error * 100) / BaudRate;
	if (MAX_BAUD_ERROR_RATE < PercentError) {
		return -1;
	}

	uartDisable(hUart);

	pRegs->BAUDGEN = Best_BRGR;
	pRegs->BAUDDIV = Best_BAUDDIV;

	pRegs->MR = 0x20;

	// STM32 uarts support fractional baudrate division :D so we do some fixed-point arithmetic

	//pRegs->BAUDGEN

	//pRegs->BAUDDIV = (((UART_FREQ/pRegs->BAUDGEN) << 4) / ulBaudrate / 16) & 0x0000FFFF;

	uartEnable(hUart);
	return BT_ERR_NONE;
}

/**
 *	This implements the UART power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR uartSetPowerState(BT_HANDLE hUart, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {

		break;
	}
	case BT_POWER_STATE_AWAKE: {

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
	/*if(isUartPeripheralClockEnabled(hUart->ulUartID)) {
		return BT_POWER_STATE_AWAKE;
	}*/
	return BT_POWER_STATE_AWAKE;
}

/**
 *	Complete a full configuration of the UART.
 **/
static BT_ERROR uartSetConfig(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {

	uartSetBaudrate(hUart, pConfig->ulBaudrate);

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hUart->eMode !=  BT_UART_MODE_POLLED) {
			if(hUart->oRxBuf.pBuf) {
				//ulSize = (BT_u32) (hUart->oRxBuf.pEnd - hUart->oRxBuf.pBuf);
				//BT_Free(ulSize, hUart->oRxBuf.pBuf);
				hUart->oRxBuf.pBuf = NULL;
				hUart->oRxBuf.pEnd = NULL;
			}
			if(hUart->oTxBuf.pBuf) {
				//ulSize = (BT_u32) (hUart->oTxBuf.pEnd - hUart->oTxBuf.pBuf);
				//BT_Free(ulSize, hUart->oTxBuf.pBuf);
				hUart->oTxBuf.pBuf = NULL;
				hUart->oTxBuf.pEnd = NULL;
			}

			// Disable TX and RX interrupts
//			pRegs->CR1 &= ~USART_CR1_RXNEIE;
			//BT_unregisterInterrupt();
			hUart->eMode = BT_UART_MODE_POLLED;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		if(hUart->eMode != BT_UART_MODE_BUFFERED) {
			if(!hUart->oRxBuf.pBuf && !hUart->oTxBuf.pBuf) {
				//hUart->oRxBuf.pBuf 	= BT_Malloc(pConfig->ulRxBufferSize);
				// Check malloc succeeded!
				hUart->oRxBuf.pIn 	= hUart->oRxBuf.pBuf;
				hUart->oRxBuf.pOut 	= hUart->oRxBuf.pBuf;
				hUart->oRxBuf.pEnd  = hUart->oRxBuf.pBuf + pConfig->ulRxBufferSize;

				//hUart->oTxBuf.pBuf 	= BT_Malloc(pConfig->ulTxBufferSize);
				// Check malloc succeeded!
				hUart->oTxBuf.pIn	= hUart->oTxBuf.pBuf;
				hUart->oTxBuf.pOut	= hUart->oTxBuf.pBuf;
				hUart->oTxBuf.pEnd 	= hUart->oTxBuf.pBuf + pConfig->ulTxBufferSize;

				//pRegs->CR1 |= USART_CR1_RXNEIE;
				hUart->eMode = BT_UART_MODE_BUFFERED;

				//BT_RegisterInterrupt(UART0_IRQ, USART1_IRQHandler);

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
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	pConfig->eMode 			= hUart->eMode;

	BT_u32 InputClk;
	volatile ZYNQ_SLCR_REGS *pSLCR = ZYNQ_SLCR;
	/*
	 *	Determine the clock source!
	 */

	BT_u32 clk_sel = ZYNQ_SLCR_CLK_CTRL_SRCSEL_VAL(pSLCR->UART_CLK_CTRL);
	switch(clk_sel) {
	case ZYNQ_SLCR_CLK_CTRL_SRCSEL_ARMPLL:
		InputClk = BT_ZYNQ_GetArmPLLFrequency();
		break;
	case ZYNQ_SLCR_CLK_CTRL_SRCSEL_IOPLL:
		InputClk = BT_ZYNQ_GetIOPLLFrequency();
		break;

	case ZYNQ_SLCR_CLK_CTRL_SRCSEL_DDRPLL:
		InputClk = BT_ZYNQ_GetDDRPLLFrequency();
		break;

	default:
		return -1;

	}

	InputClk /= ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(pSLCR->UART_CLK_CTRL);

	pConfig->ulBaudrate 	= (InputClk / pRegs->BAUDDIV);		// Clk / Divisor == ~Baudrate
	pConfig->ulTxBufferSize = (BT_u32) (hUart->oTxBuf.pEnd - hUart->oTxBuf.pBuf);
	pConfig->ulRxBufferSize = (BT_u32) (hUart->oRxBuf.pEnd - hUart->oRxBuf.pBuf);
	pConfig->ucDataBits 	= 8;

	return BT_ERR_NONE;
}

/**
 *	Make the UART active (Set the Enable bit).
 **/
static BT_ERROR uartEnable(BT_HANDLE hUart) {
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	pRegs->CR 	= ZYNQ_UART_CR_RXEN  | ZYNQ_UART_CR_TXEN;		// Enable TX and RX lines.
	pRegs->CR  |= ZYNQ_UART_CR_RXRES | ZYNQ_UART_CR_TXRES;		// Reset TX and RX data paths in uart logic.
	pRegs->CR  |= ZYNQ_UART_CR_STPBRK;							// Stop transmission break enabled.
	pRegs->CR  |= ZYNQ_UART_CR_RSTTO;							// Restart the receiver Timeout counter.

	return BT_ERR_NONE;
}

/**
 *	Make the UART inactive (Clear the Enable bit).
 **/
static BT_ERROR uartDisable(BT_HANDLE hUart) {
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	pRegs->CR |= ZYNQ_UART_CR_RXDIS | ZYNQ_UART_CR_TXDIS;

	return BT_ERR_NONE;
}


/**
 *	Now things get finally really interesting! Here we can read data for the USER API.
 *	The next 4 functions are implementing the CHAR-device interface.
 *
 *	We can also implement BLK-device and FILE/STREAM I/O interfaces.
 *
 *	Each I/O interface will be used by different Application Level API's.
 **/

static BT_ERROR uartRead(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest) {
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;
	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while((pRegs->SR & ZYNQ_UART_SR_RXEMPTY)) {
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
				//BT_ThreadYield();
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
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;
	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while(pRegs->SR & ZYNQ_UART_SR_TXFULL) {
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
			//pRegs->CR1 |= USART_CR1_TXEIE;	// Enable the interrupt
		}

		break;
	}

	default:
		break;
	}
	return BT_ERR_NONE;
}

static BT_ERROR uartFlush(BT_HANDLE hUart) {
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	while((pRegs->SR & ZYNQ_UART_SR_TXACTIVE) || !(pRegs->SR & ZYNQ_UART_SR_TXEMPTY)) {
		BT_ThreadYield();
	}

	return BT_ERR_NONE;
}

/**
 *	A driver doesn't have to implement all API's all at once, therefore we left the boring
 *	GETCH/PUTCH interfaces.
 **/
/*
static BT_ERROR uartGetch(BT_HANDLE hUart, BT_u32 ulFlags) {
	return BT_ERR_NONE;
}

static BT_ERROR uartPutch(BT_HANDLE hUart, BT_u32 ulFlags, BT_u8 ucData) {
	return BT_ERR_NONE;
}
*/

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
	.pfnRead = uartRead,										///< CH device read function.
	.pfnWrite = uartWrite,										///< CH device write function.
	.pfnFlush = uartFlush,
};

static const BT_DEV_IFS oConfigInterface = {
	(BT_DEV_INTERFACE) &oUartConfigInterface,
};

static const BT_IF_DEVICE oDeviceInterface = {
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
		(BT_HANDLE_INTERFACE) &oDeviceInterface,
	},
	BT_HANDLE_T_DEVICE,											///< Handle Type!
	uartCleanup,												///< Handle's cleanup routine.
};

static BT_HANDLE uart_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hUart;

	hUart = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hUart) {
		goto err_out;
	}

	hUart->pDevice = pDevice;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hUart->pRegs = (ZYNQ_UART_REGS *) pResource->ulStart;

	hUart->pRegs->CR	= 0x00000128;
	hUart->pRegs->MR 	= 0;
	hUart->pRegs->IER 	= 0;
	hUart->pRegs->IDR 	= 0;

	hUart->pRegs->BAUDGEN	= 0x0000028B;
	hUart->pRegs->RXTOUT   = 0;
	hUart->pRegs->RXTRIG	= 0;
	hUart->pRegs->MODEMCR 	= 0;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	Error = BT_RegisterInterrupt(pResource->ulStart, uart_irq_handler, hUart);
	if(Error) {
		goto err_free_out;
	}

	Error = BT_EnableInterrupt(pResource->ulStart);

	return hUart;

/*err_free_irq:
	BT_UnregisterInterrupt(pResource->ulStart, uart_irq_handler, hUart);*/

err_free_out:
	BT_DestroyHandle(hUart);

err_out:
	if(pError) {
		*pError = Error;
	}
	return NULL;
}

BT_INTEGRATED_DRIVER_DEF gpio_driver = {
	.name 		= "zynq,uart",
	.pfnProbe	= uart_probe,
};
