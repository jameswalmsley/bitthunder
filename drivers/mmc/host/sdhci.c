/**
 *	SD Host Controller Interface Driver Implementation for BitThunder.
 *
 *	This allows commands to be sent and received, as well as large data blocks
 *	to be transferred.
 *
 *	This is a generic layer, on-top of which the SD-Card driver sits, but
 *	its possible for a Bluetooth/Wifi/GPS drivers to sit above this, to
 *	provide support for such cards.
 **/

#include <bitthunder.h>
#include "sdhci.h"
#include "../core.h"
#include "../host.h"

BT_DEF_MODULE_NAME				("Generic SDHCI Controller Driver")
BT_DEF_MODULE_DESCRIPTION		("Implements a driver for SDCARD abstraction for SDHCI controllers")
BT_DEF_MODULE_AUTHOR			("James Walmsley")
BT_DEF_MODULE_EMAIL				("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 			h;
	const BT_INTEGRATED_DEVICE *pDevice;
	volatile SDHCI_REGS 	   *pRegs;
	BT_MMC_CARD_EVENTRECEIVER 	pfnEventReceiver;
	MMC_HOST				   *pHost;
	BT_MMC_HOST_OPS			   *pHostOps;
	BT_u32						ulFlags;
	BT_u32						ulIRQ;
};

static const BT_IF_HANDLE oHandleInterface;

static BT_ERROR sdhci_irq_handler(BT_u32 ulIRQ, void *pParam) {

	BT_HANDLE hSDHCI = (BT_HANDLE) pParam;
	if(!hSDHCI) {
		return -1;
	}

	if(hSDHCI->pRegs->NORMAL_INT_STATUS & NORMAL_INT_CARD_INSERTED) {
		// Signal to SDCARD driver that we have inserted a card,
		// and the card can be initialised.
		hSDHCI->pRegs->NORMAL_INT_STATUS = NORMAL_INT_CARD_INSERTED;

		if(hSDHCI->pRegs->PRESENT_STATE & STATE_CARD_INSERTED) {

			// Enable the SDHCI clock.
			hSDHCI->pRegs->CLOCK_CONTROL |=  CLOCK_INTERNAL_ENABLE;
			//
			// On insertion, the SD controllers clock is enabled!
			//
			// This does not guarantee that the clock is stable, this must be determined before
			// card-initialisation begins.
			//

			if(hSDHCI->pfnEventReceiver) {
				hSDHCI->pfnEventReceiver(hSDHCI->pHost, BT_MMC_CARD_DETECTED, BT_TRUE);
			}
		}
	}

	if(hSDHCI->pRegs->NORMAL_INT_STATUS & NORMAL_INT_CARD_REMOVED) {
		// Signal to SDCARD driver that we have removed a card.
		// and all data can be flagged for cleanup.
		hSDHCI->pRegs->NORMAL_INT_STATUS = NORMAL_INT_CARD_REMOVED;

		if(!(hSDHCI->pRegs->PRESENT_STATE & STATE_CARD_INSERTED)) {

			// Disable the SDHCI clock.
			hSDHCI->pRegs->CLOCK_CONTROL &= ~CLOCK_INTERNAL_ENABLE;

			if(hSDHCI->pfnEventReceiver) {
				hSDHCI->pfnEventReceiver(hSDHCI->pHost, BT_MMC_CARD_REMOVED, BT_TRUE);
			}
		}
	}

	return BT_ERR_NONE;
}

static BT_ERROR sdhci_cleanup(BT_HANDLE hSDIO) {
	const BT_RESOURCE *pResource = BT_GetIntegratedResource(hSDIO->pDevice, BT_RESOURCE_IRQ, 0);
	if(pResource) {
		BT_DisableInterrupt(pResource->ulStart);
		BT_UnregisterInterrupt(pResource->ulStart, sdhci_irq_handler, hSDIO);
	}

	// Dont't forget the handle will itself be cleanup up auto-magically!

	return BT_ERR_NONE;
}

static BT_ERROR sdhci_enable_clock(BT_HANDLE hSDIO) {
	hSDIO->pRegs->CLOCK_CONTROL |= 4;
	return BT_ERR_NONE;
}

