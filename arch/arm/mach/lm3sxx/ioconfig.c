#include <bitthunder.h>
#include "ioconfig.h"
#include "rcc.h"

void BT_LM3Sxx_SetIOConfig(BT_u32 ulPort, BT_u32 ulPin, BT_u32 ulFunction, BT_u32 ulMode, BT_u32 ulAnalogInput, BT_u32 ulOpenDrain) {
	volatile LM3Sxx_GPIO_REGS *pRegs = (LM3Sxx_GPIO_REGS *) BT_CONFIG_MACH_LM3Sxx_GPIO_BASE;

	BT_LM3Sxx_EnablePeripheral(64 + ulPort);

	pRegs->banks[ulPort].AFSEL &= ~(0x01 << ulPin);
	if (ulFunction != 0) {
		pRegs->banks[ulPort].AFSEL |= 0x01 << ulPin;
	}
	pRegs->banks[ulPort].PCTL &= ~(0x0F << (4*ulPin));
	pRegs->banks[ulPort].PCTL |= ulFunction << (4*ulPin);

	pRegs->banks[ulPort].ODR &= ~(0x01 << ulPin);
	if (ulOpenDrain) {
		pRegs->banks[ulPort].ODR |= 0x01 << ulPin;
	}

	pRegs->banks[ulPort].PDR &= ~(0x01 << ulPin);
	pRegs->banks[ulPort].PUR &= ~(0x01 << ulPin);
	if (ulMode == 1) {
		pRegs->banks[ulPort].PUR |= 0x01 << ulPin;
	}
	else if (ulMode == 2) {
		pRegs->banks[ulPort].PDR |= 0x01 << ulPin;
	}

	pRegs->banks[ulPort].DEN &= ~(0x01 << ulPin);
	pRegs->banks[ulPort].AMSEL &= ~(0x01 << ulPin);
	if (!ulAnalogInput) {
		pRegs->banks[ulPort].DEN |= 0x01 << ulPin;
	}
	else {
		pRegs->banks[ulPort].AMSEL |= 0x01 << ulPin;
	}

	pRegs->banks[ulPort].DR2R &= ~(0x01 << ulPin);
	pRegs->banks[ulPort].DR4R |=  (0x01 << ulPin);
	pRegs->banks[ulPort].DR8R &= ~(0x01 << ulPin);
	pRegs->banks[ulPort].SLR  &= ~(0x01 << ulPin);
}
