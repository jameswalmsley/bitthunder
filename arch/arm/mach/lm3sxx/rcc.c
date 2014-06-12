#include <bitthunder.h>
#include "rcc.h"


static const unsigned long g_pulXtals[] =
{
    1000000,
    1843200,
    2000000,
    2457600,
    3579545,
    3686400,
    4000000,
    4096000,
    4915200,
    5000000,
    5120000,
    6000000,
    6144000,
    7372800,
    8000000,
    8192000,
    10000000,
    12000000,
    12288000,
    13560000,
    14318180,
    16000000,
    16384000
};

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

	return 0;
}


BT_u32 BT_LM3Sxx_GetMainFrequency(void)
{
	volatile LM3Sxx_RCC_REGS *pRegs = LM3Sxx_RCC;

    unsigned long ulRCC, ulRCC2, ulPLL, ulClk;

    // Read RCC and RCC2.  For Sandstorm-class devices (which do not have
    // RCC2), the RCC2 read will return 0, which indicates that RCC2 is
    // disabled (since the LM3Sxx_RCC_RCC2_BYPASS2 bit is clear).
    ulRCC = pRegs->RCC;
    ulRCC2 = pRegs->RCC2;

    // Get the base clock rate.
    switch((ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2) ?
           (ulRCC2 & LM3Sxx_RCC_RCC2_OSCSRC2_M) :
           (ulRCC & LM3Sxx_RCC_RCC_OSCSRC_M))
    {
        // The main oscillator is the clock source.  Determine its rate from
        // the crystal setting field.
        case LM3Sxx_RCC_RCC_OSCSRC_MAIN: {
            ulClk = g_pulXtals[(ulRCC & LM3Sxx_RCC_RCC_XTAL_M) >>
                               LM3Sxx_RCC_RCC_XTAL_S];
            break;
        }

        // The internal oscillator is the source clock.
        case LM3Sxx_RCC_RCC_OSCSRC_INT: {
            // The internal oscillator on all other devices is 16 MHz.
            ulClk = 16000000;
            break;
        }

        // The internal oscillator divided by four is the source clock.
        case LM3Sxx_RCC_RCC_OSCSRC_INT4: {
            // The internal oscillator on a Tempest-class device is 16 MHz.
            ulClk = 16000000 / 4;
            break;
        }

        // The internal 30 KHz oscillator is the source clock.
        case LM3Sxx_RCC_RCC_OSCSRC_30: {
            // The internal 30 KHz oscillator has an accuracy of +/- 30%.
            ulClk = 30000;
            break;
        }

        // The 4.19 MHz clock from the hibernate module is the clock source.
        case LM3Sxx_RCC_RCC2_OSCSRC2_419: {
            ulClk = 4194304;
            break;
        }

        // The 32 KHz clock from the hibernate module is the source clock.
        case LM3Sxx_RCC_RCC2_OSCSRC2_32:{
            ulClk = 32768;
            break;
        }

        // An unknown setting, so return a zero clock (that is, an unknown
        // clock rate).
        default: {
            return(0);
        }
    }

    // See if the PLL is being used.
    if(((ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2) && !(ulRCC2 & LM3Sxx_RCC_RCC2_BYPASS2)) ||
       (!(ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2) && !(ulRCC & LM3Sxx_RCC_RCC_BYPASS)))
    {
        //
        // Get the PLL configuration.
        //
        ulPLL =pRegs->PLLCFG;

        // Compute the PLL output frequency based on its input frequency.
        // The formula for a Fury-class device is
        // "(xtal * f) / ((r + 1)*2)".
        //
        ulClk = ((ulClk * ((ulPLL & LM3Sxx_RCC_PLLCFG_F_M) >>
                           LM3Sxx_RCC_PLLCFG_F_S)) /
                 ((((ulPLL & LM3Sxx_RCC_PLLCFG_R_M) >>
                    LM3Sxx_RCC_PLLCFG_R_S) + 1)*2));

        // See if the optional output divide by 2 is being used.
        if(ulPLL & LM3Sxx_RCC_PLLCFG_OD_2) {
            ulClk /= 2;
        }

        // See if the optional output divide by 4 is being used.
        if(ulPLL & LM3Sxx_RCC_PLLCFG_OD_4)
        {
            ulClk /= 4;
        }

        //
        // Force the system divider to be enabled.  It is always used when
        // using the PLL, but in some cases it will not read as being enabled.
        //
        ulRCC |= LM3Sxx_RCC_RCC_USESYSDIV;
    }

    //
    // See if the system divider is being used.
    //
    if(ulRCC & LM3Sxx_RCC_RCC_USESYSDIV)
    {
        //
        // Adjust the clock rate by the system clock divider.
        //
        if(ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2)
        {
            if((ulRCC2 & LM3Sxx_RCC_RCC2_DIV400) &&
               (((ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2) &&
                 !(ulRCC2 & LM3Sxx_RCC_RCC2_BYPASS2)) ||
                (!(ulRCC2 & LM3Sxx_RCC_RCC2_USERCC2) &&
                 !(ulRCC & LM3Sxx_RCC_RCC_BYPASS))))

            {
                ulClk = ((ulClk * 2) / (((ulRCC2 & (LM3Sxx_RCC_RCC2_SYSDIV2_M |
                                                    LM3Sxx_RCC_RCC2_SYSDIV2LSB)) >>
                                         (LM3Sxx_RCC_RCC2_SYSDIV2_S - 1)) + 1));
            }
            else
            {
                ulClk /= (((ulRCC2 & LM3Sxx_RCC_RCC2_SYSDIV2_M) >>
                           LM3Sxx_RCC_RCC2_SYSDIV2_S) + 1);
            }
        }
        else
        {
            ulClk /= (((ulRCC & LM3Sxx_RCC_RCC_SYSDIV_M) >> LM3Sxx_RCC_RCC_SYSDIV_S) +
                      1);
        }
    }

    // Return the computed clock rate.
    return(ulClk);
}

BT_u32 BT_LM3Sxx_GetSystemFrequency(void) {

	return BT_LM3Sxx_GetMainFrequency();
}

void BT_LM3Sxx_SetSystemFrequency(BT_u32 ulConfig) {
	volatile LM3Sxx_RCC_REGS *pRCC = LM3Sxx_RCC;

	BT_u32 i;
	unsigned long ulRCC, ulRCC2;


	for (i = 0; i < 1000000; i++);

    // Get the current value of the RCC and RCC2 registers.  If using a
    // Sandstorm-class device, the RCC2 register will read back as zero and the
    // writes to it from within this function will be ignored.
    ulRCC = pRCC->RCC;
    ulRCC2 = pRCC->RCC2;

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
    if(ulRCC2 & LM3Sxx_RCC_RCC2_BYPASS2)
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
