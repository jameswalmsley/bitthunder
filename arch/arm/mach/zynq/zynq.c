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
#include "timer.h"
#include "uart.h"
#include "gpio.h"
#include <arch/common/gic.h>

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

static const BT_RESOURCE oZynq_intc_resources[] = {
	{
		.ulStart 			= BT_CONFIG_ARCH_ARM_GIC_BASE,
		.ulEnd	 			= BT_CONFIG_ARCH_ARM_GIC_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= BT_CONFIG_ARCH_ARM_GIC_DIST_BASE,
		.ulEnd				= BT_CONFIG_ARCH_ARM_GIC_DIST_BASE + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,				// Total of 95 IRQ lines in the GIC on Zynq!
		.ulEnd				= BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS - 1,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

/**
 *	Here we don't use the BT_INTEGRATED_DEVICE_DEF macro, as we don't want it to be used
 *	by the device manager's automated enumeration table.
 *
 *	We have added this to the BT_MACHINE_START structure.
 **/
static const BT_INTEGRATED_DEVICE oZynq_intc_device = {
	.name 					= "arm,common,gic",						///< Name of the driver to handle this device.
	.ulTotalResources 		= BT_ARRAY_SIZE(oZynq_intc_resources),
	.pResources 			= oZynq_intc_resources,
};

BT_MACHINE_START(ARM, ZYNQ, "Xilinx Embedded Zynq Platform")
    .ulSystemClockHz 		= BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ,
	.pInterruptController	= &oZynq_intc_device,
	.ulTotalIRQs			= 95,
	.pSystemTimer 			= &BT_ZYNQ_TIMER_oDeviceInterface,
	.ulTimerID				= BT_CONFIG_MACH_ZYNQ_SYSTICK_TIMER_ID,
	.pBootUart				= &BT_ZYNQ_UART_oDeviceInterface,
	.ulBootUartID			= BT_CONFIG_MACH_ZYNQ_BOOT_UART_ID,
BT_MACHINE_END
