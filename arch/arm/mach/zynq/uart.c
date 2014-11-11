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
 *
 **/
#include <bitthunder.h>
#include <collections/bt_fifo.h>
#include "uart.h"
#include "slcr.h"

BT_DEF_MODULE_NAME				("ZYNQ-USART")
BT_DEF_MODULE_DESCRIPTION		("Simple Uart device for the Zynq Embedded Platform")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("jwalmsley@riegl.com")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	ZYNQ_UART_REGS		   	   *pRegs;
	const BT_INTEGRATED_DEVICE *pDevice;
	BT_UART_OPERATING_MODE		eMode;
	BT_HANDLE					hRxFifo;
	BT_HANDLE					hTxFifo;

	volatile BT_u32				uRxBegin;
	volatile BT_u32				uRxEnd;
	volatile BT_u32				uRxSize;
	volatile BT_u8				*RxBuffer;

	volatile BT_u32				uTxBegin;
	volatile BT_u32				uTxEnd;
	volatile BT_u32				uTxSize;
	volatile BT_u8	   			*TxBuffer;

	BT_u32						ulIRQ;

};

static const BT_IF_HANDLE oHandleInterface;

static BT_ERROR uart_irq_handler(BT_u32 ulIRQ, void *pParam) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hUart = (BT_HANDLE) pParam;

	BT_u32 isr = hUart->pRegs->ISR;
	BT_u32 data;

	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		if((isr & ZYNQ_UART_IXR_TOUT) || (isr & ZYNQ_UART_IXR_RXTRIG)) {
			while(!(hUart->pRegs->SR & ZYNQ_UART_SR_RXEMPTY)) {
				data = hUart->pRegs->FIFO;
				BT_u8 ucData = (BT_u8) data;
				BT_FifoWriteFromISR(hUart->hRxFifo, 1, &ucData);
			}
		}

		if(isr & ZYNQ_UART_IXR_TXEMPTY) {
			if(BT_FifoIsEmpty(hUart->hTxFifo, &Error)) {	// Transmission complete, disable XMIT interrupts.
				hUart->pRegs->IDR = ZYNQ_UART_IXR_TXEMPTY;
			} else {
				BT_u32 numbytes = 16;
				while(numbytes--) {
					if(BT_FifoIsEmpty(hUart->hTxFifo, &Error)) {
						break;
					}

					BT_u8 ucData;
					BT_FifoReadFromISR(hUart->hTxFifo, 1, &ucData);
					hUart->pRegs->FIFO = ucData;
				}
			}
		}
	}
	else if(hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		if((isr & ZYNQ_UART_IXR_TOUT) || (isr & ZYNQ_UART_IXR_RXTRIG)) {
			while(!(hUart->pRegs->SR & ZYNQ_UART_SR_RXEMPTY)) {
				data = hUart->pRegs->FIFO;
				BT_u8 ucData = (BT_u8) data;

				if (((hUart->uRxEnd+1) != hUart->uRxBegin) && (!((hUart->uRxEnd == (hUart->uRxSize-1)) && (hUart->uRxBegin == 0)))) {
					if (++hUart->uRxEnd > (hUart->uRxSize-1)) {
						hUart->uRxEnd = 0;
					}
					hUart->RxBuffer[hUart->uRxEnd] = ucData;
				}
			}
		}

		if(isr & ZYNQ_UART_IXR_TXEMPTY) {
			while(((hUart->pRegs->SR & ZYNQ_UART_SR_TXFULL) == 0) && (hUart->uTxEnd != hUart->uTxBegin)) {
				hUart->pRegs->FIFO = hUart->TxBuffer[hUart->uTxBegin];
				if (++hUart->uTxBegin > (hUart->uTxSize-1)) {
					hUart->uTxBegin = 0;
				}
			}
		}
	}

	hUart->pRegs->ISR = isr;

	return Error;
}

static BT_ERROR uart_disable(BT_HANDLE hUart);
static BT_ERROR uart_enable(BT_HANDLE hUart);

