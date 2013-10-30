/**
 *	SDCard Protocol Implementation.
 *
 *	This driver provides a block device wrapper for sd host I/O devices.
 *
 **/

#include <bitthunder.h>
#include <interrupts/bt_tasklets.h>
#include <stdio.h>
#include "core.h"

BT_DEF_MODULE_NAME			("SD/MMC Manager")
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
	BT_u32				ulHostID;
	BT_u16 				rca;
	BT_HANDLE			hBlock;								///< Handle to an instantiated block device for this host.
	BT_BOOL				bSDHC;
} MMC_HOST;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	MMC_HOST 		   *pHost;
	BT_BLKDEV_DESCRIPTOR	oDescriptor;
};

static const BT_IF_HANDLE oHandleInterface;

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

	pHost->ulHostID = g_oSDHosts.ulItems;
	BT_ListAddItem(&g_oSDHosts, &pHost->oItem);

	return BT_ERR_NONE;
}


static void sd_manager_sm(void *pData) {

	BT_ERROR Error;

	MMC_HOST *pHost = (MMC_HOST *) BT_ListGetHead(&g_oSDHosts);
	while(pHost) {

		if(pHost->ulFlags & MMC_HOST_FLAGS_INITIALISE_REQUEST) {
			pHost->ulFlags &= ~MMC_HOST_FLAGS_INITIALISE_REQUEST;

			BT_ThreadSleep(10);

			if(pHost->pOps->pfnIsCardPresent(pHost->hHost, &Error)) {

				BT_kPrint("SDCARD: SDCard was inserted");

				// Attempt basic SDIO initialisation (host-specific).
				pHost->pOps->pfnInitialise(pHost->hHost);

				pHost->pOps->pfnEnableClock(pHost->hHost);

				BT_ThreadSleep(10);

				// Send GO-IDLE (CMD0), expecting no response!
				MMC_COMMAND oCommand;
				oCommand.arg 			= 0;
				oCommand.opcode 		= 0;
				oCommand.bCRC			= BT_FALSE;
				oCommand.ulResponseType = 0;
				oCommand.bIsData		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				BT_kPrint("SDCARD: Sent GO_IDLE");

				// Send CMD8 (SEND_IF_COND -- Helps determine SDHC support).
				oCommand.arg 			= 0x000001AA;		//	0xAA is the test field, it can be anything, 0x100 is the voltage range, 2.7-3.6V).
				oCommand.opcode 		= 8;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = 48;
				oCommand.bIsData		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				if((oCommand.response[0] & 0xFFF) != 0x1AA) {
					oCommand.bCRC = BT_TRUE;
				}

				BT_kPrint("SDCARD: Sent SEND_IF_COND, response = %08x", oCommand.response[0]);

				while(1) {	/* Loop while the SD-Card initialised itself. */

					oCommand.opcode = 55;				// Send CMD55 to tell card we are doing an Application Command.
					oCommand.arg 	= 0;
					oCommand.bCRC 	= BT_TRUE;
					oCommand.ulResponseType = 48;
					oCommand.bIsData		= 0;

					pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

					oCommand.opcode = 41;				// Send ACMD41
					oCommand.arg 	= 0x40FF8000;		// Tell card  High Capacity mode is supported, and specify valid voltage windows.
					oCommand.bCRC	= BT_FALSE;
					oCommand.ulResponseType = 48;
					oCommand.bIsData		= 0;

					pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

					// Check if the card is ready?
					BT_u32 card_ready = oCommand.response[0] >> 31;
					if(!card_ready) {
						continue;
					}
					break;
				}

				pHost->bSDHC = (oCommand.response[0] >> 31) & 1;

				if(pHost->bSDHC) {
					BT_kPrint("SDCARD: SDHC card support detected");
				} else {
					BT_kPrint("SDCARD: Non-SDHC card detected");
				}

				// Read the CID register
				oCommand.arg 			= 0;
				oCommand.opcode 		= 2;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = 136;
				oCommand.bIsData 		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				// We can use the information in the CID register to get things like the CARD S/N etc and manufacturer code.
				BT_kPrint("SDCARD: CID reg %08x:%08x:%08x:%08x", oCommand.response[3], oCommand.response[2], oCommand.response[1], oCommand.response[0]);

				// Place the command into the data stat (CMD3).
				oCommand.arg 			= 0;
				oCommand.opcode 		= 3;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = 48;
				oCommand.bIsData 		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				pHost->rca = oCommand.response[0] >> 16;

				BT_u32 crcError		 = (oCommand.response[0] >> 15) & 0x1;
				BT_u32 illegal_cmd	 = (oCommand.response[0] >> 14) & 0x1;
				BT_u32 error	  	 = (oCommand.response[0] >> 13) & 0x1;
				BT_u32 status	     = (oCommand.response[0] >> 9) & 0xf;
				BT_u32 ready		 = (oCommand.response[0] >> 8) & 0x1;

				if(crcError) {
					BT_kPrint("SDCARD: CRC Error");
				}

				if(illegal_cmd) {
					BT_kPrint("SDCARD: Illegal command");
				}

				if(error) {
					BT_kPrint("SDCARD: Generic Error");
				}

				if(!ready) {
					BT_kPrint("SDCARD: not ready in data state!");
				}

				BT_kPrint("SDCARD: Placed SDCARD into data state. (resp: %08x)", oCommand.response[0]);
				BT_kPrint("SDCARD: Relative Card Address (RCA): %04x", pHost->rca);

				// Get CSD

				oCommand.arg 			= pHost->rca << 16;
				oCommand.opcode 		= 9;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = 136;
				oCommand.bIsData 		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				BT_u32 ulBlocks = 0;
				BT_u32 csdversion = (oCommand.response[3] >> 22) & 0x3;

				if(csdversion != 1) {
					BT_kPrint("SDCARD: Unrecognised CSD register structure version.");
				} else {
					ulBlocks = ((oCommand.response[1] >> 8) & 0x3FFFFF) + 1;
					ulBlocks = ulBlocks * 1024;
				}

				oCommand.arg = pHost->rca << 16;
				oCommand.opcode = 7;
				oCommand.bCRC = BT_TRUE;
				oCommand.ulResponseType = 48;
				oCommand.bIsData		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				BT_kPrint("SDCARD: Selected card mmc%d:%04x", pHost->ulHostID, pHost->rca);


				BT_u32 cmd7_response = oCommand.response[0];
				status = (cmd7_response >> 9) & 0xf;

				if(status != 3 && status !=4) {
					BT_kPrint("SDCARD: invalid status %i", status);
				}

				if(!pHost->bSDHC) {
					oCommand.arg = 512;
					oCommand.opcode = 16;
					oCommand.bCRC = BT_TRUE;
					oCommand.ulResponseType = 48;
					oCommand.bIsData		= 0;

					pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
				}

				pHost->pOps->pfnSetBlockSize(pHost->hHost, 512);

				// Place SDCard into 4-bit mode. (ACMD6).

				oCommand.arg = pHost->rca << 16;
				oCommand.opcode = 55;
				oCommand.bCRC = BT_TRUE;
				oCommand.ulResponseType = 48;
				oCommand.bIsData		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				oCommand.arg = 0x2;	// Set argument to be 4-bit mode
				oCommand.opcode = 6;
				oCommand.bCRC = BT_TRUE;
				oCommand.ulResponseType = 48;
				oCommand.bIsData		= 0;

				pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

				pHost->pOps->pfnSetDataWidth(pHost->hHost, BT_MMC_WIDTH_4BIT);

				BT_kPrint("SDCARD: Configured card for 4-bit data width. (resp: %08x)", oCommand.response[0]);

				BT_kPrint("SDCARD: sucessfully initialised... registering block device");
				// Card initialised -- regster block device driver :)

				BT_HANDLE hSD = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
				if(!hSD) {

				}

				hSD->pHost = pHost;
				//hSD->oDescriptor.

				hSD->oDescriptor.oGeometry.ulBlockSize = 512;
				hSD->oDescriptor.oGeometry.ulTotalBlocks = ulBlocks;


				char buffer[10];
				sprintf(buffer, "mmc%d", (int) pHost->ulHostID);

				BT_RegisterBlockDevice(hSD, buffer, &hSD->oDescriptor);

				BT_GpioSet(7, BT_TRUE);
			}
		}

		if(pHost->ulFlags & MMC_HOST_FLAGS_INVALIDATE) {
			pHost->ulFlags &= ~MMC_HOST_FLAGS_INVALIDATE;
			if(!pHost->pOps->pfnIsCardPresent(pHost->hHost, &Error)) {
				BT_GpioSet(7, BT_FALSE);
				BT_kPrint("SDCARD: SDCard (mmc%d:%04x) was removed", pHost->ulHostID, pHost->rca);
			}
		}

		pHost = (MMC_HOST *) BT_ListGetNext(&pHost->oItem);
	}
}

