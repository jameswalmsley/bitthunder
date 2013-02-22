/**
 *	Zynq Platform Machine Description.
 *
 *	@author		James Walmsley
 *
 *	@copyright	(c)2012 Riegl Laser Measurement Systems GmBH
 *	@copyright	(c)2012 James Walmsley <james@fullfat-fs.co.uk>
 *
 **/


#include <bitthunder.h>

#include "uart.h"
#include "gpio.h"

static const BT_RESOURCE oLPC11xx_gpio_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC11xx_GPIO_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC11xx_GPIO_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= BT_CONFIG_MACH_LPC11xx_TOTAL_GPIOS-1,
		.ulFlags			= BT_RESOURCE_IO,
	},
	{
		.ulStart			= 28,
		.ulEnd				= 31,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

/**
 *	By using the BT_INTEGRATED_DEVICE_DEF macro, we ensure that this structure is
 *	placed into the device manager's integrated device table.
 *
 *	This allows it to be automatically enumerated without "registering" a driver.
 **/
BT_INTEGRATED_DEVICE_DEF oLPC11xx_gpio_device = {
	.name 					= "LPC11xx,gpio",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC11xx_gpio_resources),
	.pResources 			= oLPC11xx_gpio_resources,
};

static BT_u32 zynq_get_cpu_clock_frequency() {
	return 0;
}

BT_MACHINE_START(ARM, CORTEX_M0, "LPC Microcontroller Platform")
    .ulSystemClockHz 			= 8000000,
	.pfnGetCpuClockFrequency 	= zynq_get_cpu_clock_frequency,

	.pInterruptController		= NULL,

	.pSystemTimer 				= NULL,

	.pBootLogger				= NULL,
	.ulBootUartID				= 0,
BT_MACHINE_END