static BT_ERROR sdhci_disable_clock(BT_HANDLE hSDIO) {
	hSDIO->pRegs->CLOCK_CONTROL &= ~4;
	return BT_ERR_NONE;
}

static BT_ERROR sdhci_request(BT_HANDLE hSDIO, MMC_COMMAND *pCommand) {

	// Wait until the the command is not inhibited.
	while(hSDIO->pRegs->PRESENT_STATE & (STATE_COMMAND_INHIBIT_CMD | STATE_COMMAND_INHIBIT_DAT)) {
		BT_ThreadYield();
	}

	if(pCommand->bIsData) {
		hSDIO->pRegs->BLOCK_SIZE = 512;
		hSDIO->pRegs->BLOCK_COUNT = pCommand->ulBlocks;
	}

	hSDIO->pRegs->NORMAL_INT_STATUS = 0xFFFF;
	hSDIO->pRegs->ERROR_INT_STATUS = 0xFFFF;

	hSDIO->pRegs->ARGUMENT = pCommand->arg;

	BT_u16 cmd_reg = pCommand->opcode << 8;
	if(pCommand->bCRC) {
		cmd_reg |= COMMAND_CRC_ENABLE;
	}

	if(pCommand->bIsData) {
		cmd_reg |= COMMAND_DATA_PRESENT;
		BT_u32 tm = 0;
		//DIO->pRegs->TRANSFERMODE = 0;
		if(pCommand->bRead_nWrite) {
			tm = 1 << 4;
		}

		tm |= 1 << 5;
		tm |= 1 << 2;
		tm |= 1 << 1;

		hSDIO->pRegs->TRANSFERMODE = tm;

	} else {
		hSDIO->pRegs->TRANSFERMODE = 0;
	}

	switch(pCommand->ulResponseType) {
	case 48: {
		COMMAND_RESPONSE_SELECT_SET(cmd_reg, 2);
		break;
	}

	case 136: {
		COMMAND_RESPONSE_SELECT_SET(cmd_reg, 1);
		break;
	}

	default:
		COMMAND_RESPONSE_SELECT_SET(cmd_reg, 0);
		break;
	}

	hSDIO->pRegs->COMMAND = cmd_reg;

	// Wait until the command is complete.
	while(!(hSDIO->pRegs->NORMAL_INT_STATUS & NORMAL_INT_COMMAND_COMPLETE)) {
		BT_ThreadYield();
	}

	// Clear the command complete interrupt status field.
	hSDIO->pRegs->NORMAL_INT_STATUS = NORMAL_INT_COMMAND_COMPLETE;

	pCommand->response[0] = hSDIO->pRegs->RESPONSE[0] | (hSDIO->pRegs->RESPONSE[1] << 16);
	pCommand->response[1] = hSDIO->pRegs->RESPONSE[2] | (hSDIO->pRegs->RESPONSE[3] << 16);
	pCommand->response[2] = hSDIO->pRegs->RESPONSE[4] | (hSDIO->pRegs->RESPONSE[5] << 16);
	pCommand->response[3] = hSDIO->pRegs->RESPONSE[6] | (hSDIO->pRegs->RESPONSE[7] << 16);

	return BT_ERR_NONE;
}

static BT_u32 sdhci_read(BT_HANDLE hSDIO, BT_u32 ulBlocks, void *pBuffer, BT_ERROR *pError) {
	register BT_u8 *p = (BT_u8 *) pBuffer;

	//BT_kPrint("SDHCI: Awaiting buffer read ready interrupt:");



	//BT_kPrint("SDHCI: Buffer is now ready...");

	// Clear the buffer ready interrupt.
	BT_u32 ulRead = 0;

	while(ulRead < ulBlocks) {
		BT_u32 ulSize = 512;

		while(!(hSDIO->pRegs->NORMAL_INT_STATUS & NORMAL_INT_BUF_READ_READY)) {
			BT_ThreadYield();
		}

		hSDIO->pRegs->NORMAL_INT_STATUS = NORMAL_INT_BUF_READ_READY;

		while(ulSize) {
			BT_u32 ulData = hSDIO->pRegs->BUFFER_DATA_PORT;
			BT_u8 d0 = (BT_u8) (ulData & 0xff);
			BT_u8 d1 = (BT_u8) ((ulData >> 8) & 0xff);
			BT_u8 d2 = (BT_u8) ((ulData >> 16) & 0xff);
			BT_u8 d3 = (BT_u8) ((ulData >> 24) & 0xff);

			*p++ = d0;
			*p++ = d1;
			*p++ = d2;
			*p++ = d3;

			ulSize -= 4;
		}

		ulRead++;
	}

	while(! (hSDIO->pRegs->NORMAL_INT_STATUS & NORMAL_INT_TRANSFER_COMPLETE)) {
		//volatile BT_u32 ulData2 = (volatile) hSDIO->pRegs->BUFFER_DATA_PORT;
		BT_ThreadYield();
	}

	hSDIO->pRegs->NORMAL_INT_STATUS = NORMAL_INT_TRANSFER_COMPLETE;

	//BT_kPrint("SDHCI: Block transfer complete");

	/*hSDIO->pRegs->SOFTWARE_RESET = RESET_DATA;

	while(hSDIO->pRegs->SOFTWARE_RESET) {
		BT_ThreadYield();
	}*/

	return ulRead;
}

