/**
 *	SDCard Protocol Implementation.
 *
 *	This driver provides a block device wrapper for sd host I/O devices.
 *
 **/

#include <bitthunder.h>
#include <interrupts/bt_tasklets.h>
#include "core.h"

BT_DEF_MODULE_NAME			("BitThunder SDCARD Manager")
BT_DEF_MODULE_DESCRIPTION	("SDCARD abstraction layer for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

typedef struct _MMC_HOST {
	BT_LIST_ITEM		oItem;
	BT_HANDLE 			hHost;
	const BT_MMC_OPS   *pOps;
	BT_u32 				ulFlags;
#define MMC_HOST_FLAGS_INITIALISE_REQUEST 	0x00000001		///< This host has generated an initialisation request.
#define MMC_HOST_FLAGS_INVALIDATE			0x00000002		///< State-machine should attempt to invalidate resources dependent on this host.
} MMC_HOST;

static BT_TASKLET 	sm_tasklet;

static BT_LIST 		g_oSDHosts 	= {0};

static void card_event_handler(MMC_HOST *pHost, BT_MMC_CARD_EVENT eEvent, BT_BOOL bInterruptContext) {
	switch(eEvent) {

	case BT_MMC_CARD_DETECTED:
		// Signal SD-Card manager to attempt to initialise this host.
		pHost->ulFlags |= MMC_HOST_FLAGS_INITIALISE_REQUEST;
		break;

	case BT_MMC_CARD_REMOVED:
		// Signal SD-Card manager to invalidate all handles dependent on this host.
		pHost->ulFlags |= MMC_HOST_FLAGS_INVALIDATE;
		break;

	default:
		break;
	}

	BT_TaskletSchedule(&sm_tasklet);
}

BT_ERROR BT_RegisterSDHostController(BT_HANDLE hHost, const BT_MMC_OPS *pOps) {

	MMC_HOST *pHost = (MMC_HOST *) BT_kMalloc(sizeof(MMC_HOST));
	if(!pHost) {
		return BT_ERR_GENERIC;
	}

	pHost->hHost = hHost;
	pHost->pOps = pOps;

	if(pHost->pOps->pfnEventSubscribe) {
		pHost->pOps->pfnEventSubscribe(hHost, card_event_handler, pHost);
	}

	BT_ListAddItem(&g_oSDHosts, &pHost->oItem);

	return BT_ERR_NONE;
}


static void sd_manager_sm(void *pData) {

	BT_ERROR Error;

	MMC_HOST *pHost = (MMC_HOST *) BT_ListGetHead(&g_oSDHosts);
	while(pHost) {

		if(pHost->ulFlags & MMC_HOST_FLAGS_INITIALISE_REQUEST) {
			pHost->ulFlags &= ~MMC_HOST_FLAGS_INITIALISE_REQUEST;

			BT_ThreadSleep(100);

			if(pHost->pOps->pfnIsCardPresent(pHost->hHost, &Error)) {

				// Attempt basic SDIO initialisation (host-specific).
				pHost->pOps->pfnInitialise(pHost->hHost);

				// Send GO-IDLE (CMD0), expecting no response!
				MMC_COMMAND oCommand;
				oCommand.arg 			= 0;
				oCommand.opcode 		= 0;
				oCommand.bCRC			= BT_FALSE;
				oCommand.ulResponseType = 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				// Send CMD8 (SEND_IF_COND -- Helps determine SDHC support).
				oCommand.arg 			= 0x000001AA;		//	0xAA is the test field, it can be anything, 0x100 is the voltage range, 2.7-3.6V).
				oCommand.opcode 		= 8;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = 48;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				if((oCommand.response[0] & 0xFFF) != 0x1AA) {
					oCommand.bCRC = BT_TRUE;
				}

				while(1) {	/* Loop while the SD-Card initialised itself. */

					oCommand.opcode = 55;				// Send CMD55 to tell card we are doing an Application Command.
					oCommand.arg 	= 0;
					oCommand.bCRC 	= BT_TRUE;
					oCommand.ulResponseType = 48;

					pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

					oCommand.opcode = 41;				// Send ACMD41
					oCommand.arg 	= 0x40FF8000;		// Tell card  High Capacity mode is supported, and specify valid voltage windows.
					oCommand.bCRC	= BT_FALSE;
					oCommand.ulResponseType = 48;

					pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

					// Check if the card is ready?
					BT_u32 card_ready = oCommand.response[0] >> 31;
					if(!card_ready) {
						continue;
					}
					break;
				}

				// Read the CID register
				oCommand.arg 			= 0;
				oCommand.opcode 		= 2;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = 136;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				// We can use the information in the CID register to get things like the CARD S/N etc and manufacturer code.


				// Place the command into the data stat (CMD3).
				oCommand.arg = 0;
				oCommand.opcode = 3;
				oCommand.bCRC = BT_TRUE;
				oCommand.ulResponseType = 48;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				// Card initialised -- regster block device driver :)

				//BT_RegisterBlockDevice();

				BT_GpioSet(7, BT_TRUE);
			}
		}

		if(pHost->ulFlags & MMC_HOST_FLAGS_INVALIDATE) {
			pHost->ulFlags &= ~MMC_HOST_FLAGS_INVALIDATE;
			if(!pHost->pOps->pfnIsCardPresent(pHost->hHost, &Error)) {
				BT_GpioSet(7, BT_FALSE);
			}
		}

		pHost = (MMC_HOST *) BT_ListGetNext(&pHost->oItem);
	}
}

static BT_u32 sdcard_blockread(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer, BT_ERROR *pError) {

	return 0;
}

static BT_u32 sdcard_blockwrite(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer, BT_ERROR *pError) {

	return 0;
}

static const BT_IF_BLOCK oBlockInterface = {
		sdcard_blockread,
	sdcard_blockwrite,
};

static BT_ERROR sdcard_blockdev_cleanup(BT_HANDLE hHandle) {
	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = sdcard_blockdev_cleanup,
	.eType = BT_HANDLE_T_BLOCK,
	.oIfs = {
		.pBlockIF = &oBlockInterface,
	},
};

static BT_TASKLET sm_tasklet = {NULL, BT_TASKLET_IDLE, sd_manager_sm, NULL};

static BT_ERROR bt_sdcard_manager_init() {

	BT_ListInit(&g_oSDHosts);

	return BT_ERR_NONE;
}



BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_sdcard_manager_init,
};
