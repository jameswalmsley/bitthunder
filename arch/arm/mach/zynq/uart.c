/**
 *	ZYNQ Hal for BitThunder
 *	UART Driver Implementation.
 *
 *	This driver serves as robust example as to how implement a fully functional UART device driver
 *	for BlueThunder.
 *
 *	This driver should be easily ported to UART peripherals on other processors with little effort.
 *
 *	@author		James Walmsley <jwalmsley@riegl.com>
 *	@copyright	(c)2012 James Walmsley
 *	@copyright	(c)2012 Riegl Laser Measurement Systems GmbH
 *
 **/
#include <bitthunder.h>			 	// Include all those wonderful BitThunder APIs.
#include "zynq_hw.h"				// Hardware definitions for Zynq

/**
 *	All driver modules in the system shall be tagged with some helpful information.
 *	This way we know who to blame when things go wrong!
 **/
BT_DEF_MODULE_NAME			("ZYNQ-USART")
BT_DEF_MODULE_DESCRIPTION	("Simple Uart device for the Zynq Embedded Platform")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("jwalmsley@riegl.com")

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
 *	Here we define the register layout.
 *	Any decent micro-processor should have its registers aligned, and together will
 *	almost no reserved spaces in the register layouts.
 *
 *	Thats one reason for choosing ARM  (ST) -- Simple Drivers!
 **/
typedef struct {
	BT_u32 	CR;			//		0x00  -- Control register.
	BT_u32	MR;			//		0x04  -- Mode Register
	BT_u32	IER;		//      0x08  -- Interrupt Enable Register.
	BT_u32	IDR;        //      0x0C  -- Interrupt Disable Register.
	BT_u32	IMR;        //      0x10  -- Interrupt Mask register.
	BT_u32	ISR;        //      0x14  -- Interrupt Status register.
	BT_u32	BAUDGEN;    //      0x18  -- Baud rate generator.
	BT_u32	RXTOUT;     //      0x1C  -- RX timeout register.
	BT_u32	RXWM;       //      0x20  -- RX fifo trigger level.
	BT_u32	MODEMCR;    //      0x24  -- Modem control register.
	BT_u32	MODEMSR;    //      0x28  -- Modem status register.
	BT_u32	SR;         //      0x2C  -- Channel status register.
	BT_u32	FIFO;       //      0x30  -- TX_RX fifo.
	BT_u32	BAUDDIV;    //      0x34  -- Baudrate divider.
	BT_u32	FLOWDEL;    //      0x38  -- Flow delay register
	BT_u32	RESERVED[2];//      0x3C, 0x40
	BT_u32	TXWM;       //      0x44  -- TX fifo level trigger register.
} ZYNQ_UART_REGISTERS;

/**
 *	Now let's define an array of pointer's to the BASE_ADDRESS of each UART register block:
 *
 *	Such a declaration makes the pointers constant, i.e. in ROM, but allows the de-referenced
 *	values to be modified.
 *
 *	This is important, because we want this table to be placed in ROM by the linker, not our
 *	limited RAM! -- Remember RAM is for the USER application where possible!
 **/
static volatile ZYNQ_UART_REGISTERS * const g_UARTS[] = {
	(ZYNQ_UART_REGISTERS *) (0xE0000000),		// USART_1
	(ZYNQ_UART_REGISTERS *) (0xE0001000),		// USART_2
};

/**
 *	Probably no STM32 goes over 8 UARTS in a chip, so we can use a CHAR here.
 *	Its just for flagging if a device is already opened! I.e. prevent multiple
 *	instances of the same UART module.
 **/
//static BT_u8 g_bmpInUse = 0;				// Used for determining if a UART device is already in use.
static BT_HANDLE g_USART_HANDLES[sizeof(g_UARTS)/sizeof(ZYNQ_UART_REGISTERS *)] = {
	NULL,
	NULL,
};

static const BT_IF_HANDLE oHandleInterface;	// Protoype for the uartOpen function.
static void disableUartPeripheralClock(BT_u32 nUartID);