static BT_u32 sdhci_write(BT_HANDLE hSDIO, BT_u32 ulSize, void *pBuffer, BT_ERROR *pError) {

	return 0;
}

static BT_ERROR sdhci_event_subscribe(BT_HANDLE hSDIO, BT_MMC_CARD_EVENTRECEIVER pfnReceiver, MMC_HOST *pHost) {

	hSDIO->pfnEventReceiver = pfnReceiver;
	hSDIO->pHost 			= pHost;

	// Test for card presence, and generate detection signal.

	if(hSDIO->pRegs->PRESENT_STATE & STATE_CARD_INSERTED) {
		if(hSDIO->pfnEventReceiver) {
			hSDIO->pfnEventReceiver(hSDIO->pHost, BT_MMC_CARD_DETECTED, BT_FALSE);
		}
	}

	return BT_ERR_NONE;
}

static BT_BOOL sdhci_is_card_present(BT_HANDLE hSDIO, BT_ERROR *pError) {
	return ((hSDIO->pRegs->PRESENT_STATE & STATE_CARD_INSERTED) || (hSDIO->ulFlags & SDHCI_FLAGS_ALWAYS_PRESENT));
}

static BT_ERROR sdhci_reset(BT_HANDLE hSDIO, BT_u8 ucMask) {
	//hSDIO->pRegs->SOFTWARE_RESET = ucMask;

	hSDIO->pRegs->POWER_CONTROL = 0;		// Turn Card power off
	hSDIO->pRegs->CLOCK_CONTROL = 0;

	hSDIO->pRegs->NORMAL_INT_ENABLE = 0;	// Disable Interrupts.
	hSDIO->pRegs->ERROR_INT_ENABLE = 0;

	hSDIO->pRegs->SOFTWARE_RESET = ucMask;
	// Wait for reset to complete

	while(hSDIO->pRegs->SOFTWARE_RESET) {
		BT_ThreadYield();
	}

	if(hSDIO->pRegs->ERROR_INT_STATUS & 0xF) {
		hSDIO->pRegs->SOFTWARE_RESET = 2;
		while(hSDIO->pRegs->SOFTWARE_RESET) {
			BT_ThreadYield();
		}
	}

	if(hSDIO->pRegs->ERROR_INT_STATUS & 0x3) {
		hSDIO->pRegs->SOFTWARE_RESET = 4;
		while(hSDIO->pRegs->SOFTWARE_RESET) {
			BT_ThreadYield();
		}
	}

	hSDIO->pRegs->AUTO_CMD12_ERROR = 0;	// Clear Auto CMD12 status.

	// Enable the Card detection IRQs.
	/*hSDIO->pRegs->NORMAL_INT_ENABLE 		|=	NORMAL_INT_CARD_INSERTED
												| 	NORMAL_INT_CARD_REMOVED
												| 	NORMAL_INT_COMMAND_COMPLETE;*/

	hSDIO->pRegs->NORMAL_INT_ENABLE = 0xFFFF;
	hSDIO->pRegs->ERROR_INT_ENABLE = 0xFFFF;


	// Ensure we don't have any card-detection interrupts on init,
	// For some reason this can send the interrupt controller wild, even though
	// we have already registered our IRQ!

	hSDIO->pRegs->NORMAL_INT_STATUS		= 0xFFFF;
	__asm volatile("dsb");

	// Enable the interrupts to be signalled to the CPU.
	hSDIO->pRegs->NORMAL_INT_SIGNAL_ENABLE 	|= NORMAL_INT_CARD_INSERTED | NORMAL_INT_CARD_REMOVED;
	hSDIO->pRegs->NORMAL_INT_STATUS		= 0xFFFF;

	return BT_ERR_NONE;
}

