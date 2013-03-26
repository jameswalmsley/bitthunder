#include <bitthunder.h>
#include "rcc.h"

BT_u32 BT_LPC11xx_GetWDTFrequency(void)
{
	volatile LPC11xx_RCC_REGS *pRegs = LPC11xx_RCC;
	BT_u32 wdt_osc = 0;

	/* Determine clock frequency according to clock register values             */
	switch ((pRegs->WDTOSCCTRL >> 5) & 0x0F)
	{
		case 0:  wdt_osc =       0; break;
		case 1:  wdt_osc =  600000; break;
		case 2:  wdt_osc = 1050000; break;
		case 3:  wdt_osc = 1400000; break;
		case 4:  wdt_osc = 1750000; break;
		case 5:  wdt_osc = 2100000; break;
		case 6:  wdt_osc = 2400000; break;
		case 7:  wdt_osc = 2700000; break;
		case 8:  wdt_osc = 3000000; break;
		case 9:  wdt_osc = 3250000; break;
		case 10: wdt_osc = 3500000; break;
		case 11: wdt_osc = 3750000; break;
		case 12: wdt_osc = 4000000; break;
		case 13: wdt_osc = 4200000; break;
		case 14: wdt_osc = 4400000; break;
		case 15: wdt_osc = 4600000; break;
		}
	wdt_osc /= (((pRegs->WDTOSCCTRL & 0x1F) << 1) + 2);
	return wdt_osc;
}

BT_u32 BT_LPC11xx_GetMainFrequency(void)
{
	volatile LPC11xx_RCC_REGS *pRegs = LPC11xx_RCC;
	BT_u32 PLL_In_Freq;

	switch (pRegs->SYSPLLCLKSEL & 0x03)
	{
		case LPC11xx_RCC_SYSPLLCLKSEL_IRC_OSC  : PLL_In_Freq = BT_CONFIG_MACH_LPC11xx_SYSCLOCK_IRC_FREQ; break;
		case LPC11xx_RCC_SYSPLLCLKSEL_XTAL     : PLL_In_Freq = BT_CONFIG_MACH_LPC11xx_SYSCLOCK_FREQ; break;
		case LPC11xx_RCC_SYSPLLCLKSEL_CLKIN    : PLL_In_Freq = BT_LPC11xx_GetWDTFrequency(); break;
		case LPC11xx_RCC_SYSPLLCLKSEL_RESERVED : PLL_In_Freq = 0; break;
	}

	switch (pRegs->MAINCLKSEL & 0x00000003)
	{
		case LPC11xx_RCC_MAINCLKSEL_IRC_OSC: return BT_CONFIG_MACH_LPC11xx_SYSCLOCK_IRC_FREQ;
		case LPC11xx_RCC_MAINCLKSEL_PLL_IN:
		{
			return PLL_In_Freq;
		}
		case LPC11xx_RCC_MAINCLKSEL_WDT_OSC:
		{
			return BT_LPC11xx_GetWDTFrequency();
		}
		case LPC11xx_RCC_MAINCLKSEL_PLL_OUT:
		{
			return PLL_In_Freq * (LPC11xx_RCC_SYSPLLCTRL_MSEL(pRegs->SYSPLLCTRL));
		}

	}
	return 0;
}

BT_u32 BT_LPC11xx_GetSystemFrequency(void) {

	volatile LPC11xx_RCC_REGS *pRegs = LPC11xx_RCC;

	return BT_LPC11xx_GetMainFrequency() / pRegs->SYSAHBCLKDIV;
}

void BT_LPC11xx_SetSystemFrequency(BT_u32 MainClkSrc,
									 BT_u32 SysClkCtrl,
									 BT_u32 PLLClkSrc,
									 BT_u32 PLLClkCtrl,
									 BT_u32 WDTClkSrc,
									 BT_u32 SystemClockDivider)
{
	volatile LPC11xx_RCC_REGS *pRegs = LPC11xx_RCC;
	BT_u32 i;

	pRegs->PDRUNCFG     &= ~LPC11xx_RCC_PDRUNCFG_SYSOSC_PD;          /* Power-up System Osc      */
	pRegs->SYSOSCCTRL    = SysClkCtrl;
	for (i = 0; i < 200; i++) { __asm volatile ("nop"); };

	if (PLLClkSrc != LPC11xx_RCC_SYSPLLCLKSEL_NOT_USED)
	{
		pRegs->SYSPLLCLKSEL  = PLLClkSrc;   /* Select PLL Input         */
		pRegs->SYSPLLCLKUEN  = LPC11xx_RCC_SYSPLLCLKUEN_UPDATE;               /* Update Clock Source      */
		pRegs->SYSPLLCLKUEN  = 0x00;             							  /* Toggle Update Register   */
		pRegs->SYSPLLCLKUEN  = LPC11xx_RCC_SYSPLLCLKUEN_UPDATE;
		while (!(pRegs->SYSPLLCLKUEN & LPC11xx_RCC_SYSPLLCLKUEN_UPDATE));     /* Wait Until Updated       */

		pRegs->SYSPLLCTRL    = PLLClkCtrl;
		pRegs->PDRUNCFG     &= ~LPC11xx_RCC_PDRUNCFG_SYSPLL_PD;          /* Power-up SYSPLL          */
		while (!(pRegs->SYSPLLSTAT & LPC11xx_RCC_SYSPLLSTAT_LOCKED));	      /* Wait Until PLL Locked    */
	}

	if (WDTClkSrc != LPC11xx_RCC_WDTCLKSEL_NOT_USED)
	{
		/* Watchdog Oscillator Setup*/
		pRegs->WDTOSCCTRL    = WDTClkSrc;
		pRegs->PDRUNCFG     &= ~LPC11xx_RCC_PDRUNCFG_WDTOSC_PD;          /* Power-up WDT Clock       */

		pRegs->WDTCLKUEN  = LPC11xx_RCC_WDTCLKUEN_UPDATE;               /* Update Clock Source      */
		pRegs->WDTCLKUEN  = 0x00;             						    /* Toggle Update Register   */
		pRegs->WDTCLKUEN  = LPC11xx_RCC_WDTCLKUEN_UPDATE;
		while (!(pRegs->WDTCLKUEN & LPC11xx_RCC_WDTCLKUEN_UPDATE));     /* Wait Until Updated       */
	}

	pRegs->MAINCLKSEL    = MainClkSrc;     /* Select PLL Clock Output  */
	pRegs->MAINCLKUEN    = LPC11xx_RCC_MAINCLKUEN_UPDATE;               /* Update MCLK Clock Source */
	pRegs->MAINCLKUEN    = 0x00;            						    /* Toggle Update Register   */
	pRegs->MAINCLKUEN    = LPC11xx_RCC_MAINCLKUEN_UPDATE;
	while (!(pRegs->MAINCLKUEN & LPC11xx_RCC_MAINCLKUEN_UPDATE));       /* Wait Until Updated       */

	pRegs->SYSAHBCLKDIV  = SystemClockDivider;
}
