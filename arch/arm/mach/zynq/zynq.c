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
		.ulEnd				= 0xF8F006FF + BT_SIZE_4K - 1,
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


static const BT_RESOURCE oZynq_devcfg_resources[] = {
	{
		.ulStart			= 0xF8007000,
		.ulEnd				= 0xF8007000 + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
};

static const BT_INTEGRATED_DEVICE oZynq_devcfg_device = {
	.name 					= "zynq,devcfg",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_devcfg_resources),
	.pResources				= oZynq_devcfg_resources,
};

BT_DEVFS_INODE_DEF oZynq_devcfg_inode = {
	.szpName = "devcfg",
	.pDevice = &oZynq_devcfg_device,
};

#ifdef BT_CONFIG_MACH_ZYNQ_I2C_0
static const BT_RESOURCE oZynq_i2c0_resources[] = {
	{
		.ulStart			= BT_CONFIG_MACH_ZYNQ_I2C_0_BUSID,
		.ulEnd				= BT_CONFIG_MACH_ZYNQ_I2C_0_BUSID,
		.ulFlags			= BT_RESOURCE_BUSID,
	},
	{
		.ulStart			= 0xE0004000,
		.ulEnd				= 0xE0004000 + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart 			= 57,
		.ulEnd				= 57,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

BT_INTEGRATED_DEVICE_DEF oZynq_i2c0_device = {
	.name 					= "zynq,i2c",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_i2c0_resources),
	.pResources				= oZynq_i2c0_resources,
};

BT_DEVFS_INODE_DEF oZynq_i2c0_inode = {
	.szpName				= "i2c0",
	.pDevice				= &oZynq_i2c0_device,
};
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_I2C_1
static const BT_RESOURCE oZynq_i2c1_resources[] = {
	{
		.ulStart			= BT_CONFIG_MACH_ZYNQ_I2C_1_BUSID,
		.ulEnd				= BT_CONFIG_MACH_ZYNQ_I2C_1_BUSID,
		.ulFlags			= BT_RESOURCE_BUSID,
	},
	{
		.ulStart			= 0xE0005000,
		.ulEnd				= 0xE0005000 + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart 			= 80,
		.ulEnd				= 80,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

BT_INTEGRATED_DEVICE_DEF oZynq_i2c1_device = {
	.name 					= "zynq,i2c",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_i2c0_resources),
	.pResources				= oZynq_i2c0_resources,
};

BT_DEVFS_INODE_DEF oZynq_i2c1_inode = {
	.szpName				= "i2c1",
	.pDevice				= &oZynq_i2c0_device,
};
#endif


#ifdef BT_CONFIG_MACH_ZYNQ_UART_0
static const BT_RESOURCE oZynq_uart0_resources[] = {
	{
		.ulStart			= 0xE0000000,
		.ulEnd				= 0xE0000000 + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 59,									///< Start provides the IRQ of the private timer.
		.ulEnd				= 59,									///< End provides the IRQ of the watchdog timer.
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oZynq_uart0_device = {
	.name					= "zynq,uart",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_uart0_resources),
	.pResources				= oZynq_uart0_resources,
};


BT_DEVFS_INODE_DEF oZynq_uart0_inode = {
	.szpName = BT_CONFIG_MACH_ZYNQ_UART_0_INODE_NAME,
	.pDevice = &oZynq_uart0_device,
};
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_UART_1
static const BT_RESOURCE oZynq_uart1_resources[] = {
	{
		.ulStart			= 0xE0001000,
		.ulEnd				= 0xE0001000 + BT_SIZE_4K - 1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 82,									///< Start provides the IRQ of the private timer.
		.ulEnd				= 82,									///< End provides the IRQ of the watchdog timer.
		.ulFlags			= BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE oZynq_uart1_device = {
	.name					= "zynq,uart",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_uart1_resources),
	.pResources				= oZynq_uart1_resources,
};

BT_DEVFS_INODE_DEF oZynq_uart1_inode = {
	.szpName = BT_CONFIG_MACH_ZYNQ_UART_1_INODE_NAME,
	.pDevice = &oZynq_uart1_device,
};
#endif

static BT_ERROR zynq_boot_core(BT_u32 ulCoreID, void *p) {
	volatile BT_u32 *core2 = (BT_u32 *) 0xFFFFFFF0;
	*core2 = (BT_u32) p;
	BT_DCacheFlush();

	__asm volatile("sev");

	return BT_ERR_NONE;
}

static BT_u32 zynq_get_cpu_clock_frequency() {
	return BT_ZYNQ_GetCpuFrequency();
}

BT_MACHINE_START(ARM, ZYNQ, "Xilinx Embedded Zynq Platform")
	.pfnGetCpuClockFrequency 	= zynq_get_cpu_clock_frequency,
	.pInterruptController		= &oZynq_intc_device,
	.pSystemTimer 				= &oZynq_cpu_timer_device,
	.pfnBootCore				= zynq_boot_core,

#ifdef BT_CONFIG_MACH_ZYNQ_BOOTLOG_UART_NULL
	.pBootLogger				= NULL,
#endif
#ifdef BT_CONFIG_MACH_ZYNQ_BOOTLOG_UART_0
	.pBootLogger				= &oZynq_uart0_device,
#endif
#ifdef BT_CONFIG_MACH_ZYNQ_BOOTLOG_UART_1
	.pBootLogger				= &oZynq_uart1_device,
#endif
BT_MACHINE_END