/*
static void usartRxHandler(int id) {
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[id];
	BT_HANDLE hUart = g_USART_HANDLES[id];

	*hUart->oRxBuf.pIn++ = pRegs->FIFO & 0xFF;
	if(hUart->oRxBuf.pIn >= hUart->oRxBuf.pEnd) {
		hUart->oRxBuf.pIn = hUart->oRxBuf.pBuf;
	}
}

static void usartTxHandler(int id) {
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[id];
	BT_HANDLE hUart = g_USART_HANDLES[id];

	if(hUart->oTxBuf.pOut != hUart->oTxBuf.pIn) {
		pRegs->FIFO = *hUart->oTxBuf.pOut++;
		if(hUart->oTxBuf.pOut >= hUart->oTxBuf.pEnd) {
			hUart->oTxBuf.pOut = hUart->oTxBuf.pBuf;
		}
	} else {
		//pRegs->CR1 &= ~USART_CR1_TXEIE;
	}
}
*/
BT_ERROR USART1_IRQHandler(void) {
	/*if(USART1->SR & USART_SR_RXNE) {
		usartRxHandler(0);
	}
	if(USART1->SR & USART_SR_TXE) {
		usartTxHandler(0);
	}*/
	return 0;
}

void USART2_IRQHandler(void) {
	/*if(USART1->SR & USART_SR_RXNE) {
		usartRxHandler(1);
	}*/
	/*if(USART1->SR & USART_SR_TXE) {
		usartTxHandler(1);
	}*/
}

void USART3_IRQHandler(void) {
	/*if(USART1->SR & USART_SR_RXNE) {
		usartRxHandler(2);
	}*/
	/*if(USART1->SR & USART_SR_TXE) {
		usartTxHandler(2);
	}*/
}

/**
 *	All modules MUST provide a FULL cleanup function. This must cleanup all allocations etc
 *	e.g. we will have to populate this when we add circular buffers!
 *
 **/
static BT_ERROR uartCleanup(BT_HANDLE hUart) {
	//volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];
	//BT_u32 ulSize;
	// Disable interrupts of USART module.
	//pRegs->CR1 &= ~(USART_CR1_TXEIE | USART_CR1_RXNEIE);

	// Disable the device.
	//pRegs->CR1 = 0;

	// Disable peripheral clock.
	disableUartPeripheralClock(hUart->ulUartID);

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

	if(nDeviceID >= oHandleInterface.pIfs->pDevIF->ulTotalDevices) {	// Ensure we're not out of range!
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
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];
	pRegs->CR	= 0x00000128;
	pRegs->MR 	= 0;
	pRegs->IER 	= 0;
	pRegs->IDR 	= 0;
	//pRegs->IMR 	= 0;
	//pRegs->ISR  = 0x00000200;
	pRegs->BAUDGEN	= 0x0000028B;
	pRegs->RXTOUT   = 0;
	pRegs->RXWM		= 0;
	pRegs->MODEMCR 	= 0;

	return hUart;
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
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];

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

	InputClk = UART_FREQ;

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
 *	This actually allows the UARTS to be clocked!
 **/