static BT_ERROR uart_cleanup(BT_HANDLE hUart) {

	uart_disable(hUart);

	BT_UnregisterInterrupt(hUart->ulIRQ, uart_irq_handler, hUart);

	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		if(hUart->hRxFifo) {
			BT_CloseHandle(hUart->hRxFifo);
			hUart->hRxFifo = NULL;
		}
		if(hUart->hTxFifo) {
		 	BT_CloseHandle(hUart->hTxFifo);
			hUart->hTxFifo = NULL;
		}
	}
	else if(hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		if (hUart->RxBuffer) {
			BT_kFree((void *)hUart->RxBuffer);
			hUart->RxBuffer = NULL;
		}
		if (hUart->TxBuffer) {
			BT_kFree((void *)hUart->TxBuffer);
			hUart->TxBuffer = NULL;
		}
	}

	return BT_ERR_NONE;
}

static BT_ERROR uart_set_baudrate(BT_HANDLE hUart, BT_u32 ulBaudrate) {
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	BT_u32	InputClk;
	BT_u32	BaudRate = ulBaudrate;

	volatile ZYNQ_SLCR_REGS *pSLCR = bt_ioremap((void *)ZYNQ_SLCR, BT_SIZE_4K);

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

	BT_DIVIDER_PARAMS oDiv;
	oDiv.diva_max = 65536;
	oDiv.diva_min = 1;
	oDiv.divb_min = 4;
	oDiv.divb_max = 255;

	BT_CalculateClockDivider(InputClk, BaudRate, &oDiv);	// Calculate 2 stage divider.

	//uart_disable(hUart);

	pRegs->BAUDGEN = oDiv.diva_val;							// Apply the calculated divider values.
	pRegs->BAUDDIV = oDiv.divb_val;

	//uart_enable(hUart);
	return BT_ERR_NONE;
}

/**
 *	This implements the UART power management interface.
 *	It is called from the BT_SetPowerState() API!
 **/
static BT_ERROR uart_set_power_state(BT_HANDLE hUart, BT_POWER_STATE ePowerState) {

	switch(ePowerState) {
	case BT_POWER_STATE_ASLEEP: {
		break;
	}

	case BT_POWER_STATE_AWAKE: {
		break;
	}

	default: {
		return (BT_ERROR) -1;
	}
	}

	return BT_ERR_NONE;
}

/**
 *	This implements the UART power management interface.
 *	It is called from the BT_GetPowerState() API!
 **/
static BT_ERROR uart_get_power_state(BT_HANDLE hUart, BT_POWER_STATE *pePowerState) {
	return BT_POWER_STATE_AWAKE;
}

/**
 *	Complete a full configuration of the UART.
 **/
