/**
 *	SD Host Controller Interface Driver Implementation for BitThunder.
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("Generic SDHCI Controller Driver")
BT_DEF_MODULE_DESCRIPTION	("Implements a driver for SDCARD abstraction for SDHCI controllers")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_ERROR sdhci_cleanup(BT_HANDLE hSDIO) {
	return BT_ERR_NONE;
}

static BT_ERROR sdhci_send_command(BT_u8 ucCommand, BT_u32 ulArgument, BT_u32 *pResponse) {
	return BT_ERR_NONE;

}

static BT_HANDLE sdhci_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {
	//BT_HANDLE hSDIO = BT_CreateHandle();

	return NULL;
}

static const BT_DEV_IF_SDIO oDeviceOps = {
	.pfnSendCommand = sdhci_send_command,
};

static const BT_IF_DEVICE oDeviceInterface = {
	.eConfigType = BT_DEV_IF_T_INTC,
	.unConfigIfs = {
		.pSdioIF = &oDeviceOps,
	},
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = sdhci_cleanup,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
	},
};

BT_INTEGRATED_DRIVER_DEF sdhci_driver = {
	.name		= "mmc,sdhci",
	.pfnProbe	= sdhci_probe,
};
