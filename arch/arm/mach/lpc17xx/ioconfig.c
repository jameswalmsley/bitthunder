#include <bitthunder.h>
#include "ioconfig.h"
#include "rcc.h"

void BT_LPC17xx_SetIOConfig(BT_u32 ulIO, BT_u32 ulFunction, BT_u32 ulMode, BT_BOOL bOpenDrain) {
	volatile LPC17xx_IOCON_REGS * pRegs = LPC17xx_IOCON;

	BT_u32 ulPort 	 = ulIO / 32;
	BT_u32 ulPin 	 = ulIO % 32;
	BT_u32 ulPinsel  = (ulIO % 16)*2;
	BT_u32 ulPortSel = ulIO / 16;

	pRegs->LPC17xx_PINSELP[ulPortSel] &= ~(0x03 << ulPinsel);
	pRegs->LPC17xx_PINSELP[ulPortSel] |= ((ulFunction & 0x03) << ulPinsel);

	pRegs->LPC17xx_PINMODE[ulPortSel] &= ~(0x03 << ulPinsel);
	pRegs->LPC17xx_PINMODE[ulPortSel] |= ((ulMode & 0x03) << ulPinsel);

	pRegs->LPC17xx_PINMODE_OD[ulPort] &= ~ulPin;
	if (bOpenDrain)
		pRegs->LPC17xx_PINMODE_OD[ulPort] |= (0x01 << ulPin);
}
