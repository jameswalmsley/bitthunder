/**
 *	LPC17xx Platform Machine Description.
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

static const BT_RESOURCE oLPC17xx_gpio_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_GPIO_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_GPIO_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= BT_CONFIG_MACH_LPC17xx_TOTAL_GPIOS-1,
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
BT_INTEGRATED_DEVICE_DEF oLPC17xx_gpio_device = {
	.name 					= "LPC17xx,gpio",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_gpio_resources),
	.pResources 			= oLPC17xx_gpio_resources,
};


static const BT_RESOURCE oLPC17xx_nvic_resources[] = {
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

static const BT_INTEGRATED_DEVICE oLPC17xx_nvic_device = {
	.name					= "arm,common,nvic",
	.ulTotalResources	   	= BT_ARRAY_SIZE(oLPC17xx_nvic_resources),
	.pResources				= oLPC17xx_nvic_resources,
};

static const BT_RESOURCE oLPC17xx_systick_resources[] = {
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

static const BT_INTEGRATED_DEVICE oLPC17xx_systick_device = {
	.name					= "arm,cortex-mx,systick",
	.ulTotalResources		= BT_ARRAY_SIZE(oLPC17xx_systick_resources),
	.pResources				= oLPC17xx_systick_resources,
};


static void lpc17xx_gpio_init(void) {
	#ifdef BT_CONFIG_LPC17xx_PIO0_0_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 0, BT_CONFIG_LPC17xx_PIO0_0_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_1_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 1, BT_CONFIG_LPC17xx_PIO0_1_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_2_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 2, BT_CONFIG_LPC17xx_PIO0_2_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_3_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 3, BT_CONFIG_LPC17xx_PIO0_3_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_4_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 4, BT_CONFIG_LPC17xx_PIO0_4_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_5_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 5, BT_CONFIG_LPC17xx_PIO0_5_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_6_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 6, BT_CONFIG_LPC17xx_PIO0_6_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_7_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 7, BT_CONFIG_LPC17xx_PIO0_7_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_8_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 8, BT_CONFIG_LPC17xx_PIO0_8_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_9_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 9, BT_CONFIG_LPC17xx_PIO0_9_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_10_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 10, BT_CONFIG_LPC17xx_PIO0_10_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_11_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 11, BT_CONFIG_LPC17xx_PIO0_11_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_15_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 15, BT_CONFIG_LPC17xx_PIO0_15_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_16_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 16, BT_CONFIG_LPC17xx_PIO0_16_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_17_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 17, BT_CONFIG_LPC17xx_PIO0_17_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_18_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 18, BT_CONFIG_LPC17xx_PIO0_18_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_19_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 19, BT_CONFIG_LPC17xx_PIO0_19_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_20_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 20, BT_CONFIG_LPC17xx_PIO0_20_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_21_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 21, BT_CONFIG_LPC17xx_PIO0_21_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_22_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 22, BT_CONFIG_LPC17xx_PIO0_22_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_23_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 23, BT_CONFIG_LPC17xx_PIO0_23_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_24_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 24, BT_CONFIG_LPC17xx_PIO0_24_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_25_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 25, BT_CONFIG_LPC17xx_PIO0_25_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_26_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 26, BT_CONFIG_LPC17xx_PIO0_26_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_27_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 27, BT_CONFIG_LPC17xx_PIO0_27_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_28_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 28, BT_CONFIG_LPC17xx_PIO0_28_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_29_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 29, BT_CONFIG_LPC17xx_PIO0_29_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO0_30_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO0, 30, BT_CONFIG_LPC17xx_PIO0_30_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_0_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 0, BT_CONFIG_LPC17xx_PIO1_0_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_1_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 1, BT_CONFIG_LPC17xx_PIO1_1_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_4_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 4, BT_CONFIG_LPC17xx_PIO1_4_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_8_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 8, BT_CONFIG_LPC17xx_PIO1_8_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_9_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 9, BT_CONFIG_LPC17xx_PIO1_9_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_10_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 10, BT_CONFIG_LPC17xx_PIO1_10_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_14_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 14, BT_CONFIG_LPC17xx_PIO1_14_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_15_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 15, BT_CONFIG_LPC17xx_PIO1_15_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_16_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 16, BT_CONFIG_LPC17xx_PIO1_16_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_17_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 17, BT_CONFIG_LPC17xx_PIO1_17_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_18_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 18, BT_CONFIG_LPC17xx_PIO1_18_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_19_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 19, BT_CONFIG_LPC17xx_PIO1_19_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_20_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 20, BT_CONFIG_LPC17xx_PIO1_20_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_21_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 21, BT_CONFIG_LPC17xx_PIO1_21_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_22_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 22, BT_CONFIG_LPC17xx_PIO1_22_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_23_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 23, BT_CONFIG_LPC17xx_PIO1_23_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_24_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 24, BT_CONFIG_LPC17xx_PIO1_24_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_25_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 25, BT_CONFIG_LPC17xx_PIO1_25_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_26_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 26, BT_CONFIG_LPC17xx_PIO1_26_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_27_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 27, BT_CONFIG_LPC17xx_PIO1_27_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_28_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 28, BT_CONFIG_LPC17xx_PIO1_28_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_29_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 29, BT_CONFIG_LPC17xx_PIO1_29_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_30_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 30, BT_CONFIG_LPC17xx_PIO1_30_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO1_31_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO1, 31, BT_CONFIG_LPC17xx_PIO1_31_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_0_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 0, BT_CONFIG_LPC17xx_PIO2_0_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_1_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 1, BT_CONFIG_LPC17xx_PIO2_1_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_2_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 2, BT_CONFIG_LPC17xx_PIO2_2_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_3_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 3, BT_CONFIG_LPC17xx_PIO2_3_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_4_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 4, BT_CONFIG_LPC17xx_PIO2_4_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_5_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 5, BT_CONFIG_LPC17xx_PIO2_5_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_6_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 6, BT_CONFIG_LPC17xx_PIO2_6_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_7_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 7, BT_CONFIG_LPC17xx_PIO2_7_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_8_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 8, BT_CONFIG_LPC17xx_PIO2_8_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_9_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 9, BT_CONFIG_LPC17xx_PIO2_9_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_10_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 10, BT_CONFIG_LPC17xx_PIO2_10_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_11_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 11, BT_CONFIG_LPC17xx_PIO2_11_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_12_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 12, BT_CONFIG_LPC17xx_PIO2_12_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO2_13_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO2, 13, BT_CONFIG_LPC17xx_PIO2_13_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO3_25_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO3, 25, BT_CONFIG_LPC17xx_PIO3_25_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO3_26_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO3, 26, BT_CONFIG_LPC17xx_PIO3_26_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO4_28_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO4, 28, BT_CONFIG_LPC17xx_PIO4_28_FUNCTION);
	#endif
	#ifdef BT_CONFIG_LPC17xx_PIO4_29_FUNCTION
		BT_LPC17xx_SetIOConfig(LPC17xx_PIO4, 29, BT_CONFIG_LPC17xx_PIO4_29_FUNCTION);
	#endif
}

static BT_u32 lpc17xx_get_cpu_clock_frequency() {
	return BT_LPC17xx_GetSystemFrequency();
}


static BT_u32 lpc17xx_machine_init() {

	BT_LPC17xx_SetSystemFrequency(BT_CONFIG_SYSCLK_CTRL,
								  BT_CONFIG_MAINPLLCLK_SRC,
								  BT_CONFIG_MAINPLLCLK_CTRL,
								  BT_CONFIG_SYSCLK_DIV,
								  BT_CONFIG_USBCLK_SRC,
								  BT_CONFIG_USBPLLCLK_CTRL,
								  BT_CONFIG_USBCLK_DIV);

	lpc17xx_gpio_init();
	return BT_ERR_NONE;
}


#ifdef BT_CONFIG_MACH_LPC17xx_UART_0
static const BT_RESOURCE oLPC17xx_uart0_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_UART0_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_UART0_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 21,
		.ulEnd				= 21,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_uart0_device = {
	.id						= 0,
	.name 					= "LPC17xx,usart",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_uart0_resources),
	.pResources 			= oLPC17xx_uart0_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_uart0_inode = {
	.szpName = BT_CONFIG_MACH_LPC17xx_UART_0_INODE_NAME,
	.pDevice = &oLPC17xx_uart0_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC17xx_UART_1
static const BT_RESOURCE oLPC17xx_uart1_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_UART1_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_UART1_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 22,
		.ulEnd				= 22,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_uart1_device = {
	.id						= 1,
	.name 					= "LPC17xx,usart",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_uart1_resources),
	.pResources 			= oLPC17xx_uart1_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_uart1_inode = {
	.szpName = BT_CONFIG_MACH_LPC17xx_UART_1_INODE_NAME,
	.pDevice = &oLPC17xx_uart1_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC17xx_UART_2
static const BT_RESOURCE oLPC17xx_uart2_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_UART2_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_UART2_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 23,
		.ulEnd				= 23,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_uart2_device = {
	.id						= 2,
	.name 					= "LPC17xx,usart",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_uart2_resources),
	.pResources 			= oLPC17xx_uart2_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_uart2_inode = {
	.szpName = BT_CONFIG_MACH_LPC17xx_UART_2_INODE_NAME,
	.pDevice = &oLPC17xx_uart2_device,
};
#endif

#ifdef BT_CONFIG_MACH_LPC17xx_UART_3
static const BT_RESOURCE oLPC17xx_uart3_resources[] = {
	{
		.ulStart 			= BT_CONFIG_MACH_LPC17xx_UART3_BASE,
		.ulEnd 				= BT_CONFIG_MACH_LPC17xx_UART3_BASE + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.ulStart			= 24,
		.ulEnd				= 24,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oLPC17xx_uart3_device = {
	.id						= 3,
	.name 					= "LPC17xx,usart",
	.ulTotalResources 		= BT_ARRAY_SIZE(oLPC17xx_uart3_resources),
	.pResources 			= oLPC17xx_uart3_resources,
};

const BT_DEVFS_INODE_DEF oLPC17xx_uart3_inode = {
	.szpName = BT_CONFIG_MACH_LPC17xx_UART_3_INODE_NAME,
	.pDevice = &oLPC17xx_uart3_device,
};
#endif



BT_MACHINE_START(ARM, CORTEX_M3, "LPC Microcontroller Platform")
    .ulSystemClockHz 			= BT_CONFIG_MACH_LPC17xx_SYSCLOCK_FREQ,
	.pfnGetCpuClockFrequency 	= lpc17xx_get_cpu_clock_frequency,
	.pfnMachineInit				= lpc17xx_machine_init,
	.pInterruptController		= &oLPC17xx_nvic_device,

	.pSystemTimer 				= &oLPC17xx_systick_device,


#ifdef BT_CONFIG_MACH_LPC17xx_BOOTLOG_UART_NULL
	.pBootLogger				= NULL,
#endif
#ifdef BT_CONFIG_MACH_LPC17xx_BOOTLOG_UART_0
	.pBootLogger				= &oLPC17xx_uart0_device,
#endif
#ifdef BT_CONFIG_MACH_LPC11xx_BOOTLOG_UART_1
	.pBootLogger				= &oLPC11xx_uart1_device,
#endif
#ifdef BT_CONFIG_MACH_LPC11xx_BOOTLOG_UART_2
	.pBootLogger				= &oLPC11xx_uart2_device,
#endif
#ifdef BT_CONFIG_MACH_LPC11xx_BOOTLOG_UART_3
	.pBootLogger				= &oLPC11xx_uart3_device,
#endif
BT_MACHINE_END