static BT_ERROR sdhci_set_data_width(BT_HANDLE hSDIO, BT_MMC_WIDTH eWidth);

static BT_ERROR sdhci_initialise(BT_HANDLE hSDIO) {

	hSDIO->pRegs->POWER_CONTROL = 0;
	hSDIO->pRegs->NORMAL_INT_SIGNAL_ENABLE = 0;
	hSDIO->pRegs->ERROR_INT_SIGNAL_ENABLE = 0;

	// Completely reset the SDIO hardware.
	sdhci_disable_clock(hSDIO);	// Enable the clock to allow the SDCard to initialise.
	sdhci_reset(hSDIO, RESET_ALL);

	hSDIO->pRegs->POWER_CONTROL = POWER_ENABLE | POWER_SELECT_3_3V;

	sdhci_set_data_width(hSDIO, BT_MMC_WIDTH_1BIT);

	if(!sdhci_is_card_present(hSDIO, NULL)) {
		return BT_ERR_GENERIC;
	}

	// Enable the SDIO clock, and attempt to reset the card if present.
	BT_u32 reg = hSDIO->pRegs->CLOCK_CONTROL;
	reg &= 0x00FF;	// Mask out the clock selection freq!
	reg |= (1 << 8);	// Set SDClk to be base (50mhz / 128 ~400khz).
	reg |= 1;			// Enable internal SDHCI clock.
	hSDIO->pRegs->CLOCK_CONTROL = reg;

	while(!(hSDIO->pRegs->CLOCK_CONTROL & 2)) {	// Spin until the clock is stable.
		BT_ThreadYield();
	}

	hSDIO->pRegs->TIMEOUT_CONTROL = 0x7;	// Timeout at TMCLK 2 ^ 20;

	BT_u32 caps = hSDIO->pRegs->CAPABILITIES;
	if(caps & CAPS_1_8V) {
		POWER_SELECT_SET(hSDIO->pRegs->POWER_CONTROL, POWER_SELECT_1_8V);
	}

	if(caps & CAPS_3_0V) {
		POWER_SELECT_SET(hSDIO->pRegs->POWER_CONTROL, POWER_SELECT_3_0V);
	}

	if(caps & CAPS_3_3V) {
		POWER_SELECT_SET(hSDIO->pRegs->POWER_CONTROL, POWER_SELECT_3_3V);
	}

	// Enable bus power!
	POWER_ENABLE_SET(hSDIO->pRegs->POWER_CONTROL, 1);

	//sdhci_reset(hSDIO, RESET_CMD);
	//sdhci_reset(hSDIO, RESET_DATA);


	// Enable the Card detection IRQs.
	hSDIO->pRegs->NORMAL_INT_ENABLE 		|=	NORMAL_INT_CARD_INSERTED
											| 	NORMAL_INT_CARD_REMOVED
											| 	NORMAL_INT_COMMAND_COMPLETE
		                                    |	NORMAL_INT_BUF_READ_READY
		                                    | 	NORMAL_INT_TRANSFER_COMPLETE;

	// Ensure we don't have any card-detection interrupts on init,
	// For some reason this can send the interrupt controller wild, even though
	// we have already registered our IRQ!

	// Enable the interrupts to be signalled to the CPU.
	hSDIO->pRegs->NORMAL_INT_STATUS		= 0xFFFF;

	hSDIO->pRegs->NORMAL_INT_SIGNAL_ENABLE 	|= NORMAL_INT_CARD_INSERTED | NORMAL_INT_CARD_REMOVED;

	BT_EnableInterrupt(hSDIO->ulIRQ);

	return BT_ERR_NONE;
}

