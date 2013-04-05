/**
 *	LPC11xx Platform Machine Description.
 *
 *	@author		Robert Steinbauer
 *
 *	@copyright	(c)2013 Riegl Laser Measurement Systems GmBH
 *	@copyright	(c)2013 Robert Steinbauer <rsteinbauer@riegl.com>
 *
 **/


#include <bitthunder.h>

#include "rcc.h"
#include "ioconfig.h"
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


static const BT_RESOURCE oLPC11xx_nvic_resources[] = {
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

static const BT_INTEGRATED_DEVICE oLPC11xx_nvic_device = {
	.name					= "arm,common,nvic",
	.ulTotalResources	   	= BT_ARRAY_SIZE(oLPC11xx_nvic_resources),
	.pResources				= oLPC11xx_nvic_resources,
};

static const BT_RESOURCE oLPC11xx_systick_resources[] = {
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

static const BT_INTEGRATED_DEVICE oLPC11xx_systick_device = {
	.name					= "arm,cortex-mx,systick",
	.ulTotalResources		= BT_ARRAY_SIZE(oLPC11xx_systick_resources),
	.pResources				= oLPC11xx_systick_resources,
};

static BT_u32 lpc11xx_get_cpu_clock_frequency() {
	return BT_LPC11xx_GetSystemFrequency();
}


static BT_u32 lpc11xx_machine_init() {

	BT_LPC11xx_SetSystemFrequency(BT_CONFIG_MAINCLK_SRC,
								  BT_CONFIG_SYSCLK_CTRL,
								  BT_CONFIG_PLLCLK_SRC,
								  BT_CONFIG_PLLCLK_CTRL,
								  BT_CONFIG_WDTCLK_CTRL,
								  BT_CONFIG_SYSCLK_DIV);

#ifdef BT_CONFIG_PIO0_4_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO0_4, BT_CONFIG_PIO0_4_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO0_5_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO0_5, BT_CONFIG_PIO0_5_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO0_8_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO0_8, BT_CONFIG_PIO0_8_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_0_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_0, BT_CONFIG_PIO1_0_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_1_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_1, BT_CONFIG_PIO1_1_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_2_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_2, BT_CONFIG_PIO1_2_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_3_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_3, BT_CONFIG_PIO1_3_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_4_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_4, BT_CONFIG_PIO1_4_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_5_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_5, BT_CONFIG_PIO1_5_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_6_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_6, BT_CONFIG_PIO1_6_FUNCTION);
#endif
#ifdef BT_CONFIG_PIO1_7_FUNCTION
	BT_LPC11xx_SetIOConfig(LPC11xx_PIO1_7, BT_CONFIG_PIO1_7_FUNCTION);
#endif

	return BT_ERR_NONE;
}


BT_MACHINE_START(ARM, CORTEX_M0, "LPC Microcontroller Platform")
    .ulSystemClockHz 			= BT_CONFIG_MACH_LPC11xx_SYSCLOCK_FREQ,
	.pfnGetCpuClockFrequency 	= lpc11xx_get_cpu_clock_frequency,
	.pfnMachineInit				= lpc11xx_machine_init,
	.pInterruptController		= &oLPC11xx_nvic_device,

	.pSystemTimer 				= &oLPC11xx_systick_device,


#ifdef MACH_LPC11xx_BOOTLOG_UART_NULL
	.pBootLogger				= NULL,
#endif
#ifdef MACH_LPC11xx_BOOTLOG_UART_0
	.pBootLogger				= &oLPC11xx_uart0_device,
#endif
BT_MACHINE_END
