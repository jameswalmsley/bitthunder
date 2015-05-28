#include <bitthunder.h>
#include "gpio.h"
#include "devman/bt_integrated_device.h"


static const BT_RESOURCE bcm2835_gpio_resources[] = {
	{
		.ulStart 	= 0x20200000,
		.ulEnd	 	= 0x202000B0,
		.ulFlags	= BT_RESOURCE_MEM,
	},
	{
		.ulStart	= 0,
		.ulEnd		= 53,
		.ulFlags	= BT_RESOURCE_IO,
	},
};

BT_INTEGRATED_DEVICE_DEF bcm2835_gpio_device = {
	.name = "bcm2835,gpio",
	.ulTotalResources = BT_ARRAY_SIZE(bcm2835_gpio_resources),
	.pResources = bcm2835_gpio_resources,
};

static const BT_RESOURCE bcm2835_intc_resources[] = {
	{
		.ulStart 	= 0x2000B200,
		.ulEnd		= 0x2000B200 + BT_SIZE_4K - 1,
		.ulFlags	= BT_RESOURCE_MEM,
	},
	{
		.ulStart = 0,
		.ulEnd	 = 71,
		.ulFlags = BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE bcm2835_intc_device = {
	.name 				= "bcm2835,intc",
	.ulTotalResources 	= BT_ARRAY_SIZE(bcm2835_intc_resources),
	.pResources			= bcm2835_intc_resources,
};

static const BT_RESOURCE oTimerResources[] = {
	{
		.ulStart = 0x2000B400,
		.ulEnd	 = 0x2000B400 + BT_SIZE_4K - 1,
		.ulFlags = BT_RESOURCE_MEM,
	},
	{
		.ulStart = 64,
		.ulEnd 	 = 64,
		.ulFlags = BT_RESOURCE_IRQ,
	},
};

static const BT_INTEGRATED_DEVICE bcm2835_cpu_timer_device = {
	.name = "bcm2835,timer",
	.ulTotalResources = BT_ARRAY_SIZE(oTimerResources),
	.pResources = oTimerResources,
};

static const BT_RESOURCE oUartResources[] = {
	{
		.ulStart = 0x20215000,
		.ulEnd	 = 0x20215000 + BT_SIZE_4K - 1,
		.ulFlags = BT_RESOURCE_MEM,
	},
};

static const BT_INTEGRATED_DEVICE bcm2835_uart_0_device = {
	.name = "bcm2835,aux,uart",
	.ulTotalResources = BT_ARRAY_SIZE(oUartResources),
	.pResources = oUartResources,
};

static BT_ERROR bcm2835_init(struct _BT_MACHINE_DESCRIPTION *pMachine) {

	volatile BCM2835_GPIO_REGS *pRegs = bt_ioremap((bcm2835_gpio_resources[0].pStart), BT_SIZE_4K);

	// UART enable gpios
	pRegs->GPFSEL[1] &= ~(7 << 12);
	pRegs->GPFSEL[1] |= 2 << 12;

	return BT_ERR_NONE;
}

static BT_u32 bcm2835_get_cpu_frequency() {
	return BT_CONFIG_MACH_BCM2835_SYSCLOCK_FREQ;
}

BT_MACHINE_START(ARM, BCM2835, "Broadcom 2835 System-on-chip")
.pfnGetCpuClockFrequency 	= bcm2835_get_cpu_frequency,
    .ulSystemClockHz		= 125000000,
	.pfnMachineInit			= bcm2835_init,
	.pInterruptController 	= &bcm2835_intc_device,
	.pSystemTimer 			= &bcm2835_cpu_timer_device,
	.pBootLogger 			= &bcm2835_uart_0_device,
BT_MACHINE_END
