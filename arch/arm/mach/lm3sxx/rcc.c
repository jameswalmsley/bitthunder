#include <bitthunder.h>
#include "rcc.h"

static void __attribute__((naked)) SysCtlDelay(unsigned long ulCount)
{
    __asm("    subs    r0, #1\n"
          "    bne     SysCtlDelay\n"
          "    bx      lr");
}


BT_u32 BT_LM3Sxx_EnablePeripheral(BT_u32 ulPeripheral) {

	volatile LM3Sxx_RCC_REGS *pRegs = LM3Sxx_RCC;

	BT_u32 ulBank = ulPeripheral / 32;
	BT_u32 ulBit  = ulPeripheral % 32;

	pRegs->RCGC[ulBank] |= 0x01 << ulBit;
}


BT_u32 BT_LM3Sxx_GetMainFrequency(void)
{
	volatile LM3Sxx_RCC_REGS *pRegs = LM3Sxx_RCC;

	return 80000000;
}

BT_u32 BT_LM3Sxx_GetSystemFrequency(void) {

	volatile LM3Sxx_RCC_REGS *pRegs = LM3Sxx_RCC;

	return BT_LM3Sxx_GetMainFrequency();
}

void BT_LM3Sxx_SetSystemFrequency(BT_u32 ulConfig) {
	volatile LM3Sxx_RCC_REGS *pRCC = LM3Sxx_RCC;

	BT_u32 i;

	for (i = 0; i < 1000000; i++);

    // Bypass the PLL and system clock dividers for now.
/*	pRCC->RCC |= LM3Sxx_RCC_RCC_BYPASS;
	pRCC->RCC &= ~(LM3Sxx_RCC_RCC_USESYSDIV);
	pRCC->RCC2 |= LM3Sxx_RCC_RCC2_BYPASS2;

    // See if either oscillator needs to be enabled.
    if(((pRCC->RCC & LM3Sxx_RCC_RCC_IOSCDIS) && !(ulConfig & LM3Sxx_RCC_RCC_IOSCDIS)) ||
       ((pRCC->RCC & LM3Sxx_RCC_RCC_MOSCDIS) && !(ulConfig & LM3Sxx_RCC_RCC_MOSCDIS)))
    {
        //
        // Make sure that the required oscillators are enabled.  For now, the
        // previously enabled oscillators must be enabled along with the newly
        // requested oscillators.
        //
    	pRCC->RCC &= (~(LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS) |
                  (ulConfig & (LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS)));

        //
        // Wait for a bit, giving the oscillator time to stabilize.  The number
        // of iterations is adjusted based on the current clock source; a
        // smaller number of iterations is required for slower clock rates.
        //
        if(((pRCC->RCC2 & LM3Sxx_RCC_RCC2_USERCC2) &&
            (((pRCC->RCC2 & LM3Sxx_RCC_RCC2_OSCSRC2_M) == LM3Sxx_RCC_RCC2_OSCSRC2_30) ||
             ((pRCC->RCC2 & LM3Sxx_RCC_RCC2_OSCSRC2_M) == LM3Sxx_RCC_RCC2_OSCSRC2_32))) ||
           (!(pRCC->RCC2 & LM3Sxx_RCC_RCC2_USERCC2) &&
            ((pRCC->RCC & LM3Sxx_RCC_RCC_OSCSRC_M) == LM3Sxx_RCC_RCC_OSCSRC_30)))
        {
            //
            // Delay for 4096 iterations.
            //
            SysCtlDelay(4096);
        }
        else
        {
            //
            // Delay for 524,288 iterations.
            //
            SysCtlDelay(524288);
        }
    }

    //
    // Set the new crystal value, oscillator source, and PLL configuration.
    // Since the OSCSRC2 field in RCC2 overlaps the XTAL field in RCC, the
    // OSCSRC field has a special encoding within ulConfig to avoid the
    // overlap.
    //
    pRCC->RCC &= ~(LM3Sxx_RCC_RCC_XTAL_M | LM3Sxx_RCC_RCC_OSCSRC_M | LM3Sxx_RCC_RCC_PWRDN);
    pRCC->RCC |= ulConfig & (LM3Sxx_RCC_RCC_XTAL_M | LM3Sxx_RCC_RCC_OSCSRC_M | LM3Sxx_RCC_RCC_PWRDN);
    pRCC->RCC2 &= ~(LM3Sxx_RCC_RCC2_USERCC2 | LM3Sxx_RCC_RCC2_OSCSRC2_M | LM3Sxx_RCC_RCC2_PWRDN2);
    pRCC->RCC2 |= ulConfig & (LM3Sxx_RCC_RCC2_USERCC2 | LM3Sxx_RCC_RCC_OSCSRC_M | LM3Sxx_RCC_RCC2_PWRDN2);
    pRCC->RCC2 |= (ulConfig & 0x00000008) << 3;

    //
    // Clear the PLL lock interrupt.
    //
    pRCC->MISC = LM3Sxx_RCC_MISC_PLLLMIS;

    // Wait for a bit so that new crystal value and oscillator source can take
    // effect.
    SysCtlDelay(16);

    // Set the requested system divider and disable the appropriate
    // oscillators.  This will not get written immediately.
    pRCC->RCC &= ~(LM3Sxx_RCC_RCC_SYSDIV_M | LM3Sxx_RCC_RCC_USESYSDIV | LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS);
    pRCC->RCC |= ulConfig & (LM3Sxx_RCC_RCC_SYSDIV_M | LM3Sxx_RCC_RCC_USESYSDIV | LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS);
    pRCC->RCC2 &= ~(LM3Sxx_RCC_RCC2_SYSDIV2_M);
    pRCC->RCC2 |= ulConfig & LM3Sxx_RCC_RCC2_SYSDIV2_M;
    if(ulConfig & LM3Sxx_RCC_RCC2_DIV400)
    {
    	pRCC->RCC |= LM3Sxx_RCC_RCC_USESYSDIV;
    	pRCC->RCC2 &= ~(LM3Sxx_RCC_RCC_USESYSDIV);
    	pRCC->RCC2 |= ulConfig & (LM3Sxx_RCC_RCC2_DIV400 | LM3Sxx_RCC_RCC2_SYSDIV2LSB);
    }
    else
    {
    	pRCC->RCC2 &= ~(LM3Sxx_RCC_RCC2_DIV400);
    }

    //
    // See if the PLL output is being used to clock the system.
    //
    if(!(ulConfig & LM3Sxx_RCC_RCC_BYPASS))
    {
        // Wait until the PLL has locked.
    	BT_u32 ulDelay;

        for(ulDelay = 32768; ulDelay > 0; ulDelay--)
        {
            if(pRCC->RIS & LM3Sxx_RCC_RIS_PLLLRIS)
            {
                break;
            }
        }

        // Enable use of the PLL.
        pRCC->RCC &= ~(LM3Sxx_RCC_RCC_BYPASS);
        pRCC->RCC2 &= ~(LM3Sxx_RCC_RCC2_BYPASS2);
    }

    // Delay for a little bit so that the system divider takes effect.
    SysCtlDelay(16);*/

    //
    // Get the current value of the RCC and RCC2 registers.  If using a
    // Sandstorm-class device, the RCC2 register will read back as zero and the
    // writes to it from within this function will be ignored.
    //
    BT_u32 ulRCC = pRCC->RCC;
    BT_u32 ulRCC2 = pRCC->RCC2;

    //
    // Bypass the PLL and system clock dividers for now.
    //
    ulRCC |= LM3Sxx_RCC_RCC_BYPASS;
    ulRCC &= ~(LM3Sxx_RCC_RCC_USESYSDIV);
    ulRCC2 |= LM3Sxx_RCC_RCC2_BYPASS2;

    //
    // Write the new RCC value.
    //
    pRCC->RCC = ulRCC;
    pRCC->RCC2 = ulRCC2;

    //
    // See if either oscillator needs to be enabled.
    //
    if(((ulRCC & LM3Sxx_RCC_RCC_IOSCDIS) && !(ulConfig & LM3Sxx_RCC_RCC_IOSCDIS)) ||
       ((ulRCC & LM3Sxx_RCC_RCC_MOSCDIS) && !(ulConfig & LM3Sxx_RCC_RCC_MOSCDIS)))
    {
        //
        // Make sure that the required oscillators are enabled.  For now, the
        // previously enabled oscillators must be enabled along with the newly
        // requested oscillators.
        //
        ulRCC &= (~(LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS) |
                  (ulConfig & (LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS)));

        //
        // Write the new RCC value.
        //
        pRCC->RCC = ulRCC;

        //
        // Wait for a bit, giving the oscillator time to stabilize.  The number
        // of iterations is adjusted based on the current clock source; a
        // smaller number of iterations is required for slower clock rates.
        //
        if(((ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2) &&
            (((ulRCC2 & LM3Sxx_RCC_RCC2_OSCSRC2_M) == LM3Sxx_RCC_RCC2_OSCSRC2_30) ||
             ((ulRCC2 & LM3Sxx_RCC_RCC2_OSCSRC2_M) == LM3Sxx_RCC_RCC2_OSCSRC2_32))) ||
           (!(ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2) &&
            ((ulRCC & LM3Sxx_RCC_RCC_OSCSRC_M) == LM3Sxx_RCC_RCC_OSCSRC_30)))
        {
            //
            // Delay for 4096 iterations.
            //
            SysCtlDelay(4096);
        }
        else
        {
            //
            // Delay for 524,288 iterations.
            //
            SysCtlDelay(524288);
        }
    }

    //
    // Set the new crystal value, oscillator source, and PLL configuration.
    // Since the OSCSRC2 field in RCC2 overlaps the XTAL field in RCC, the
    // OSCSRC field has a special encoding within ulConfig to avoid the
    // overlap.
    //
    ulRCC &= ~(LM3Sxx_RCC_RCC_XTAL_M | LM3Sxx_RCC_RCC_OSCSRC_M |
               LM3Sxx_RCC_RCC_PWRDN);
    ulRCC |= (ulConfig & (LM3Sxx_RCC_RCC_XTAL_M | LM3Sxx_RCC_RCC_OSCSRC_M |
                         LM3Sxx_RCC_RCC_PWRDN));
    ulRCC2 &= ~(LM3Sxx_RCC_RCC2_USERCC2 | LM3Sxx_RCC_RCC2_OSCSRC2_M |
                LM3Sxx_RCC_RCC2_PWRDN2);
    ulRCC2 |= (ulConfig & (LM3Sxx_RCC_RCC2_USERCC2 | LM3Sxx_RCC_RCC_OSCSRC_M |
                          LM3Sxx_RCC_RCC2_PWRDN2));
    ulRCC2 |= (ulConfig & 0x00000008) << 3;

    //
    // Clear the PLL lock interrupt.
    //
    pRCC->MISC = LM3Sxx_RCC_MISC_PLLLMIS;

    //
    // Write the new RCC value.
    //
    if(ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2)
    {
        pRCC->RCC2 = ulRCC2;
        pRCC->RCC = ulRCC;
    }
    else
    {
        pRCC->RCC = ulRCC;
        pRCC->RCC2 = ulRCC2;
    }

    //
    // Wait for a bit so that new crystal value and oscillator source can take
    // effect.
    //
    SysCtlDelay(16);

    //
    // Set the requested system divider and disable the appropriate
    // oscillators.  This will not get written immediately.
    //
    ulRCC &= ~(LM3Sxx_RCC_RCC_SYSDIV_M | LM3Sxx_RCC_RCC_USESYSDIV |
               LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS);
    ulRCC |= ulConfig & (LM3Sxx_RCC_RCC_SYSDIV_M | LM3Sxx_RCC_RCC_USESYSDIV |
                         LM3Sxx_RCC_RCC_IOSCDIS | LM3Sxx_RCC_RCC_MOSCDIS);
    ulRCC2 &= ~(LM3Sxx_RCC_RCC2_SYSDIV2_M);
    ulRCC2 |= ulConfig & LM3Sxx_RCC_RCC2_SYSDIV2_M;
    if(ulConfig & LM3Sxx_RCC_RCC2_DIV400)
    {
        ulRCC |= LM3Sxx_RCC_RCC_USESYSDIV;
        ulRCC2 &= ~(LM3Sxx_RCC_RCC_USESYSDIV);
        ulRCC2 |= ulConfig & (LM3Sxx_RCC_RCC2_DIV400 | LM3Sxx_RCC_RCC2_SYSDIV2LSB);
    }
    else
    {
        ulRCC2 &= ~(LM3Sxx_RCC_RCC2_DIV400);
    }

    //
    // See if the PLL output is being used to clock the system.
    //
    if(!(ulConfig & LM3Sxx_RCC_RCC_BYPASS))
    {
        //
        // Wait until the PLL has locked.
        //
    	BT_u32 ulDelay;
        for(ulDelay = 32768; ulDelay > 0; ulDelay--)
        {
            if(pRCC->RIS & LM3Sxx_RCC_RIS_PLLLRIS)
            {
                break;
            }
        }

        //
        // Enable use of the PLL.
        //
        ulRCC &= ~(LM3Sxx_RCC_RCC_BYPASS);
        ulRCC2 &= ~(LM3Sxx_RCC_RCC2_BYPASS2);
    }

    //
    // Write the final RCC value.
    //
    pRCC->RCC = ulRCC;
    pRCC->RCC2 = ulRCC2;

    //
    // Delay for a little bit so that the system divider takes effect.
    //
    SysCtlDelay(16);

}