static void enableUartPeripheralClock(BT_u32 nUartID) {
	switch(nUartID) {
	case 0: {
		//RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
		break;
	}

	case 1: {
		//RCC->APB2ENR |= RCC_APB1ENR_USART2EN;
		break;
	}

	case 2: {
	//	RCC->APB2ENR |= RCC_APB1ENR_USART3EN;
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
		//RCC->APB2ENR &= ~RCC_APB2ENR_USART1EN;
		break;
	}

	case 1: {
		//RCC->APB2ENR &= ~RCC_APB1ENR_USART2EN;
		break;
	}

	case 2: {
//		RCC->APB2ENR &= ~RCC_APB1ENR_USART3EN;
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
		/*if(RCC->APB2ENR & RCC_APB2ENR_USART1EN) {
			return BT_TRUE;
		}*/
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
	//volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];
//	BT_u32 ulSize;

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
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];

	pConfig->eMode 			= hUart->eMode;
	pConfig->ulBaudrate 	= (UART_FREQ / pRegs->BAUDDIV);		// Clk / Divisor == ~Baudrate
	pConfig->ulTxBufferSize = (BT_u32) (hUart->oTxBuf.pEnd - hUart->oTxBuf.pBuf);
	pConfig->ulRxBufferSize = (BT_u32) (hUart->oRxBuf.pEnd - hUart->oRxBuf.pBuf);
	pConfig->ucDataBits 	= 8;

	return BT_ERR_NONE;
}

/**
 *	Make the UART active (Set the Enable bit).
 **/
static BT_ERROR uartEnable(BT_HANDLE hUart) {
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];


	pRegs->CR  = XUARTPS_CR_RX_EN | XUARTPS_CR_TX_EN;				// Enable Rx and TX lines.
	pRegs->CR  |= XUARTPS_CR_TXRST | XUARTPS_CR_RXRST;
	pRegs->CR  |= XUARTPS_CR_STOPBRK;
	pRegs->CR  |= XUARTPS_CR_TORST;

	return BT_ERR_NONE;
}

/**
 *	Make the UART inactive (Clear the Enable bit).
 **/
static BT_ERROR uartDisable(BT_HANDLE hUart) {
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];

	pRegs->CR |= XUARTPS_CR_RX_DIS | XUARTPS_CR_TX_DIS;

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

#define XUartPs_IsReceiveData(BaseAddress)			 \
	!((Xil_In32((BaseAddress) + XUARTPS_SR_OFFSET) & 	\
	XUARTPS_SR_RXEMPTY) == XUARTPS_SR_RXEMPTY)

static BT_ERROR uartRead(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, BT_u8 *pucDest) {
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];
	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while((pRegs->SR & XUARTPS_SR_RXEMPTY)) {
				//BT_ThreadYield();
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
	volatile ZYNQ_UART_REGISTERS *pRegs = g_UARTS[hUart->ulUartID];
	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while(pRegs->SR & XUARTPS_SR_TXFULL) {
				//BT_ThreadYield();
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

static const BT_CONFIG_IF_UART oUartConfigInterface = {
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
	NULL,//uartGetch,											///< This API wasn't implemented in this driver.
	NULL,//uartPutch,											///< :: Therefore the pointer must be NULL for BT to handle.
};

static const BT_DEV_CONFIG_IFS oConfigInterface = {
	(BT_INTERFACE) &oUartConfigInterface,
};

static const BT_IF_DEVICE oDeviceInterface = {
	sizeof(g_UARTS)/sizeof(ZYNQ_UART_REGISTERS *),				///< Allow upto 3 uart instances!
	uartOpen,													///< Special Open interface.
	&oPowerInterface,											///< Device does not support powerstate functionality.
	BT_DEV_CONFIG_IF_UART,												///< Allow configuration through the UART api.
	&oConfigInterface,											///< This is the implementation of UART configuration API.
//	&oCharDevInterface,											///< Provide a Character device interface implementation.
};


static const BT_UN_IFS oDevIF = {
	(BT_HANDLE_INTERFACE) &oDeviceInterface,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_NAME,												///< Provides the standard module name in ROM.
	BT_MODULE_DESCRIPTION,
	BT_MODULE_AUTHOR,
	BT_MODULE_EMAIL,
	&oDevIF,													///< Pointer to a Device interface if its a device.
	BT_HANDLE_T_DEVICE,											///< Handle Type!
	uartCleanup,												///< Handle's cleanup routine.
};

static BT_ERROR driver_init(void) {
	return BT_ERR_NONE;
}

static const BT_MODULE_ENTRY_DESCRIPTOR entryDescriptor = {
	(BT_s8 *) "usart",
	driver_init,
	&oHandleInterface,
};

BT_MODULE_ENTRY(entryDescriptor);


