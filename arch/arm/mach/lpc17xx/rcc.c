#include <bitthunder.h>
#include "rcc.h"


BT_u32 BT_LPC17xx_GetPeripheralClock(BT_u32 ulPeripheral) {

	volatile LPC17xx_RCC_REGS *pRegs = LPC17xx_RCC;

	BT_u32 ulShift = 2*(ulPeripheral % 16);
	BT_u32 ulMask = 0x03 << ulShift;


	BT_u32 ulDivider = 1;
	if (ulPeripheral < 16)
	{
		ulDivider = (pRegs->PCLKSEL0 & ulMask) >> ulShift;
	}
	else if (ulPeripheral < 32)
	{
		ulDivider = (pRegs->PCLKSEL1 & ulMask) >> ulShift;
	}
	switch (ulDivider){
		case 0: ulDivider = 4; break;
		case 1: ulDivider = 1; break;
		case 2: ulDivider = 2; break;
		case 3: ulDivider = 8; break;
	}
	return BT_LPC17xx_GetSystemFrequency() / ulDivider;
}


BT_u32 BT_LPC17xx_GetMainPLLFrequency(void)
{
	volatile LPC17xx_RCC_REGS *pRegs = LPC17xx_RCC;
	BT_u32 PLL_In_Freq;

	switch (pRegs->CLKSRCSEL & 0x03)
	{
		case LPC17xx_RCC_CLKSRCSEL_IRC_OSC  : PLL_In_Freq = BT_CONFIG_MACH_LPC17xx_SYSCLOCK_IRC_FREQ; break;
		case LPC17xx_RCC_CLKSRCSEL_MAIN_OSC : PLL_In_Freq = BT_CONFIG_MACH_LPC17xx_SYSCLOCK_FREQ; break;
		case LPC17xx_RCC_CLKSRCSEL_RTC_OSC  : PLL_In_Freq = 0; break;
		case LPC17xx_RCC_CLKSRCSEL_RESERVED : PLL_In_Freq = 0; break;
	}

	return PLL_In_Freq * (LPC11xx_RCC_MAINPLLCTRL_MSEL(pRegs->PLL0CFG)) * 2 / (LPC11xx_RCC_MAINPLLCTRL_NSEL(pRegs->PLL0CFG));
}

BT_u32 BT_LPC17xx_GetSystemFrequency(void) {

	volatile LPC17xx_RCC_REGS *pRegs = LPC17xx_RCC;

	return BT_LPC17xx_GetMainPLLFrequency() / (pRegs->CCLKCFG + 1);
}

void BT_LPC17xx_SetSystemFrequency(BT_u32 SysClkCtrl,
								   BT_u32 MainPLLClkSrc,
								   BT_u32 MainPLLClkCtrl,
								   BT_u32 SystemClockDivider,
								   BT_u32 USBClkSrc,
								   BT_u32 USBPLLClkCtrl,
								   BT_u32 USBClkDivider)
{
	volatile LPC17xx_RCC_REGS *pRegs = LPC17xx_RCC;

	pRegs->SCS = SysClkCtrl;

	if (pRegs->SCS & LPC17xx_RCC_SCS_OSCEN ) {             /* If Main Oscillator is enabled      */
		while (!(pRegs->SCS & LPC17xx_RCC_SCS_OSCSTAT));/* Wait for Oscillator to be ready    */
	}

	pRegs->CCLKCFG = SystemClockDivider-1;

	pRegs->CLKSRCSEL = MainPLLClkSrc;    /* Select Clock Source for PLL0       */

	pRegs->PLL0CFG   = MainPLLClkCtrl;      /* configure PLL0                     */
	pRegs->PLL0FEED  = LPC17xx_RCC_MAINPLLFEED_A;
	pRegs->PLL0FEED  = LPC17xx_RCC_MAINPLLFEED_B;

	pRegs->PLL0CON   = LPC17xx_RCC_MAINPLL_ENABLE;             /* PLL0 Enable                        */
	pRegs->PLL0FEED  = LPC17xx_RCC_MAINPLLFEED_A;
	pRegs->PLL0FEED  = LPC17xx_RCC_MAINPLLFEED_B;
	while (!(pRegs->PLL0STAT & LPC17xx_RCC_MAINPLLSTAT_LOCKED));/* Wait for PLOCK0                    */

	pRegs->PLL0CON   = LPC17xx_RCC_MAINPLL_ENABLE | LPC17xx_RCC_MAINPLL_CONNECT;             /* PLL0 Enable & Connect              */
	pRegs->PLL0FEED  = LPC17xx_RCC_MAINPLLFEED_A;
	pRegs->PLL0FEED  = LPC17xx_RCC_MAINPLLFEED_B;
	while (!(pRegs->PLL0STAT & (LPC17xx_RCC_MAINPLLSTAT_ENABLE | LPC17xx_RCC_MAINPLLSTAT_CONNECT)));/* Wait for PLLC0_STAT & PLLE0_STAT */


	BT_u32 ulCPUClock = BT_LPC17xx_GetSystemFrequency();
	BT_u32 ulFlashCfg = 0;

	if (ulCPUClock <= 20000000) ulFlashCfg = 0;
	if (ulCPUClock <= 40000000) ulFlashCfg = 1;
	if (ulCPUClock <= 60000000) ulFlashCfg = 2;
	if (ulCPUClock <= 80000000) ulFlashCfg = 3;
	if (ulCPUClock <= 120000000) ulFlashCfg = 4;

	pRegs->FLASHCFG  = (pRegs->FLASHCFG & ~LPC17xx_RCC_FLASHCFG_MASK) | (ulFlashCfg << 12);
}
