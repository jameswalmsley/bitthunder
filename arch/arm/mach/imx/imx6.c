#include <bitthunder.h>


static const BT_RESOURCE imx_intc_resources[] = {
	{
		.ulStart 			= BT_CONFIG_ARCH_ARM_CORTEX_A9_MPCORE_BASE + 0x0100,
		.ulEnd	 			= BT_CONFIG_ARCH_ARM_CORTEX_A9_MPCORE_BASE + 0x0100 + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= BT_CONFIG_ARCH_ARM_CORTEX_A9_MPCORE_BASE + 0x1000,
		.ulEnd				= BT_CONFIG_ARCH_ARM_CORTEX_A9_MPCORE_BASE + 0x1000 + BT_SIZE_4K - 1,
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
static const BT_INTEGRATED_DEVICE imx_intc_device = {
	.name 					= "arm,common,gic",						///< Name of the driver to handle this device.
	.ulTotalResources 		= BT_ARRAY_SIZE(imx_intc_resources),
	.pResources 			= imx_intc_resources,
};

static const BT_RESOURCE imx_cpu_timer_resources[] = {
	{
		.ulStart			= 0x00A00600,
		.ulEnd				= 0x00A006FF + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 29,									///< Start provides the IRQ of the private timer.
		.ulEnd				= 30,									///< End provides the IRQ of the watchdog timer.
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE imx_cpu_timer_device = {
	.name					= "arm,cortex-a9,cpu-timer",
	.ulTotalResources		= BT_ARRAY_SIZE(imx_cpu_timer_resources),
	.pResources				= imx_cpu_timer_resources,
};

static const BT_INTEGRATED_DEVICE imx_watchdog_device = {
	.name					= "arm,cortex-a9,wdt",
	.ulTotalResources		= BT_ARRAY_SIZE(imx_cpu_timer_resources),
	.pResources				= imx_cpu_timer_resources,
};

static BT_u32 imx_get_cpu_clock_frequency() {
	return 800000000;
}

static BT_ERROR imx_machine_init(struct _BT_MACHINE_DESCRIPTION *pMachine) {
	return BT_ERR_NONE;
}

BT_MACHINE_START(ARM, IMX, "Freescale IMX")
	.pfnMachineInit 			= imx_machine_init,
	.pfnGetCpuClockFrequency 	= imx_get_cpu_clock_frequency,
	.pInterruptController		= &imx_intc_device,
	.pSystemTimer 				= &imx_cpu_timer_device,
	//.pfnBootCore				= imx_boot_core,
#ifndef BT_CONFIG_OF
#ifdef BT_CONFIG_MACH_IMX_BOOTLOG_UART_NULL
	.pBootLogger				= NULL,
#endif
#ifdef BT_CONFIG_MACH_IMX_BOOTLOG_UART_0
	.pBootLogger				= &imx_uart0_device,
#endif
#ifdef BT_CONFIG_MACH_IMX_BOOTLOG_UART_1
	.pBootLogger				= &imx_uart1_device,
#endif
#endif
	//.pEarlyConsole				= &imx_early_console_device,	///< Provides really early low-level UART output, before memory management etc is available.
BT_MACHINE_END
