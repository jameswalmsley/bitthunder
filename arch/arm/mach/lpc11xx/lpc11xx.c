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

static const BT_RESOURCE oZynq_gpio_resources[] = {
	{
		.ulStart 			= ZYNQ_GPIO_BASE,
		.ulEnd 				= ZYNQ_GPIO_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 53,
		.ulFlags			= BT_RESOURCE_IO,
	},
	{
		.ulStart			= 52,
		.ulEnd				= 52,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

/**
 *	By using the BT_INTEGRATED_DEVICE_DEF macro, we ensure that this structure is
 *	placed into the device manager's integrated device table.
 *
 *	This allows it to be automatically enumerated without "registering" a driver.
 **/
BT_INTEGRATED_DEVICE_DEF oZynq_gpio_device = {
	.name 					= "zynq,gpio",
	.ulTotalResources 		= BT_ARRAY_SIZE(oZynq_gpio_resources),
	.pResources 			= oZynq_gpio_resources,
};

static BT_u32 zynq_get_cpu_clock_frequency() {
	return 0;
}

BT_MACHINE_START(ARM, LPC11xx, "LPC Microcontroller Platform")
    .ulSystemClockHz 			= 8000000,
	.pfnGetCpuClockFrequency 	= zynq_get_cpu_clock_frequency,

	.pInterruptController		= NULL,

	.pSystemTimer 				= NULL,

	.pBootLogger				= NULL,
	.ulBootUartID				= 0,
BT_MACHINE_END
