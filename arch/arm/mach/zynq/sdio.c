/**
 *	This file describes the SDIO device on the Zynq platform.
 *
 **/
#include <bitthunder.h>

#include "slcr.h"
//#include "sdio.h"
#include "mmc/host.h"
#include <mmc/host/sdhci.h>

#if defined (BT_CONFIG_MACH_ZYNQ_SDIO_0) || defined(BT_CONFIG_MACH_ZYNQ_SDIO_1)
static BT_u32 sdio_get_input_clock(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {
	return 50000000;
}

static BT_MMC_HOST_OPS host_ops = {
	.pfnGetInputClock = sdio_get_input_clock,
};
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_SDIO_0
static const BT_RESOURCE oZynq_sdio_resources_0[] = {
	{
		.ulStart 			= 0xE0100000,
		.ulEnd 				= 0xE0100000 + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 56,
		.ulEnd				= 56,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
	{
		.ulStart 			= 0,
		.ulEnd				= 0,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.pParam				= &host_ops,
		.ulFlags			= BT_RESOURCE_PARAM,
	},
	{
#ifdef BT_CONFIG_MACH_ZYNQ_SDIO_0_ALWAYS_PRESENT
		.ulConfigFlags		= SDHCI_FLAGS_ALWAYS_PRESENT,
#else
		.ulConfigFlags		= 0,
#endif
		.ulFlags			= BT_RESOURCE_FLAGS,
	},
};

BT_INTEGRATED_DEVICE_DEF oZynq_sdio_device_0 = {
	.name				= "zynq,mmc,sdhci",
	.ulTotalResources	= BT_ARRAY_SIZE(oZynq_sdio_resources_0),
	.pResources			= oZynq_sdio_resources_0,
};
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_SDIO_1
static const BT_RESOURCE oZynq_sdio_resources_1[] = {
	{
		.ulStart 			= 0xE0101000,
		.ulEnd 				= 0xE0101000 + BT_SIZE_4K - 1,
		.ulFlags 			= BT_RESOURCE_MEM,
	},
	{
		.ulStart			= 79,
		.ulEnd				= 79,
		.ulFlags			= BT_RESOURCE_IRQ,
	},
	{
		.ulStart 			= 1,
		.ulEnd				= 1,
		.ulFlags			= BT_RESOURCE_ENUM,
	},
	{
		.pParam				= &host_ops,
		.ulFlags			= BT_RESOURCE_PARAM,
	},
	{
#ifdef BT_CONFIG_MACH_ZYNQ_SDIO_1_ALWAYS_PRESENT
		.ulConfigFlags		= SDHCI_FLAGS_ALWAYS_PRESENT,
#else
		.ulConfigFlags		= 0,
#endif
		.ulFlags			= BT_RESOURCE_FLAGS,
	},
};

BT_INTEGRATED_DEVICE_DEF oZynq_sdio_device_1 = {
	.name				= "zynq,mmc,sdhci",
	.ulTotalResources	= BT_ARRAY_SIZE(oZynq_sdio_resources_1),
	.pResources			= oZynq_sdio_resources_1,
};
#endif


/**
 *	This is really acting as a filter driver, so only the zynq HAL can know if the SDHCI
 *	controller is enabled.
 *
 *	If it is, we ask the generic SDHCI driver to do its stuff, otherwise we return an error.
 **/
static BT_HANDLE zynq_sdhci_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	BT_BOOL bEnabled = BT_FALSE;
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_ENUM, 0);
	if(!pResource) {
		goto err_out;
	}

	switch(pResource->ulStart) {
	case 0:
		bEnabled = (ZYNQ_SLCR->SDIO_CLK_CTRL & ZYNQ_SLCR_CLK_CTRL_CLKACT_0);
		if(bEnabled) {	// Attempt to reset the device!
			ZYNQ_SLCR->SLCR_UNLOCK = 0xDF0D;
			ZYNQ_SLCR->SDIO_RST_CTRL |= 0x11;
			ZYNQ_SLCR->SDIO_RST_CTRL &= ~0x11;
			ZYNQ_SLCR->SLCR_LOCK = 0x767B;
		}
		break;

	case 1:
		bEnabled = (ZYNQ_SLCR->SDIO_CLK_CTRL & ZYNQ_SLCR_CLK_CTRL_CLKACT_0);
		if(bEnabled) {
			ZYNQ_SLCR->SLCR_UNLOCK = 0xDF0D;
			ZYNQ_SLCR->SDIO_RST_CTRL |= 0x22;
			ZYNQ_SLCR->SDIO_RST_CTRL &= ~0x22;
			ZYNQ_SLCR->SLCR_LOCK = 0x767B;
		}
		break;

	default:
		break;
	}

	if(!bEnabled) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName("mmc,sdhci");
	if(!pDriver) {
		Error = BT_ERR_GENERIC;
		goto err_out;
	}

	return pDriver->pfnProbe(pDevice, pError);

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER_DEF zynq_sdhci_driver = {
	.name 				= "zynq,mmc,sdhci",
	.pfnProbe			= zynq_sdhci_probe,
};