static BT_ERROR uart_set_config(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 MR = ZYNQ_UART_MR_CHMODE_NORM;

	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		if(hUart->hRxFifo) {
			BT_CloseHandle(hUart->hRxFifo);
			hUart->hRxFifo = NULL;
		}
		if(hUart->hTxFifo) {
		 	BT_CloseHandle(hUart->hTxFifo);
			hUart->hTxFifo = NULL;
		}
	}
	else if(hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		if (hUart->RxBuffer) {
			BT_kFree((void *)hUart->RxBuffer);
			hUart->RxBuffer = NULL;
		}
		if (hUart->TxBuffer) {
			BT_kFree((void *)hUart->TxBuffer);
			hUart->TxBuffer = NULL;
		}
	}

	if (pConfig->ucStopBits == BT_UART_TWO_STOP_BITS) {
		MR |= ZYNQ_UART_MR_STOPMODE_2_BIT;
	}
	else {
		MR |= ZYNQ_UART_MR_STOPMODE_1_BIT;
	}

	if (pConfig->ucParity == BT_UART_PARITY_SPACE) {
		MR |= ZYNQ_UART_MR_PARITY_SPACE;
	}
	else if (pConfig->ucParity == BT_UART_PARITY_MARK) {
		MR |= ZYNQ_UART_MR_PARITY_MARK;
	}
	else if (pConfig->ucParity == BT_UART_PARITY_EVEN) {
		MR |= ZYNQ_UART_MR_PARITY_EVEN;
	}
	else if (pConfig->ucParity == BT_UART_PARITY_ODD) {
		MR |= ZYNQ_UART_MR_PARITY_ODD;
	}
	else {
		MR |= ZYNQ_UART_MR_PARITY_NONE;
	}

	if (pConfig->ucDataBits == BT_UART_6_DATABITS) {
		MR |= ZYNQ_UART_MR_CHARLEN_6_BIT;
	}
	else if (pConfig->ucDataBits == BT_UART_7_DATABITS) {
		MR |= ZYNQ_UART_MR_CHARLEN_7_BIT;
	}
	else {
		MR |= ZYNQ_UART_MR_CHARLEN_8_BIT;
	}

	hUart->pRegs->MR = MR;

	uart_set_baudrate(hUart, pConfig->ulBaudrate);

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hUart->eMode !=  BT_UART_MODE_POLLED) {
			hUart->eMode = BT_UART_MODE_POLLED;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED: {
		/*if(hUart->eMode != BT_UART_MODE_BUFFERED)*/ {
			if(!hUart->hRxFifo) {
				hUart->hRxFifo = BT_FifoCreate(pConfig->ulRxBufferSize, 1, 0, &Error);
			}
			if(!hUart->hTxFifo) {
				hUart->hTxFifo = BT_FifoCreate(pConfig->ulTxBufferSize, 1, 0, &Error);
			}
			hUart->eMode = BT_UART_MODE_BUFFERED;
			BT_EnableInterrupt(hUart->ulIRQ);
		}
		break;
	}

	case BT_UART_MODE_SIMPLE_BUFFERED: {
		/*if(hUart->eMode != BT_UART_MODE_SIMPLE_BUFFERED)*/ {
			hUart->uTxSize = pConfig->ulTxBufferSize;
			hUart->uRxSize = pConfig->ulRxBufferSize;
			hUart->uRxBegin = 0;
			hUart->uRxEnd = 0;
			hUart->uTxBegin = 0;
			hUart->uTxEnd = 0;
			if (!hUart->RxBuffer) {
				hUart->RxBuffer = BT_kMalloc(hUart->uRxSize);
			}
			if (!hUart->TxBuffer) {
				hUart->TxBuffer = BT_kMalloc(hUart->uTxSize);
			}
			hUart->eMode = BT_UART_MODE_SIMPLE_BUFFERED;
			BT_EnableInterrupt(hUart->ulIRQ);
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
static BT_ERROR uart_get_config(BT_HANDLE hUart, BT_UART_CONFIG *pConfig) {

	BT_ERROR Error 					= BT_ERR_NONE;
	volatile ZYNQ_UART_REGS *pRegs 	= hUart->pRegs;
	pConfig->eMode 					= hUart->eMode;

	BT_u32 InputClk;
	volatile ZYNQ_SLCR_REGS *pSLCR = bt_ioremap((void *)ZYNQ_SLCR, BT_SIZE_4K);
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
		Error = -1;
		goto err_out;
	}

	InputClk /= ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(pSLCR->UART_CLK_CTRL);

	pConfig->ulBaudrate 	= (InputClk / pRegs->BAUDDIV);		// Clk / Divisor == ~Baudrate

	if (hUart->eMode == BT_UART_MODE_POLLED) {
		pConfig->ulTxBufferSize = 64;
		pConfig->ulRxBufferSize = 64;
	}
	else if (hUart->eMode == BT_UART_MODE_BUFFERED) {
		pConfig->ulTxBufferSize = BT_FifoSize(hUart->hTxFifo);
		pConfig->ulRxBufferSize = BT_FifoSize(hUart->hRxFifo);
	}
	else if (hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		pConfig->ulTxBufferSize = hUart->uTxSize;
		pConfig->ulRxBufferSize = hUart->uRxSize;
	}

	if (pRegs->MR & ZYNQ_UART_MR_STOPMODE_2_BIT)
		pConfig->ucStopBits = BT_UART_TWO_STOP_BITS;
	else
		pConfig->ucStopBits = BT_UART_ONE_STOP_BIT;

	if ((pRegs->MR & ZYNQ_UART_MR_PARITY_SPACE) == ZYNQ_UART_MR_PARITY_SPACE)
		pConfig->ucParity = BT_UART_PARITY_SPACE;
	else if ((pRegs->MR & ZYNQ_UART_MR_PARITY_MARK) == ZYNQ_UART_MR_PARITY_MARK)
		pConfig->ucParity = BT_UART_PARITY_MARK;
	else if ((pRegs->MR & ZYNQ_UART_MR_PARITY_ODD) == ZYNQ_UART_MR_PARITY_ODD)
		pConfig->ucParity = BT_UART_PARITY_ODD;
	else if ((pRegs->MR & ZYNQ_UART_MR_PARITY_NONE) == ZYNQ_UART_MR_PARITY_NONE)
		pConfig->ucParity = BT_UART_PARITY_NONE;
	else
		pConfig->ucParity = BT_UART_PARITY_EVEN;

	if ((pRegs->MR & ZYNQ_UART_MR_CHARLEN_6_BIT) == ZYNQ_UART_MR_CHARLEN_6_BIT)
		pConfig->ucDataBits = BT_UART_6_DATABITS;
	else if ((pRegs->MR & ZYNQ_UART_MR_CHARLEN_7_BIT) == ZYNQ_UART_MR_CHARLEN_7_BIT)
		pConfig->ucDataBits = BT_UART_7_DATABITS;
	else
		pConfig->ucDataBits = BT_UART_8_DATABITS;

err_out:
	bt_iounmap(pSLCR);

	return Error;
}

static BT_ERROR uart_flush(BT_HANDLE hUart) {

	BT_ERROR Error;
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		while(!BT_FifoIsEmpty(hUart->hTxFifo, &Error)) {
			BT_ThreadYield();
		}
	}
	else if(hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		while(hUart->uTxBegin != hUart->uTxEnd) {
			BT_ThreadYield();
		}
	}

	while((pRegs->SR & ZYNQ_UART_SR_TXACTIVE) || !(pRegs->SR & ZYNQ_UART_SR_TXEMPTY)) {
		BT_ThreadYield();
	}

	return BT_ERR_NONE;
}

static BT_ERROR uart_enable(BT_HANDLE hUart) {

	BT_DisableInterrupt(hUart->ulIRQ);

	hUart->pRegs->CR 	= ZYNQ_UART_CR_TXDIS | ZYNQ_UART_CR_RXDIS;
	hUart->pRegs->CR 	|= ZYNQ_UART_CR_TXRES | ZYNQ_UART_CR_RXRES;

	BT_u32 status = hUart->pRegs->CR;
	hUart->pRegs->CR		= (status & ~(ZYNQ_UART_CR_TXDIS | ZYNQ_UART_CR_RXDIS)) | ZYNQ_UART_CR_TXEN | ZYNQ_UART_CR_RXEN | ZYNQ_UART_CR_STPBRK;
	hUart->pRegs->RXTRIG	= 14;
	hUart->pRegs->RXTOUT   	= 10;

	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		hUart->pRegs->IER 	= ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT;
		BT_EnableInterrupt(hUart->ulIRQ);
	}
	else if(hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		hUart->uRxBegin=0;
		hUart->uRxEnd=0;
		hUart->uTxBegin=0;
		hUart->uTxEnd=0;
		hUart->pRegs->IER 	= ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT;
		BT_EnableInterrupt(hUart->ulIRQ);
	}
	else {
		hUart->pRegs->IDR 	= ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT;
	}

	return BT_ERR_NONE;
}

