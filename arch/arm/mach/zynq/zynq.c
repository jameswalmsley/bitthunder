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
#include <string.h>

#include "slcr.h"
#include "uart.h"
#include "gpio.h"
#include "qspi.h"

#ifdef BT_CONFIG_MACH_ZYNQ_GPIO
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
#endif

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

#ifdef BT_CONFIG_MACH_ZYNQ_DEVCFG
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
	.szpName = "xdevcfg",
	.pDevice = &oZynq_devcfg_device,
};
#endif

#ifndef BT_CONFIG_OF
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
	.name 					= "xlnx,ps7-i2c-1.00.a",
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
	.name 					= "xlnx,ps7-i2c-1.00.a",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_i2c1_resources),
	.pResources				= oZynq_i2c1_resources,
};

BT_DEVFS_INODE_DEF oZynq_i2c1_inode = {
	.szpName				= "i2c1",
	.pDevice				= &oZynq_i2c1_device,
};
#endif
#endif

#ifndef BT_CONFIG_OF
#ifdef BT_CONFIG_MACH_ZYNQ_QSPI
static const BT_RESOURCE oZynq_qspi_resources[] = {
	{
		.ulStart			= ZYNQ_QSPI_CONTROLLER_BASE,
		.ulEnd				= ZYNQ_QSPI_CONTROLLER_BASE + BT_SIZE_1K -1,
		.ulFlags			= BT_RESOURCE_MEM,
	},
	{
		.ulStart 			= 51,
		.ulEnd				= 51,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
	{
		.ulStart			= 0,			// bus_num
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_BUSID,
	},
	{
		.ulStart 			= 1,			// num_cs
		.ulEnd				= 1,
		.ulFlags 			= BT_RESOURCE_NUM_CS,
	},
	{
		.ulStart			= 0,			// is_dual
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_FLAGS,
	},
};

BT_INTEGRATED_DEVICE_DEF oZynq_qspi_device = {
	.name					= "xlnx,ps7-qspi-1.00.a",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_qspi_resources),
	.pResources				= oZynq_qspi_resources,
};
#endif
#endif

#ifndef BT_CONFIG_OF
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
	.name					= "xlnx,xuartps",
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
	.name					= "xlnx,xuartps",
	.ulTotalResources		= BT_ARRAY_SIZE(oZynq_uart1_resources),
	.pResources				= oZynq_uart1_resources,
};

BT_DEVFS_INODE_DEF oZynq_uart1_inode = {
	.szpName = BT_CONFIG_MACH_ZYNQ_UART_1_INODE_NAME,
	.pDevice = &oZynq_uart1_device,
};
#endif
#endif

extern BT_u32 zynq_trampoline_jump, zynq_trampoline, zynq_trampoline_end;

static BT_ERROR zynq_boot_core(BT_u32 ulCoreID, void *address, bt_register_t a, bt_register_t b, bt_register_t c, bt_register_t d) {

	BT_u32 trampoline_size = (&zynq_trampoline_end - &zynq_trampoline) * 4;

	volatile BT_u32 *zero = bt_ioremap(0, trampoline_size);
	memcpy((void *) zero, &zynq_trampoline, trampoline_size);

	zero += (&zynq_trampoline_jump - &zynq_trampoline);

	zero[0] = (BT_u32) address;
	zero[1] = a;
	zero[2] = b;
	zero[3] = c;
	zero[4] = d;

	BT_DCacheFlush();
	BT_DCacheDisable();

	//bt_iounmap(zero);

	zynq_slcr_cpu_stop(ulCoreID);
	zynq_slcr_cpu_start(ulCoreID);

	while(1) {
		if(zero[0] != (BT_u32) address) {
			break;
		}
	}

	return BT_ERR_NONE;
}

static BT_u32 zynq_get_cpu_clock_frequency() {
	return BT_ZYNQ_GetCpuFrequency();
}

extern BT_ERROR arm_pl310_init();

static BT_ERROR zynq_machine_init(struct _BT_MACHINE_DESCRIPTION *pMachine) {
	arm_pl310_init();
	zynq_slcr_init();
	return BT_ERR_NONE;
}

/**
 *	This function runs in a super low-level environment, from the OCM.
 *	It can completely run all over the DDR-RAM space from 0x00100000+
 *
 **/

static const char hex_table[16] = {
	'0',
	'1',
	'2',
	'3',
	'4',
	'5',
	'6',
	'7',
	'8',
	'9',
	'a',
	'b',
	'c',
	'd',
	'e',
	'f',
};

void bt_zynq_ram_test_error(const BT_i8 *reason, BT_u32 addr) {
	bt_early_printk(reason, -1);
	BT_u32 i;
	for(i = 0; i < 8; i++) {
		BT_u8 nibble = (addr >> (28 - (4 * i))) & 0xF;
		bt_early_printk(&hex_table[nibble], 1);
	}

	bt_early_printk("\n", 1);
}

void bt_zynq_ram_test() {
	bt_early_printk_init();
	bt_early_printk("Low-Level RAM Test.\n", -1);

	BT_u32 i;
	BT_u32 *p = 0xF0100000;
	for(i = 0; i < 0x00100000; i++) {
		p[i] = ~i;
	}

	for(i = 0; i < 0x00100000; i++) {
		if(p[i] != ~i) {
			bt_early_printk("Error: Inconsistent data in memory.\n", -1);
			BT_u32 addr = (BT_u32)  p + i;
			bt_early_printk("Address: 0x", -1);
			for(i = 0; i < 8; i++) {
				BT_u8 nibble = (addr >> (28 - (4 * i))) & 0xF;
				bt_early_printk(&hex_table[nibble], 1);
			}
			bt_early_printk("\n", 1);
			break;
		}
	}
}

extern BT_DEV_IF_EARLY_CONSOLE oZynq_early_console_device;

BT_MACHINE_START(ARM, ZYNQ, "Xilinx Embedded Zynq Platform")
	.pfnMachineInit 			= zynq_machine_init,
	.pfnGetCpuClockFrequency 	= zynq_get_cpu_clock_frequency,
	.pInterruptController		= &oZynq_intc_device,
	.pSystemTimer 				= &oZynq_cpu_timer_device,
	.pfnBootCore				= zynq_boot_core,
#ifndef BT_CONFIG_OF
#ifdef BT_CONFIG_MACH_ZYNQ_BOOTLOG_UART_NULL
	.pBootLogger				= NULL,
#endif
#ifdef BT_CONFIG_MACH_ZYNQ_BOOTLOG_UART_0
	.pBootLogger				= &oZynq_uart0_device,
#endif
#ifdef BT_CONFIG_MACH_ZYNQ_BOOTLOG_UART_1
	.pBootLogger				= &oZynq_uart1_device,
#endif
#endif
BT_MACHINE_END
