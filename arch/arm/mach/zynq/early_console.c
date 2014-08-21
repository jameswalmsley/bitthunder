#include <bitthunder.h>
#include "uart.h"
#include "slcr.h"
#include "uart.h"

static void early_init() {
	volatile ZYNQ_UART_REGS *regs = (ZYNQ_UART_REGS *) 0xE0001000;

	BT_u32	InputClk;
	BT_u32	BaudRate = 115200;

	volatile ZYNQ_SLCR_REGS *pSLCR = (ZYNQ_SLCR_REGS *) ZYNQ_SLCR;

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
		return;
	}

	InputClk /= ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(pSLCR->UART_CLK_CTRL);

	BT_DIVIDER_PARAMS oDiv;
	oDiv.diva_max = 65536;
	oDiv.diva_min = 1;
	oDiv.divb_min = 4;
	oDiv.divb_max = 255;

	BT_CalculateClockDivider(InputClk, BaudRate, &oDiv);	// Calculate 2 stage divider.

	regs->IDR = (ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT);
	regs->CR |= ZYNQ_UART_CR_RXDIS | ZYNQ_UART_CR_TXDIS;

	regs->CR 	= ZYNQ_UART_CR_TXDIS | ZYNQ_UART_CR_RXDIS;
	regs->CR 	|= ZYNQ_UART_CR_TXRES | ZYNQ_UART_CR_RXRES;
	//uart_disable(hUart);

	regs->MODEMCR = 0;
	regs->BAUDGEN = oDiv.diva_val;							// Apply the calculated divider values.
	regs->BAUDDIV = oDiv.divb_val;

	BT_u32 status = regs->CR;
	regs->CR		= (status & ~(ZYNQ_UART_CR_TXDIS | ZYNQ_UART_CR_RXDIS)) | ZYNQ_UART_CR_TXEN | ZYNQ_UART_CR_RXEN | ZYNQ_UART_CR_STPBRK;
	regs->RXTRIG	= 14;
	regs->RXTOUT   	= 10;

	regs->IDR 		= ZYNQ_UART_IXR_TXEMPTY | ZYNQ_UART_IXR_RXTRIG | ZYNQ_UART_IXR_TOUT;
}

static void early_write(const BT_u8 *data, BT_u32 ulLength) {

	volatile ZYNQ_UART_REGS *regs = (ZYNQ_UART_REGS *) 0xE0001000;

	while(ulLength && *data) {
			while(regs->SR & ZYNQ_UART_SR_TXFULL) {
				;
			}
			regs->FIFO = *data++;
			ulLength--;
	}

	while((regs->SR & ZYNQ_UART_SR_TXACTIVE) || !(regs->SR & ZYNQ_UART_SR_TXEMPTY)) {
		;
	}
}

static void early_cleanup() {
	ZYNQ_UART_REGS *regs = (ZYNQ_UART_REGS *) 0xE0001000;
}

BT_DEV_IF_EARLY_CONSOLE oZynq_early_console_device = {
	.pfnInit 	= early_init,
	.pfnWrite 	= early_write,
	.pfnCleanup = early_cleanup,
};
