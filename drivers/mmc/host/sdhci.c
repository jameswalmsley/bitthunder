/**
 *	SD Host Controller Interface Driver Implementation for BitThunder.
 **/

#include <bitthunder.h>
#include "sdhci.h"
#include "../core.h"

BT_DEF_MODULE_NAME			("Generic SDHCI Controller Driver")
BT_DEF_MODULE_DESCRIPTION	("Implements a driver for SDCARD abstraction for SDHCI controllers")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	const BT_INTEGRATED_DEVICE *pDevice;
	SDHCI_REGS 		   		   *pRegs;
	BT_MMC_CARD_EVENTRECEIVER 	pfnEventReceiver;
	MMC_HOST				   *pHost;
};

static const BT_IF_HANDLE oHandleInterface;

static BT_ERROR sdhci_irq_handler(BT_u32 ulIRQ, void *pParam) {

	BT_HANDLE hSDHCI = (BT_HANDLE) pParam;
	if(!hSDHCI) {
		return -1;
	}

	if(hSDHCI->pRegs->NORMAL_ERROR_INTSTAT & NORMAL_INT_CARD_INSERTED) {
		// Signal to SDCARD driver that we have inserted a card,
		// and the card can be initialised.
		hSDHCI->pRegs->NORMAL_ERROR_INTSTAT |= NORMAL_INT_CARD_INSERTED;
	}

	if(hSDHCI->pRegs->NORMAL_ERROR_INTSTAT & NORMAL_INT_CARD_REMOVED) {
		// Signal to SDCARD driver that we have removed a card.
		// and all data can be flagged for cleanup.
		hSDHCI->pRegs->NORMAL_ERROR_INTSTAT |= NORMAL_INT_CARD_REMOVED;
	}

	return BT_ERR_NONE;
}

static BT_ERROR sdhci_cleanup(BT_HANDLE hSDIO) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSDIO->pDevice, BT_RESOURCE_IRQ, 0);
	if(pResource) {
		BT_DisableInterrupt(pResource->ulStart);
		BT_UnregisterInterrupt(pResource->ulStart, sdhci_irq_handler, hSDIO);
	}

	BT_kFree(hSDIO);

	return BT_ERR_NONE;
}

static BT_ERROR sdhci_request(BT_HANDLE hSDIO, MMC_COMMAND *pCommand) {

	return BT_ERR_NONE;
}


static BT_ERROR sdhci_event_subscribe(BT_HANDLE hSDIO, BT_MMC_CARD_EVENTRECEIVER pfnReceiver, MMC_HOST *pHost) {

	hSDIO->pfnEventReceiver = pfnReceiver;
	hSDIO->pHost 			= pHost;

	// Test for card presence, and generate detection signal.

	if(hSDIO->pRegs->PRESENT_STATE & STATE_CARD_INSERTED) {
		if(hSDIO->pfnEventReceiver) {
			hSDIO->pfnEventReceiver(hSDIO->pHost, BT_MMC_CARD_DETECTED);
		}
	}

	return BT_ERR_NONE;
}

static const BT_MMC_OPS sdhci_mmc_ops = {
	.ulCapabilites1 	= 0,
	.pfnRequest			= sdhci_request,
	.pfnEventSubscribe 	= sdhci_event_subscribe,
};

static BT_HANDLE sdhci_probe(const BT_INTEGRATED_DEVICE *pDevice, BT_ERROR *pError) {

	BT_ERROR Error;

	BT_HANDLE hSDIO = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hSDIO) {
		goto err_out;
	}

	hSDIO->pDevice = pDevice;	// Provide access to parameters on cleanup.

	const BT_RESOURCE *pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_MEM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hSDIO->pRegs = (SDHCI_REGS *) pResource->ulStart;

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	BT_u32 ulIRQ = pResource->ulStart;

	Error = BT_RegisterInterrupt(ulIRQ, sdhci_irq_handler, hSDIO);
	if(Error) {
		goto err_free_out;
	}

	// This enables the interrupt at with the interrupt controller.
	Error = BT_EnableInterrupt(ulIRQ);
	if(Error) {
		goto err_free_irq;
	}

	// Enable the Card detection IRQs.
	hSDIO->pRegs->NORMAL_ERROR_INTSTAT_EN 	|= NORMAL_INT_CARD_INSERTED | NORMAL_INT_CARD_REMOVED;

	// Ensure we don't have any card-detection interrupts on init,
	// For some reason this can send the interrupt controller wild, even though
	// we have already registered our IRQ!

	hSDIO->pRegs->NORMAL_ERROR_INTSTAT 		|= NORMAL_INT_CARD_INSERTED | NORMAL_INT_CARD_REMOVED;

	// Enable the interrupts to be signalled to the CPU.
	hSDIO->pRegs->NORMAL_ERROR_INTSIG_EN 	|= NORMAL_INT_CARD_INSERTED | NORMAL_INT_CARD_REMOVED;

	// Register with the SD Host controller.
	Error = BT_RegisterSDHostController(hSDIO, &sdhci_mmc_ops);
	if(Error) {
		goto err_free_irq;
	}

	return hSDIO;

err_free_irq:
	BT_UnregisterInterrupt(ulIRQ, sdhci_irq_handler, hSDIO);

err_free_out:
	BT_kFree(hSDIO);

	if(pError) {
		*pError = Error;
	}

err_out:
	return NULL;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = sdhci_cleanup,
	/*.oIfs = {
		.pDevIF = &oDeviceInterface,
		},*/
};

BT_INTEGRATED_DRIVER_DEF sdhci_driver = {
	.name		= "mmc,sdhci",
	.pfnProbe	= sdhci_probe,
};
