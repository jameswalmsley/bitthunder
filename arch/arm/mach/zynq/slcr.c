#include <bitthunder.h>
#include "slcr.h"

//static volatile ZYNQ_SLCR_REGS *const pRegs = (ZYNQ_SLCR_REGS *) (0xF8000000);

/*typedef enum _ZYNQ_PLL {


} ZYNQ_PLL;


BT_u32 BT_ZYNQ_GetPLLFrequency(ZYNQ_PLL ePLL) {

}*/

BT_u32 BT_ZYNQ_GetArmPLLFrequency() {

	ZYNQ_SLCR_REGS *pRegs = ZYNQ_SLCR;

	BT_u32 ctl = pRegs->ARM_PLL_CTRL;
	BT_BOOL	bBypassed = BT_FALSE;


	if(!(ctl & ZYNQ_SLCR_PLL_CTRL_BYPASS_FORCE)) {
		if(ctl & ZYNQ_SLCR_PLL_CTRL_BYPASS_QUAL) {
			if(pRegs->BOOT_MODE & ZYNQ_SLCR_BOOT_MODE_ARM_PLL_BYPASS) {
				bBypassed = BT_TRUE;
			}
		}
	} else {
		bBypassed = BT_TRUE;
	}

	if(bBypassed) {
		return BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ;
	}

	BT_u64 fdiv = ZYNQ_SLCR_PLL_CTRL_FDIV_VAL(ctl);
	BT_u64 val = (BT_u64) BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ;
	val *= fdiv;

	//BT_u64 clk_div = ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(pRegs->ARM_CLK_CTRL);

	return val;// / clk_div;
}

BT_u32 BT_ZYNQ_GetIOPLLFrequency() {

	ZYNQ_SLCR_REGS *pRegs = ZYNQ_SLCR;

	BT_u32 ctl = pRegs->IO_PLL_CTRL;
	BT_BOOL bBypassed = BT_FALSE;

	if(!(ctl & ZYNQ_SLCR_PLL_CTRL_BYPASS_FORCE)) {
		if(ctl & ZYNQ_SLCR_PLL_CTRL_BYPASS_QUAL) {
			if(pRegs->BOOT_MODE & ZYNQ_SLCR_BOOT_MODE_IO_PLL_BYPASS) {
				bBypassed = BT_TRUE;
			}
		}
	} else {
		bBypassed = BT_TRUE;
	}

	if(bBypassed) {
		return BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ;
	}

	BT_u64 fdiv = ZYNQ_SLCR_PLL_CTRL_FDIV_VAL(ctl);
	BT_u64 val = (BT_u64) BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ;
	val *= fdiv;

	//BT_u64 clk_div = ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(pRegs->IO_CLK_CTRL);

	return val;
}

BT_u32 BT_ZYNQ_GetDDRPLLFrequency() {
	ZYNQ_SLCR_REGS *pRegs = ZYNQ_SLCR;

	BT_u32 ctl = pRegs->DDR_PLL_CTRL;
	BT_BOOL bBypassed = BT_FALSE;

	if(!(ctl & ZYNQ_SLCR_PLL_CTRL_BYPASS_FORCE)) {
		if(ctl & ZYNQ_SLCR_PLL_CTRL_BYPASS_QUAL) {
			if(pRegs->BOOT_MODE & ZYNQ_SLCR_BOOT_MODE_IO_PLL_BYPASS) {
				bBypassed = BT_TRUE;
			}
		}
	} else {
		bBypassed = BT_TRUE;
	}

	if(bBypassed) {
		return BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ;
	}

	BT_u64 fdiv = ZYNQ_SLCR_PLL_CTRL_FDIV_VAL(ctl);
	BT_u64 val = (BT_u64) BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ;
	val *= fdiv;

	//BT_u64 clk_div = ZYNQ_SLCR_CLK_CTRL_DIVISOR_VAL(pRegs->IO_CLK_CTRL);

	return val;
}
