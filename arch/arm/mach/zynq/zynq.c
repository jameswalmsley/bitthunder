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
#include <arch/common/gic.h>
#include <arch/common/cortex-a9-cpu-timers.h>

#include "slcr.h"
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

static const BT_RESOURCE oZynq_cpu_timer_resources[] = {
	{
		.ulStart			= 0xF8F00600,
		.ulEnd				= 0xF8F006FF,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 29,									///< Start provides the IRQ of the private timer.
		.ulEnd				= 30,									///< End provides the IRQ of the watchdog timer.
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oZynq_cpu_timer_device = {
	.name					= "arm,cortex-a9,cpu-timer",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_cpu_timer_resources),
	.pResources				= oZynq_cpu_timer_resources,
};

static const BT_INTEGRATED_DEVICE oZynq_watchdog_device = {
	.name					= "arm,cortex-a9,wdt",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_cpu_timer_resources),
	.pResources				= oZynq_cpu_timer_resources,
};

static BT_u32 zynq_get_cpu_clock_frequency() {
	return BT_ZYNQ_GetCpuFrequency();
}

BT_MACHINE_START(ARM, ZYNQ, "Xilinx Embedded Zynq Platform")
    .ulSystemClockHz 			= BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ,
	.pfnGetCpuClockFrequency 	= zynq_get_cpu_clock_frequency,

	.pInterruptController		= &oZynq_intc_device,

	.pSystemTimer 				= &oZynq_cpu_timer_device,

	.pBootLogger				= &BT_ZYNQ_UART_oDeviceInterface,
	.ulBootUartID				= BT_CONFIG_MACH_ZYNQ_BOOT_UART_ID,
BT_MACHINE_END