static BT_ERROR sdhci_set_interrupt_mask(BT_HANDLE hSDIO, BT_MMC_INTERRUPTS eInterrupt, BT_BOOL bEnable) {

	switch(eInterrupt) {
	case BT_MMC_INTERRUPT_CARD:
		if(bEnable) {
			hSDIO->pRegs->NORMAL_INT_ENABLE |= NORMAL_INT_CARD;
		} else {
			hSDIO->pRegs->NORMAL_INT_ENABLE &= ~NORMAL_INT_CARD;
		}
		break;
	}

	return BT_ERR_NONE;
}

static BT_ERROR sdhci_set_data_width(BT_HANDLE hSDIO, BT_MMC_WIDTH eWidth) {
	switch(eWidth) {
	case BT_MMC_WIDTH_1BIT:
		hSDIO->pRegs->HOST_CONTROL &= ~HOST_TRANSFER_WIDTH;
		break;

	case BT_MMC_WIDTH_4BIT:
		hSDIO->pRegs->HOST_CONTROL |= HOST_TRANSFER_WIDTH;
		break;

	default:
		break;
	}
	return BT_ERR_NONE;
}

static BT_ERROR sdhci_set_block_size(BT_HANDLE hSDIO, BT_u32 ulBlockSize) {
	hSDIO->pRegs->BLOCK_SIZE = ulBlockSize;
	return BT_ERR_NONE;
}

static const BT_MMC_OPS sdhci_mmc_ops = {
	.ulCapabilites1 	= 0,
	.pfnRequest			= sdhci_request,
	.pfnRead			= sdhci_read,
	.pfnWrite			= sdhci_write,
	.pfnEventSubscribe 	= sdhci_event_subscribe,
	.pfnIsCardPresent	= sdhci_is_card_present,
	.pfnSetInterruptMask = sdhci_set_interrupt_mask,
	.pfnSetDataWidth    = sdhci_set_data_width,
	.pfnSetBlockSize    = sdhci_set_block_size,
	.pfnInitialise 		= sdhci_initialise,
	.pfnEnableClock		= sdhci_enable_clock,
	.pfnDisableClock	= sdhci_disable_clock,
};

#ifdef BT_CONFIG_SDHCI_DUMP_REGS
static void sdhci_dump_regs(BT_HANDLE hSDIO) {
	BT_kPrint("SDMA_ADDRESS  : %08X", hSDIO->pRegs->SDMA_Address);
	BT_kPrint("BLOCK_SIZE    : %08X", hSDIO->pRegs->BLOCK_SIZE);
	BT_kPrint("BLOCK_COUNT   : %08X", hSDIO->pRegs->BLOCK_COUNT);
	BT_kPrint("ARGUMENT      : %08X", hSDIO->pRegs->ARGUMENT);
	BT_kPrint("TRANSFERMODE  : %08X", hSDIO->pRegs->COMMAND);
	BT_kPrint("RESPONSE_0    : %08X", hSDIO->pRegs->RESPONSE[0]);
	BT_kPrint("RESPONSE_1    : %08X", hSDIO->pRegs->RESPONSE[1]);
	BT_kPrint("RESPONSE_2    : %08X", hSDIO->pRegs->RESPONSE[2]);
	BT_kPrint("RESPONSE_3    : %08X", hSDIO->pRegs->RESPONSE[3]);
	BT_kPrint("BUFFER_DATA   : %08X", hSDIO->pRegs->BUFFER_DATA_PORT);
	BT_kPrint("PRESENT_STATE : %08X", hSDIO->pRegs->PRESENT_STATE);
	BT_kPrint("HOST_CONTROL  : %08X", hSDIO->pRegs->HOST_CONTROL);
	BT_kPrint("POWER_CONTROL : %08X", hSDIO->pRegs->POWER_CONTROL);
	BT_kPrint("BLOCK_GAP_CTL : %08X", hSDIO->pRegs->BLOCK_GAP_CONTROL);
	BT_kPrint("WAKEUP_CTL    : %08X", hSDIO->pRegs->WAKEUP_CONTROL);
	BT_kPrint("CLOCK_CONTROL : %08X", hSDIO->pRegs->CLOCK_CONTROL);
	BT_kPrint("TIMEOUT_CTL   : %08X", hSDIO->pRegs->TIMEOUT_CONTROL);
	BT_kPrint("SOFTWARE_RST  : %08X", hSDIO->pRegs->SOFTWARE_RESET);
	BT_kPrint("NORMAL_INT_ST : %08X", hSDIO->pRegs->NORMAL_INT_STATUS);
	BT_kPrint("ERROR_INT_ST  : %08X", hSDIO->pRegs->ERROR_INT_STATUS);
	BT_kPrint("NORMAL_INT_EN : %08X", hSDIO->pRegs->NORMAL_INT_ENABLE);
	BT_kPrint("ERROR_INT_EN  : %08X", hSDIO->pRegs->ERROR_INT_ENABLE);
	BT_kPrint("NORMAL_INT_SIG: %08X", hSDIO->pRegs->NORMAL_INT_SIGNAL_ENABLE);
	BT_kPrint("ERROR_INT_SIG : %08X", hSDIO->pRegs->ERROR_INT_SIGNAL_ENABLE);
	BT_kPrint("AUTO12_ERROR  : %08X", hSDIO->pRegs->AUTO_CMD12_ERROR);
	BT_kPrint("CAPABILITIES  : %08X", hSDIO->pRegs->CAPABILITIES);
	BT_kPrint("MAX_CURRENT_C : %08X", hSDIO->pRegs->MAX_CURRENT_CAPS);
	BT_kPrint("FORCE_EVT_C12 : %08X", hSDIO->pRegs->FORCE_EVENT_AUTO_CMD12);
	BT_kPrint("ADMA_ERROR_ST : %08X", hSDIO->pRegs->ADMA_ERROR_STATUS);
	BT_kPrint("ADMA_SYS_ADDR : %08X", hSDIO->pRegs->ADMA_SYS_ADDRESS);
	BT_kPrint("VERSION       : %08X", hSDIO->pRegs->SLOT_INT_STAT_HCVERSION);
}
#endif

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

