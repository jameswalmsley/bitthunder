#include <bitthunder.h>
#include "ioconfig.h"
#include "rcc.h"

void BT_LPC11xx_SetIOConfig(BT_u32 * pIOCON, BT_u32 config)
{
	volatile LPC11xx_RCC_REGS 	*pRCC   = LPC11xx_RCC;

	pRCC->SYSAHBCLKCTRL	|= LPC11xx_RCC_SYSAHBCLKCTRL_IOCON;
	*pIOCON = config;
}