static BT_ERROR uart_disable(BT_HANDLE hUart) {

	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	BT_DisableInterrupt(hUart->ulIRQ);

	hUart->pRegs->IDR = (ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT);
	pRegs->CR |= ZYNQ_UART_CR_RXDIS | ZYNQ_UART_CR_TXDIS;

	return BT_ERR_NONE;
}

static BT_ERROR uart_get_available(BT_HANDLE hUart, BT_u32 *pTransmit, BT_u32 *pReceive) {

	if (hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		if (hUart->uTxEnd >= hUart->uTxBegin)
			*pTransmit = hUart->uTxSize - hUart->uTxEnd + hUart->uTxBegin - 1;
		else
			*pTransmit = hUart->uTxBegin - hUart->uTxEnd - 1;

		if (hUart->uRxEnd >= hUart->uRxBegin)
			*pReceive = hUart->uRxEnd - hUart->uRxBegin;
		else
			*pReceive = hUart->uRxSize - hUart->uRxBegin + hUart->uRxEnd;
	}

	return BT_ERR_NONE;
}


static BT_s32 uart_read(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer) {

	BT_u8 *pucDest = (BT_u8 *) pBuffer;
	BT_s32 read = 0;
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	switch(hUart->eMode) {

	case BT_UART_MODE_POLLED:
	{
		while((pRegs->SR & ZYNQ_UART_SR_RXEMPTY)) {
			if(ulFlags & BT_FILE_NON_BLOCK) {
				return read;
			}
			BT_ThreadYield();
		}

		while(!(pRegs->SR & ZYNQ_UART_SR_RXEMPTY) && ulSize) {
			*pucDest++ = pRegs->FIFO & 0x000000FF;
			ulSize--;
			read++;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		read = BT_FifoRead(hUart->hRxFifo, ulSize, pBuffer, ulFlags);
		break;
	}

	case BT_UART_MODE_SIMPLE_BUFFERED:
	{
		while(hUart->uRxBegin == hUart->uRxEnd) {
			if(ulFlags & BT_FILE_NON_BLOCK) {
				return read;
			}
			BT_ThreadYield();
		}

		while((hUart->uRxBegin != hUart->uRxEnd) && ulSize) {
			if (++hUart->uRxBegin > (hUart->uRxSize-1)) {
				hUart->uRxBegin = 0;
			}

			*pucDest++ = hUart->RxBuffer[hUart->uRxBegin];
			ulSize--;
			read++;
		}
		break;
	}

	default:
		// ERR, invalid handle configuration.
		break;

	}

	return read;
}

static BT_s32 uart_write(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_u32 ulWritten = 0;
	BT_u8 *pucSource = (BT_u8 *) pBuffer;
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
			ulWritten += 1;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		const BT_u8 *pData = (BT_u8 *) pBuffer;
		do {
			while(!BT_FifoIsFull(hUart->hTxFifo, &Error)) {
				BT_FifoWrite(hUart->hTxFifo, 1, pData++, 0);
				ulWritten++;
				if(ulWritten >= ulSize) {
					break;
				}
			}

			// Trigger TX
			while(!BT_FifoIsEmpty(hUart->hTxFifo, &Error) && !(hUart->pRegs->SR & ZYNQ_UART_SR_TXFULL)) {
				BT_u8 ucData;
				BT_FifoRead(hUart->hTxFifo, 1, &ucData, ulFlags);
				hUart->pRegs->FIFO = ucData;
			}

			BT_u32 status = hUart->pRegs->CR;
			hUart->pRegs->CR = ((status & ~ZYNQ_UART_CR_TXDIS) | ZYNQ_UART_CR_TXEN);

			hUart->pRegs->IER = ZYNQ_UART_IXR_TXEMPTY;

			if(ulWritten >= ulSize) {
				break;
			}

			BT_FifoWrite(hUart->hTxFifo, 1, pData++, 0);
			ulWritten++;

		} while(ulWritten < ulSize);
		break;
	}

	case BT_UART_MODE_SIMPLE_BUFFERED:
	{
		do {
			if(((hUart->uTxEnd+1) == hUart->uTxBegin) || ((hUart->uTxEnd == (hUart->uTxSize-1)) && (hUart->uTxBegin == 0)))
			{
				if(ulFlags & BT_FILE_NON_BLOCK) {
					break;
				}

				while(((hUart->uTxEnd+1) == hUart->uTxBegin) || ((hUart->uTxEnd == (hUart->uTxSize-1)) && (hUart->uTxBegin == 0))) {
					BT_ThreadYield();
				}
			};

			hUart->TxBuffer[hUart->uTxEnd] = *pucSource++;
			if (++hUart->uTxEnd > (hUart->uTxSize-1)) {
				hUart->uTxEnd = 0;
			}

			if (hUart->pRegs->SR & ZYNQ_UART_SR_TXEMPTY)
			{
				BT_DisableInterrupt(hUart->ulIRQ);
				if ((hUart->pRegs->SR & ZYNQ_UART_SR_TXEMPTY) && (hUart->uTxEnd != hUart->uTxBegin)) {
					hUart->pRegs->FIFO = hUart->TxBuffer[hUart->uTxBegin];
					if (++hUart->uTxBegin > (hUart->uTxSize-1)) {
						hUart->uTxBegin = 0;
					}
				}
				BT_EnableInterrupt(hUart->ulIRQ);
			}

		} while(++ulWritten < ulSize);
		break;
	}

	default:
		break;
	}

	return ulWritten;
}

