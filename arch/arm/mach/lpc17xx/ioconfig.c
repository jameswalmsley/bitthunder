#include <bitthunder.h>
#include "ioconfig.h"
#include "rcc.h"

void BT_LPC17xx_SetIOConfig(BT_u32 * pIOCON, BT_u32 ulIO, BT_u32 config) {
	if (ulIO > 15) pIOCON++;
	*pIOCON &= ~(0x3 << (2*ulIO));
	*pIOCON |= (config << (2*ulIO));
}