#ifdef BT_CONFIG_SDHCI_DUMP_REGS
	sdhci_dump_regs(hSDIO);
#endif

	sdhci_initialise(hSDIO);
	//sdhci_reset(hSDIO, RESET_ALL);

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_IRQ, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_out;
	}

	hSDIO->ulIRQ = pResource->ulStart;

	Error = BT_RegisterInterrupt(hSDIO->ulIRQ, sdhci_irq_handler, hSDIO);
	if(Error) {
		goto err_free_out;
	}

	// This enables the interrupt at with the interrupt controller.
	/*Error = BT_EnableInterrupt(ulIRQ);
	if(Error) {
		goto err_free_irq;
		}*/

	BT_GpioSetDirection(0, BT_GPIO_DIR_OUTPUT);
	BT_GpioSet(0, 0);


	sdhci_set_data_width(hSDIO, BT_MMC_WIDTH_1BIT);

	sdhci_reset(hSDIO, RESET_ALL);

	//BT_u32 hc_version 	= hSDIO->pRegs->SLOT_INT_STAT_HCVERSION;
	//BT_u32 vendor 		= HCVERSION_VENDOR_GET(hc_version);
	//BT_u32 sdversion 	= HCVERSION_SPECV_GET(hc_version);

	// We could check controller version to ensure compatibility, but
	// in all likely-hood (clutching at straws) newer versions would remain mostly
	// compatible.

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_PARAM, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_irq;
	}

	hSDIO->pHostOps = (BT_MMC_HOST_OPS *) pResource->pParam;

	// Register with the SD Host controller.
	Error = BT_RegisterSDHostController(hSDIO, &sdhci_mmc_ops);
	if(Error) {
		goto err_free_irq;
	}

	pResource = BT_GetIntegratedResource(pDevice, BT_RESOURCE_FLAGS, 0);
	if(!pResource) {
		Error = BT_ERR_GENERIC;
		goto err_free_irq;
	}

	hSDIO->ulFlags = pResource->ulConfigFlags;

	//if(hSDIO->ulFlags & SDHCI_FLAGS_ALWAYS_PRESENT) {
	hSDIO->pfnEventReceiver(hSDIO->pHost, BT_MMC_CARD_DETECTED, BT_TRUE);
	//}

	return hSDIO;

err_free_irq:
	BT_UnregisterInterrupt(hSDIO->ulIRQ, sdhci_irq_handler, hSDIO);

err_free_out:
	BT_DestroyHandle(hSDIO);

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