static BT_ERROR uart_tx_buffer_clear(BT_HANDLE hUart) {

	if(hUart->eMode == BT_UART_MODE_SIMPLE_BUFFERED) {
		if (hUart->uTxEnd != hUart->uTxBegin) {
			BT_DisableInterrupt(hUart->ulIRQ);
			hUart->uTxEnd = hUart->uTxBegin,
			BT_EnableInterrupt(hUart->ulIRQ);
		}
	}

	return BT_ERR_NONE;
}

static const BT_DEV_IF_UART oUartConfigInterface = {
	.pfnSetBaudrate = uart_set_baudrate,						///< UART setBaudrate implementation.
	.pfnSetConfig 	= uart_set_config,							///< UART set config imple.
	.pfnGetConfig 	= uart_get_config,
	.pfnEnable 		= uart_enable,								///< Enable/disable the device.
	.pfnDisable 	= uart_disable,
	.pfnGetAvailable = uart_get_available,
	.pfnTxBufferClear = uart_tx_buffer_clear,
};

static const BT_IF_POWER oPowerInterface = {
	.pfnSetPowerState = uart_set_power_state,					///< Pointers to the power state API implementations.
	.pfnGetPowerState = uart_get_power_state,					///< This gets the current power state.
};

static const BT_DEV_IFS oConfigInterface = {
	(BT_DEV_INTERFACE) &oUartConfigInterface,
};

