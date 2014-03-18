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
};

static const BT_IF_HANDLE oHandleInterface;

static BT_ERROR uart_irq_handler(BT_u32 ulIRQ, void *pParam) {

	BT_ERROR Error = BT_ERR_NONE;
	BT_HANDLE hUart = (BT_HANDLE) pParam;

	BT_u32 isr = hUart->pRegs->ISR;
	BT_u32 data;

	if((isr & ZYNQ_UART_IXR_TOUT) || (isr & ZYNQ_UART_IXR_RXTRIG)) {
		// RX timeout
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

	hUart->pRegs->ISR = isr;

	return Error;
}

static BT_ERROR uart_disable(BT_HANDLE hUart);
static BT_ERROR uart_enable(BT_HANDLE hUart);

static BT_ERROR uart_cleanup(BT_HANDLE hUart) {

	uart_disable(hUart);
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_IRQ, 0);

	BT_DisableInterrupt(pResource->ulStart);
	BT_UnregisterInterrupt(pResource->ulStart, uart_irq_handler, hUart);

	// Free any buffers if used.
	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		if(hUart->hRxFifo) {
			BT_CloseHandle(hUart->hRxFifo);
		}
		if(hUart->hTxFifo) {
		 	BT_CloseHandle(hUart->hTxFifo);
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

	uart_disable(hUart);

	pRegs->BAUDGEN = oDiv.diva_val;							// Apply the calculated divider values.
	pRegs->BAUDDIV = oDiv.divb_val;

	uart_enable(hUart);
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

	uart_set_baudrate(hUart, pConfig->ulBaudrate);

	switch(pConfig->eMode) {
	case BT_UART_MODE_POLLED: {
		if(hUart->eMode !=  BT_UART_MODE_POLLED) {
			if(hUart->hRxFifo) {
				BT_CloseHandle(hUart->hRxFifo);
				hUart->hRxFifo = NULL;
			}
			if(hUart->hTxFifo) {
				BT_CloseHandle(hUart->hTxFifo);
				hUart->hTxFifo = NULL;
			}

			const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_IRQ, 0);
			BT_DisableInterrupt(pResource->ulStart);

			hUart->eMode = BT_UART_MODE_POLLED;
		}
		break;
	}

	case BT_UART_MODE_BUFFERED:
	{
		if(hUart->eMode != BT_UART_MODE_BUFFERED) {
			if(!hUart->hRxFifo && !hUart->hTxFifo) {
				hUart->eMode = BT_UART_MODE_BUFFERED;
				hUart->hRxFifo = BT_FifoCreate(pConfig->ulRxBufferSize, 1, &Error);
				hUart->hTxFifo = BT_FifoCreate(pConfig->ulTxBufferSize, 1, &Error);

				const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_IRQ, 0);
				BT_EnableInterrupt(pResource->ulStart);
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
	pConfig->ulTxBufferSize = BT_FifoSize(hUart->hTxFifo);
	pConfig->ulRxBufferSize = BT_FifoSize(hUart->hRxFifo);
	pConfig->ucDataBits 	= 8;

err_out:
	bt_iounmap(pSLCR);

	return Error;
}

static BT_ERROR uart_flush(BT_HANDLE hUart) {
	BT_ERROR Error;
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		// Empty the TX fifo's.
		while(!BT_FifoIsEmpty(hUart->hTxFifo, &Error)) {
			BT_ThreadYield();
		}
	}

	while((pRegs->SR & ZYNQ_UART_SR_TXACTIVE) || !(pRegs->SR & ZYNQ_UART_SR_TXEMPTY)) {
		BT_ThreadYield();
	}

	return BT_ERR_NONE;
}

static BT_ERROR uart_enable(BT_HANDLE hUart) {

	//uart_flush(hUart);
	/*while(!(hUart->pRegs->SR & ZYNQ_UART_SR_RXEMPTY)) {
		volatile BT_u8 data = hUart->pRegs->FIFO;
	}*/

	hUart->pRegs->CR 	= ZYNQ_UART_CR_TXDIS | ZYNQ_UART_CR_RXDIS;
	hUart->pRegs->CR 	|= ZYNQ_UART_CR_TXRES | ZYNQ_UART_CR_RXRES;

	BT_u32 status = hUart->pRegs->CR;

	hUart->pRegs->CR		= (status & ~(ZYNQ_UART_CR_TXDIS | ZYNQ_UART_CR_RXDIS)) | ZYNQ_UART_CR_TXEN | ZYNQ_UART_CR_RXEN | ZYNQ_UART_CR_STPBRK;
	hUart->pRegs->MR 		= ZYNQ_UART_MR_CHMODE_NORM | ZYNQ_UART_MR_STOPMODE_1_BIT | ZYNQ_UART_MR_PARITY_NONE | ZYNQ_UART_MR_CHARLEN_8_BIT;
	hUart->pRegs->RXTRIG	= 14;
	hUart->pRegs->RXTOUT   	= 10;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_IRQ, 0);

	if(hUart->eMode == BT_UART_MODE_BUFFERED) {
		hUart->pRegs->IER 	= ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT;
		BT_EnableInterrupt(pResource->ulStart);
	} else {
		hUart->pRegs->IDR 	= ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT;
		BT_DisableInterrupt(pResource->ulStart);
	}



	return BT_ERR_NONE;
}

static BT_ERROR uart_disable(BT_HANDLE hUart) {
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	hUart->pRegs->IDR = (ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT);
	pRegs->CR |= ZYNQ_UART_CR_RXDIS | ZYNQ_UART_CR_TXDIS;

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hUart->pDevice, BT_RESOURCE_IRQ, 0);
	BT_DisableInterrupt(pResource->ulStart);

	return BT_ERR_NONE;
}

static BT_s32 uart_read(BT_HANDLE hUart, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer) {

	BT_u8 *pucDest = (BT_u8 *) pBuffer;
	BT_s32 read = 0;
	volatile ZYNQ_UART_REGS *pRegs = hUart->pRegs;

	switch(hUart->eMode) {
	case BT_UART_MODE_POLLED:
	{
		while(ulSize) {
			while((pRegs->SR & ZYNQ_UART_SR_RXEMPTY)) {
				if(ulFlags & BT_FILE_NON_BLOCK) {
					return read;
				}
				BT_ThreadYield();
			}

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

	default:
		break;
	}

	return ulWritten;
}

static const BT_DEV_IF_UART oUartConfigInterface = {
	.pfnSetBaudrate = uart_set_baudrate,						///< UART setBaudrate implementation.
	.pfnSetConfig 	= uart_set_config,							///< UART set config imple.
	.pfnGetConfig 	= uart_get_config,
	.pfnEnable 		= uart_enable,								///< Enable/disable the device.
	.pfnDisable 	= uart_disable,
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

	Error = BT_RegisterInterrupt(pResource->ulStart, uart_irq_handler, hUart);
	if(Error) {
		goto err_free_out;
	}

	uart_disable(hUart);

	Error = BT_EnableInterrupt(pResource->ulStart);
	if(Error) {
		goto err_free_irq;
	}

	return hUart;

err_free_irq:
	BT_UnregisterInterrupt(pResource->ulStart, uart_irq_handler, hUart);

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