static BT_u32 sdcard_blockread(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer, BT_ERROR *pError) {

	//BT_kPrint("SDCARD: BlockRead (%d[%d])", ulBlock, ulCount);

	if(!hBlock->pHost->rca) {
		// Card not initialised!
		return 0;
	}

	BT_u32 ulRead;
	BT_s32 nlRetryCount = 0;
	while(1)
	{
		MMC_COMMAND oCommand;
		oCommand.opcode 		= 13;
		oCommand.arg 			= hBlock->pHost->rca << 16;
		oCommand.bCRC 			= BT_TRUE;
		oCommand.ulResponseType = 48;
		oCommand.bIsData		= BT_FALSE;

		hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

		BT_u32 ulStatus = oCommand.response[0];
		BT_u32 ulState = (ulStatus >> 9) & 0xF;

		//BT_kPrint("SDCARD: Status (%02x)", ulState);

		switch(ulState) {

		case 4:
			break;

		case 5:
			oCommand.opcode = 12;
			oCommand.arg = 0;
			oCommand.bCRC = BT_TRUE;
			oCommand.ulResponseType = 48;
			oCommand.bIsData = BT_FALSE;

			hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

			//BT_kPrint("SDCARD: Sent CMD12");

			oCommand.opcode 		= 13;
			oCommand.arg 			= hBlock->pHost->rca << 16;
			oCommand.bCRC 			= BT_TRUE;
			oCommand.ulResponseType = 48;
			oCommand.bIsData		= BT_FALSE;

			hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

			BT_u32 ulStatus = oCommand.response[0];
			ulState = (ulStatus >> 9) & 0xF;

			break;


		default:
			BT_kPrint("SDCARD: Unknown card state %d", ulState);
			break;
		}

		if(!hBlock->pHost->bSDHC) {
			ulBlock *= 512;
		}

		oCommand.opcode 		= 18;
		oCommand.bCRC 			= BT_FALSE;
		oCommand.ulResponseType = 48;
		oCommand.bIsData 		= BT_TRUE;
		oCommand.arg 			= ulBlock;
		oCommand.bRead_nWrite	= BT_TRUE;
		oCommand.ulBlocks		= ulCount;

		hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

		BT_u32 cmd17_response = oCommand.response[0];
		if(cmd17_response != 0x900) {	// STATE = transfer, READY_FOR_DATA = set
			BT_kPrint("SDCARD: Invalid CMD18 response");
			return 0;
		}

		//BT_kPrint("SDCARD: Read command complete, waiting for data");

		// Read the data.

		ulRead = hBlock->pHost->pOps->pfnRead(hBlock->pHost->hHost, ulCount, pBuffer, pError);
		if(ulRead == ulCount) break;

		if(nlRetryCount++ >= 3) {
			BT_kPrint("SDCARD: read block (%d,%d) fatal error!", ulBlock, ulCount);
			break;
		} else {
			BT_kPrint("SDCARD: read block (%d,%d) error, retrying (%d) ... ", ulBlock, ulCount, nlRetryCount);
		}
	}

	return ulRead;
}