static const BT_IF_DEVICE oDeviceInterface = {
	&oPowerInterface,											///< Device does not support powerstate functionality.
	BT_DEV_IF_T_UART,											///< Allow configuration through the UART api.
	.unConfigIfs = {
		.pUartIF = &oUartConfigInterface,
	},
};

static const BT_IF_FILE oFileInterface = {
	.pfnRead = uart_read,
	.pfnWrite = uart_write,
	.pfnFlush = uart_flush,
	.ulSupported = BT_FILE_NON_BLOCK,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.oIfs = {
		(BT_HANDLE_INTERFACE) &oDeviceInterface,
	},
	.pFileIF = &oFileInterface,
	.eType = BT_HANDLE_T_DEVICE,								///< Handle Type!
	.pfnCleanup = uart_cleanup,									///< Handle's cleanup routine.
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

	hUart->pRegs = (ZYNQ_UART_REGS *) bt_ioremap((void *)pResource->ulStart, BT_SIZE_4K);

	hUart->pRegs->BAUDGEN	= 0x0000028B;
	hUart->pRegs->MODEMCR 	= 0;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}
	hUart->ulIRQ = pResource->ulStart;

	Error = BT_RegisterInterrupt(hUart->ulIRQ, uart_irq_handler, hUart);
	if(Error) {
		goto err_free_out;
	}

	uart_disable(hUart);

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
	.name 		= "xlnx,xuartps",
	.eType		= BT_DRIVER_INTEGRATED | BT_DRIVER_DEVFS_PROBE,	///< Integrated device that should only be probed on devfs open.
	.pfnProbe	= uart_probe,
};
