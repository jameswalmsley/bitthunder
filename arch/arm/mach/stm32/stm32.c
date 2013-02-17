/**
 *	STM32 Platform Machine Description.
 *
 *	@author		James Walmsley
 *
 *	@copyright	(c)2012 James Walmsley <james@fullfat-fs.co.uk>
 **/

#include <bitthunder.h>

#include "uart.h"
#include "gpio.h"

static const BT_RESOURCE oSTM32_gpio_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_STM32_GPIO_BASE,
		.ulEnd 				= BT_CONFIG_MACH_STM32_GPIO_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= BT_CONFIG_MACH_STM32_TOTAL_GPIOS-1,
		.ulFlags			= BT_RESOURCE_IO,
	},
	{
		.ulStart			= 6,
		.ulEnd				= 10,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
	{
		.ulStart			= 23,
		.ulEnd				= 23,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
	{
		.ulStart			= 40,
		.ulEnd				= 40,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

/**
 *	By using the BT_INTEGRATED_DEVICE_DEF macro, we ensure that this structure is
 *	placed into the device manager's integrated device table.
 *
 *	This allows it to be automatically enumerated without "registering" a driver.
 **/
BT_INTEGRATED_DEVICE_DEF oSTM32_gpio_device = {
	.name 					= "stm32,gpio",
	.ulTotalResources 		= BT_ARRAY_SIZE(oSTM32_gpio_resources),
	.pResources 			= oSTM32_gpio_resources,
};

static const BT_RESOURCE oSTM32_nvic_resources[] = {
	{
		.ulStart			= BT_CONFIG_ARCH_ARM_NVIC_BASE,
		.ulEnd				= BT_CONFIG_ARCH_ARM_NVIC_BASE + BT_SIZE_4K,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS - 1,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oSTM32_nvic_device = {
	.name					= "arm,common,nvic",
	.ulTotalResources	   	= BT_ARRAY_SIZE(oSTM32_nvic_resources),
	.pResources				= oSTM32_nvic_resources,
};

static const BT_RESOURCE oSTM32_systick_resources[] = {
	{
		.ulStart 			= 0xE000E010,
		.ulEnd				= 0xE000E01F,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 16,
		.ulEnd				= 16,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oSTM32_systick_device = {
	.name					= "arm,cortex-mx,systick",
	.ulTotalResources		= BT_ARRAY_SIZE(oSTM32_systick_resources),
	.pResources				= oSTM32_systick_resources,
};

static BT_u32 stm32_get_cpu_clock_frequency() {
	return 8000000;
}

BT_MACHINE_START(ARM, CORTEX_M3, "STM32 Family Micro-controllers")
    .ulSystemClockHz 			= 8000000,
	.pfnGetCpuClockFrequency 	= stm32_get_cpu_clock_frequency,

	.pInterruptController		= &oSTM32_nvic_device,

	.pSystemTimer 				= &oSTM32_systick_device,

	.pBootLogger				= NULL,
	.ulBootUartID				= 0,
BT_MACHINE_END
