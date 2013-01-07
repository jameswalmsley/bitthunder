#include <bitthunder.h>
#include "devman/bt_integrated_device.h"





static const BT_RESOURCE bcm2835_gpio_resources[] = {
	{	
		.ulStart 	= 0x7E200000,
		.ulEnd	 	= 0x7E2000B0,
		.ulFlags	= BT_RESOURCE_MEM,
	},
};

static const BT_INTEGRATED_DEVICE bcm2835_gpio_device = {
	.name = "bcm2835,gpio",
	.ulTotalResources = BT_ARRAY_SIZE(bcm2835_gpio_resources),
	.pResources = bcm2835_gpio_resources,
};


static BT_ERROR bcm2835_init(struct _BT_MACHINE_DESCRIPTION *pMachine) {
	return BT_ERR_NONE;
}

BT_MACHINE_START(ARM, BCM2835, "Broadcom 2835 System-on-chip")
    .ulSystemClockHz	= 125000000,
	.pfnMachineInit		= bcm2835_init,
BT_MACHINE_END