static BT_u32 sdcard_blockwrite(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer, BT_ERROR *pError) {

	//BT_kPrint("SDCARD: BlockWrite (%d,%d)", ulBlock, ulCount);

	if(!hBlock->pHost->rca) {
		// Card not initialised!
		return 0;
	}

	MMC_COMMAND oCommand;
	oCommand.opcode 		= 13;
	oCommand.arg 			= hBlock->pHost->rca << 16;
	oCommand.bCRC 			= BT_TRUE;
	oCommand.ulResponseType = 48;
	oCommand.bIsData		= BT_FALSE;

	hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

	BT_u32 ulStatus = oCommand.response[0];
	BT_u32 ulState = (ulStatus >> 9) & 0xF;

	//BT_kPrint("SDCARD: Status (%02x)", ulState);

	switch(ulState) {

	case 4:
		break;

	case 5:
		oCommand.opcode = 12;
		oCommand.arg = 0;
		oCommand.bCRC = BT_TRUE;
		oCommand.ulResponseType = 48;
		oCommand.bIsData = BT_FALSE;

		hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

		BT_kPrint("SDCARD: Sent CMD12");

		oCommand.opcode 		= 13;
		oCommand.arg 			= hBlock->pHost->rca << 16;
		oCommand.bCRC 			= BT_TRUE;
		oCommand.ulResponseType = 48;
		oCommand.bIsData		= BT_FALSE;

		hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

		BT_u32 ulStatus = oCommand.response[0];
		ulState = (ulStatus >> 9) & 0xF;

		break;


	default:
		BT_kPrint("SDCARD: Unknown card state %d", ulState);
		break;
	}

	if(!hBlock->pHost->bSDHC) {
		ulBlock *= 512;
	}

	oCommand.opcode 		= 25;
	oCommand.bCRC 			= BT_FALSE;
	oCommand.ulResponseType = 48;
	oCommand.bIsData 		= BT_TRUE;
	oCommand.arg 			= ulBlock;
	oCommand.bRead_nWrite	= BT_FALSE;
	oCommand.ulBlocks		= ulCount;

	hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);

	BT_u32 cmd25_response = oCommand.response[0];
	if(cmd25_response != 0x900) {	// STATE = transfer, READY_FOR_DATA = set
		BT_kPrint("SDCARD: Invalid CMD25 response (0x%08x)",cmd25_response);
		return 0;
	}

	//BT_kPrint("SDCARD: Write command complete, writing data now");

	// Write the data.

	BT_u32 ulWritten = hBlock->pHost->pOps->pfnWrite(hBlock->pHost->hHost, ulCount, pBuffer, pError);

	return ulWritten;
}

static const BT_IF_BLOCK oBlockInterface = {
	sdcard_blockread,
	sdcard_blockwrite,
};

static BT_ERROR sdcard_blockdev_cleanup(BT_HANDLE hHandle) {
	// Whenever this is called, its probably just the application layer saying.... i'm done for now!

	// However.... we must only close the device if the actual reference count drops to 0 in the inode handle,

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

BT_MODULE_INIT_0_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_sdcard_manager_init,
};
