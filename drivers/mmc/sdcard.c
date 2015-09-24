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
#include "sdcard.h"

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
	BT_HANDLE_HEADER 		h;
	MMC_HOST 		   	   *pHost;
	BT_BLKDEV_DESCRIPTOR	oDescriptor;
	BT_BOOL					bInUse;
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
	pHost->pOps  = pOps;
	pHost->rca   = 0;

	if(pHost->pOps->pfnEventSubscribe) {
		pHost->pOps->pfnEventSubscribe(hHost, card_event_handler, pHost);
	}

	pHost->ulHostID = g_oSDHosts.ulItems;
	BT_ListAddItem(&g_oSDHosts, &pHost->oItem);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_RegisterSDHostController);

static void sd_manager_sm(void *pData) {

	BT_ERROR Error;

	MMC_HOST *pHost = (MMC_HOST *) BT_ListGetHead(&g_oSDHosts);
	while(pHost) {

		if(pHost->ulFlags & MMC_HOST_FLAGS_INITIALISE_REQUEST) {
			pHost->ulFlags &= ~MMC_HOST_FLAGS_INITIALISE_REQUEST;

			BT_ThreadSleep(10);

			if(pHost->pOps->pfnIsCardPresent(pHost->hHost, &Error)) {

				BT_kDebug("SDCard was inserted");

				// Attempt basic SDIO initialisation (host-specific).
				pHost->pOps->pfnInitialise(pHost->hHost);

				pHost->pOps->pfnEnableClock(pHost->hHost);

				if (pHost->pOps->pfnSelect)
					pHost->pOps->pfnSelect(pHost->hHost);

				BT_ThreadSleep(10);

				// Send GO-IDLE (CMD0), expecting no response!
				MMC_COMMAND oCommand;
				oCommand.arg 			= 0;
				oCommand.opcode 		= 0;
				oCommand.bCRC			= BT_FALSE;
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_NONE;
				oCommand.bIsData		= 0;

				Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
				if(Error) {
					BT_kDebug("%s: GO_IDLE timed out.");
					goto next_host;
				}

				BT_kDebug("Sent GO_IDLE");

				// Send CMD8 (SEND_IF_COND -- Helps determine SDHC support).
				oCommand.arg 			= 0x000001AA;		//	0xAA is the test field, it can be anything, 0x100 is the voltage range, 2.7-3.6V).
				oCommand.opcode 		= 8;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R7;
				oCommand.bIsData		= 0;

				Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
				if(Error) {
					BT_kDebug("SEND_IF_COND timed out.");
					goto next_host;
				}

				if((oCommand.response[0] & 0xFFF) != 0x1AA) {
					oCommand.bCRC = BT_TRUE;
				}

				BT_kDebug("Sent SEND_IF_COND, response = %08x", oCommand.response[0]);

				while(1) {	/* Loop while the SD-Card initialised itself. */

					oCommand.opcode = 55;				// Send CMD55 to tell card we are doing an Application Command.
					oCommand.arg 	= 0;
					oCommand.bCRC 	= BT_TRUE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
					oCommand.bIsData		= 0;

					Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
					if(Error) {
						BT_kDebug("CMD%d timed out.", oCommand.opcode);
						goto next_host;
					}

					oCommand.opcode = 41;				// Send ACMD41
					oCommand.arg 	= 0x40FF8000;		// Tell card  High Capacity mode is supported, and specify valid voltage windows.
					if (pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
						oCommand.arg 	= 0x40000000;		// Tell card  High Capacity mode is supported
					oCommand.bCRC	= BT_FALSE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R3;
					oCommand.bIsData		= 0;
					if (pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
						oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;

					Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
					if(Error) {
						BT_kDebug("CMD%d timed out.", oCommand.opcode);
						goto next_host;
					}

					BT_u32 card_ready = oCommand.response[0] >> 31;
					if (pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
						card_ready = (oCommand.response[0] == 0x00);
					if(!card_ready) {
						continue;
					}
					break;
				}

				// Check if the card is ready?
				if (pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE) {
					oCommand.opcode = 58;				// Send CMD58
					oCommand.arg 	= 0;		// Tell card  High Capacity mode is supported, and specify valid voltage windows.
					oCommand.bCRC	= BT_FALSE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R3;
					oCommand.bIsData		= 0;

					pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
				}

				pHost->bSDHC = (oCommand.response[0] >> 30) & 1;

				if(pHost->bSDHC) {
					BT_kDebug("SDHC card support detected");
				} else {
					BT_kDebug("Non-SDHC card detected");
				}


				// Read the CID register
				oCommand.arg 			= pHost->rca << 16;
				oCommand.opcode 		= 2;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1_DATA;			//equals to SDIO R2
				oCommand.bIsData 		= 0;
				if (pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
					oCommand.opcode 		= 10;

				Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
				if(Error) {
					BT_kDebug("CMD%d timed out.", oCommand.opcode);
					goto next_host;
				}

				tCID *CID = (tCID*)oCommand.response;

				char pnm[6];
				memcpy(pnm, CID->PNM, 5);
				pnm[5] = '\0';

				BT_kDebug("Manufacturer ID       : %d", CID->MID);
				BT_kDebug("OEM/Application ID    : 0x%1x%1x", CID->OID[0], CID->OID[1]);
				BT_kDebug("Productname           : %s", pnm);
				BT_kDebug("Product revision      : %d.%d", CID->PRVMajor, CID->PRVMinor);
				BT_kDebug("Product serial number : %d", CID->PSN);
				BT_kDebug("Manufacturing date    : %d.%d", CID->Month, CID->Year+2000);

				// We can use the information in the CID register to get things like the CARD S/N etc and manufacturer code.
				BT_kDebug("CID reg %08x:%08x:%08x:%08x", oCommand.response[3], oCommand.response[2], oCommand.response[1], oCommand.response[0]);


				BT_u32 crcError		 = 0;
				BT_u32 illegal_cmd	 = 0;
				BT_u32 error	  	 = 0;
				BT_u32 status	     = 0;
				BT_u32 ready		 = 1;
				if (!(pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)) {
					// Place the command into the data stat (CMD3).
					oCommand.arg 			= 0;
					oCommand.opcode 		= 3;
					oCommand.bCRC 			= BT_TRUE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R7;
					oCommand.bIsData 		= 0;

					pHost->pOps->pfnRequest(pHost->hHost, &oCommand);

					pHost->rca = oCommand.response[0] >> 16;

					crcError	 = (oCommand.response[0] >> 15) & 0x1;
					illegal_cmd	 = (oCommand.response[0] >> 14) & 0x1;
					error	  	 = (oCommand.response[0] >> 13) & 0x1;
					status	     = (oCommand.response[0] >> 9) & 0xf;
					ready		 = (oCommand.response[0] >> 8) & 0x1;
				}


				if(crcError) {
					BT_kDebug("CRC Error");
				}

				if(illegal_cmd) {
					BT_kDebug("Illegal command");
				}

				if(error) {
					BT_kDebug("Generic Error");
				}

				if(!ready) {
					BT_kDebug("not ready in data state!");
				}

				BT_kDebug("Placed SDCARD into data state. (resp: %08x)", oCommand.response[0]);
				BT_kDebug("Relative Card Address (RCA): %04x", pHost->rca);

				// Get CSD

				oCommand.arg 			= pHost->rca << 16;
				oCommand.opcode 		= 9;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1_DATA;			//equals to SDIO R2
				oCommand.bIsData 		= 0;
				if (pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1_DATA;

				Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
				if(Error) {
					BT_kDebug("CMD%d timed out.", oCommand.opcode);
					goto next_host;
				}

				/*
				 * BLOCKnr = (C_SIZE+1) * MULT
				 * MULT = 2^(C_SIZE_MULT+2)			.. C_SIZE_MULT is less than 8
				 * BLOCK_LEN = 2^READ_BL_LEN		.. READ_BL_LEN is less than 12
				 */
				
				BT_u32 ulBlocks = 0;
				BT_u32 ulBlockSize = 512;
				BT_u32 csdversion = 0;
				
				csdversion = (oCommand.response[3] >> 22) & 0x3;
					
				if (csdversion == 0) {
					tCSD1_x *CSD = (tCSD1_x*)oCommand.response;
					ulBlockSize  = ((BT_u32)0x01 << (CSD->Read_BL_Len));
					ulBlocks     = CSD->C_Size * ((BT_u32)0x01 << (CSD->C_Size_Mult + 2));
				}
				else if (csdversion == 1) {
					tCSD2_x *CSD = (tCSD2_x*)oCommand.response;
					ulBlockSize  = ((BT_u32)0x01 << (CSD->Read_BL_Len));
					ulBlocks     = CSD->C_Size * 1024;
				}
				else {
					BT_kDebug("SDCARD: Unrecognised CSD register structure version.");
				}

				BT_kDebug("Block Size            : %lu", ulBlockSize);
				BT_kDebug("Total Blocks          : %lu", ulBlocks);
				BT_kDebug("Total Size (bytes)    : %llu", (BT_u64) (ulBlocks * ulBlockSize));
				BT_kDebug("Total Size (mb)       : %llu", (BT_u64) ((ulBlocks * ulBlockSize) / 1024 / 1024));


				if (!(pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)) {
					oCommand.arg = pHost->rca << 16;
					oCommand.opcode = 7;
					oCommand.bCRC = BT_TRUE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1b;
					oCommand.bIsData		= 0;

					Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
					if(Error) {
						BT_kDebug("CMD%d timed out.", oCommand.opcode);
						goto next_host;
					}

					BT_kDebug("Selected card mmc%d:%04x", pHost->ulHostID, pHost->rca);

					BT_u32 cmd7_response = oCommand.response[0];
					status = (cmd7_response >> 9) & 0xf;

					if(status != 3 && status !=4) {
						BT_kDebug("invalid status %i", status);
					}
				}

				if(!pHost->bSDHC) {
					oCommand.arg = 512;
					oCommand.opcode = 16;
					oCommand.bCRC = BT_TRUE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
					oCommand.bIsData		= 0;

					Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
					if(Error) {
						BT_kDebug("CMD%d timed out.", oCommand.opcode);
						goto next_host;
					}
				}

				if (pHost->pOps->pfnSetBlockSize)
					pHost->pOps->pfnSetBlockSize(pHost->hHost, ulBlockSize);

				if (!(pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)) {
					// Place SDCard into 4-bit mode. (ACMD6).

					oCommand.arg = pHost->rca << 16;
					oCommand.opcode = 55;
					oCommand.bCRC = BT_TRUE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
					oCommand.bIsData		= 0;

					Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
					if(Error) {
						BT_kDebug("CMD%d timed out.", oCommand.opcode);
						goto next_host;
					}

					oCommand.arg = 0x2;	// Set argument to be 4-bit mode
					oCommand.opcode = 6;
					oCommand.bCRC = BT_TRUE;
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
					oCommand.bIsData		= 0;

					Error = pHost->pOps->pfnRequest(pHost->hHost, &oCommand);
					if(Error) {
						BT_kDebug("CMD%d timed out.", oCommand.opcode);
						goto next_host;
					}

					if (pHost->pOps->pfnSetDataWidth)
						pHost->pOps->pfnSetDataWidth(pHost->hHost, BT_MMC_WIDTH_4BIT);

					BT_kDebug("Configured card for 4-bit data width. (resp: %08x)", oCommand.response[0]);
				}

				BT_kDebug("sucessfully initialised... registering block device");
				// Card initialised -- regster block device driver :)

				BT_HANDLE hSD = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
				if(!hSD) {

				}

				hSD->pHost = pHost;
				//hSD->oDescriptor.

				hSD->oDescriptor.oGeometry.ulBlockSize = ulBlockSize;
				hSD->oDescriptor.oGeometry.ulTotalBlocks = ulBlocks;


				char buffer[10];
				bt_sprintf(buffer, "mmc%d", (int) pHost->ulHostID);

				if (pHost->pOps->pfnDeselect)
					pHost->pOps->pfnDeselect(pHost->hHost);

				BT_RegisterBlockDevice(hSD, buffer, &hSD->oDescriptor);

			}
		}

		if(pHost->ulFlags & MMC_HOST_FLAGS_INVALIDATE) {
			pHost->ulFlags &= ~MMC_HOST_FLAGS_INVALIDATE;
			if(!pHost->pOps->pfnIsCardPresent(pHost->hHost, &Error)) {
				BT_GpioSet(7, BT_FALSE);
				BT_kPrint("SDCARD: SDCard (mmc%d:%04x) was removed", pHost->ulHostID, pHost->rca);
			}
		}

next_host:
		pHost = (MMC_HOST *) BT_ListGetNext(&pHost->oItem);
	}
}


static BT_s32 sdcard_blockread(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer) {

	if ((!hBlock->pHost->rca) && (!(hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE))) {
		return BT_ERR_GENERIC;
	}

	BT_ERROR Error = BT_ERR_NONE;
	BT_s32 slRead = 0;
	BT_s32 nlRetryCount = 0;
	BT_u32 ulStatus = 0;
	BT_u32 ulState = 0;
	MMC_COMMAND oCommand;

	if (hBlock->pHost->pOps->pfnSelect)
		hBlock->pHost->pOps->pfnSelect(hBlock->pHost->hHost);

	while(1) {
		if (!(hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)) {
			oCommand.opcode 		= 13;
			oCommand.arg 			= hBlock->pHost->rca << 16;
			oCommand.bCRC 			= BT_TRUE;
			oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
			oCommand.bIsData		= BT_FALSE;
			if (hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R2;

			Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
			if(Error) {
				BT_kDebug("CMD%d timed out.", oCommand.opcode);
				if(nlRetryCount >= 3) {
					return BT_ERR_GENERIC;
				}
			}

			ulStatus = oCommand.response[0];
			ulState = (ulStatus >> 9) & 0xF;

			switch(ulState) {

			case 4:
				break;

			case 5:
				oCommand.opcode = 12;
				oCommand.arg = 0;
				oCommand.bCRC = BT_TRUE;
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1b;
				oCommand.bIsData = BT_FALSE;

				Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
				if(Error) {
					BT_kDebug("CMD%d timed out.", oCommand.opcode);
					return Error;
				}

				oCommand.opcode 		= 13;
				oCommand.arg 			= hBlock->pHost->rca << 16;
				oCommand.bCRC 			= BT_TRUE;
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
				oCommand.bIsData		= BT_FALSE;
				if (hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
					oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R2;

				Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
				if(Error) {
					BT_kDebug("CMD%d timed out.", oCommand.opcode);
					return Error;
				}

				ulStatus = oCommand.response[0];
				ulState = (ulStatus >> 9) & 0xF;
				break;


			default:
				BT_kDebug("Unknown card state %d", ulState);
				break;
			}
		}

		BT_u32 ulSize = ulBlock;
		if(!hBlock->pHost->bSDHC) {
			ulSize *= 512;
		}

		oCommand.opcode 		= 18;
		oCommand.bCRC 			= BT_FALSE;
		oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
		oCommand.bIsData 		= BT_TRUE;
		oCommand.arg 			= ulSize;
		oCommand.bRead_nWrite	= BT_TRUE;
		oCommand.ulBlocks		= ulCount;

		if ((hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE) && (ulCount == 1)) {
			oCommand.opcode = 17;
		}

		Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
		if(Error) {
			BT_kDebug("CMD%d timed out.", oCommand.opcode);
			if(nlRetryCount >= 3) {
				return BT_ERR_GENERIC;
			}
		}

		BT_u32 cmd18_response = oCommand.response[0];

		if (hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE) {
			if (!(cmd18_response & 0x01))
				cmd18_response = 0x900;
		}
		if(cmd18_response != 0x900) {	// STATE = transfer, READY_FOR_DATA = set
			BT_kDebug("Invalid CMD18 response");
			return BT_ERR_GENERIC;
		}

		slRead = hBlock->pHost->pOps->pfnRead(hBlock->pHost->hHost, ulCount, pBuffer);

		if ((hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE) && (ulCount != 1)) {
			oCommand.opcode 		= 12;
			oCommand.bCRC 			= BT_FALSE;
			oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1b;

			Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
			if(Error) {
				BT_kDebug("CMD%d timed out.", oCommand.opcode);
				if(nlRetryCount >= 3) {
					return BT_ERR_GENERIC;
				}
			}
		}

		if(slRead == ulCount || slRead < 0) break;

		if(nlRetryCount++ >= 3) {
			BT_kDebug("(%d,%d) fatal error!", ulBlock, ulCount);
			return BT_ERR_GENERIC;
			break;
		} else {
			BT_kDebug("read block (%d,%d) error, retrying (%d) ... ", ulBlock, ulCount, nlRetryCount);
		}
	}

	if (hBlock->pHost->pOps->pfnDeselect)
		hBlock->pHost->pOps->pfnDeselect(hBlock->pHost->hHost);


	return slRead;
}

static BT_s32 sdcard_blockwrite(BT_HANDLE hBlock, BT_u32 ulBlock, BT_u32 ulCount, void *pBuffer) {

	BT_ERROR Error = BT_ERR_NONE;

	if ((!hBlock->pHost->rca) && (!(hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE))) {
		return BT_ERR_GENERIC;
	}

	if (hBlock->pHost->pOps->pfnSelect)
		hBlock->pHost->pOps->pfnSelect(hBlock->pHost->hHost);

	MMC_COMMAND oCommand;
	if (!(hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)) {
		oCommand.opcode 		= 13;
		oCommand.arg 			= hBlock->pHost->rca << 16;
		oCommand.bCRC 			= BT_TRUE;
		oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
		oCommand.bIsData		= BT_FALSE;
		if (hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
			oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R2;

		Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
		if(Error) {
			BT_kDebug("CMD%d timed out.", oCommand.opcode);
			return Error;
		}

		BT_u32 ulStatus = oCommand.response[0];
		BT_u32 ulState = (ulStatus >> 9) & 0xF;

		switch(ulState) {

		case 4:
			break;

		case 5:
			oCommand.opcode = 12;
			oCommand.arg = 0;
			oCommand.bCRC = BT_TRUE;
			oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1b;
			oCommand.bIsData = BT_FALSE;

			Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
			if(Error) {
				BT_kDebug("CMD%d timed out.", oCommand.opcode);
				return Error;
			}


			BT_kDebug("Sent CMD12");

			oCommand.opcode 		= 13;
			oCommand.arg 			= hBlock->pHost->rca << 16;
			oCommand.bCRC 			= BT_TRUE;
			oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
			oCommand.bIsData		= BT_FALSE;
			if (hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE)
				oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R2;

			Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
			if(Error) {
				BT_kDebug("CMD%d timed out.", oCommand.opcode);
				return Error;
			}

			BT_u32 ulStatus = oCommand.response[0];
			ulState = (ulStatus >> 9) & 0xF;

			break;


		default:
			BT_kDebug("Unknown card state %d", ulState);
			break;
		}
	}

	if(!hBlock->pHost->bSDHC) {
		ulBlock *= 512;
	}

	oCommand.opcode 		= 25;
	oCommand.bCRC 			= BT_FALSE;
	oCommand.ulResponseType = BT_SDCARD_RESPONSE_TYPE_R1;
	oCommand.bIsData 		= BT_TRUE;
	oCommand.arg 			= ulBlock;
	oCommand.bRead_nWrite	= BT_FALSE;
	oCommand.ulBlocks		= ulCount;

	Error = hBlock->pHost->pOps->pfnRequest(hBlock->pHost->hHost, &oCommand);
	if(Error) {
		BT_kDebug("CMD%d timed out.", oCommand.opcode);
		return Error;
	}

	BT_u32 cmd25_response = oCommand.response[0];
	if (hBlock->pHost->pOps->ulCapabilites1 & BT_MMC_SPI_MODE) {
		if (!(cmd25_response & 0x01))
			cmd25_response = 0x900;
	}
	if(cmd25_response != 0x900) {	// STATE = transfer, READY_FOR_DATA = set
		BT_kDebug("Invalid CMD25 response (0x%08x)", cmd25_response);
		return BT_ERR_GENERIC;
	}

	BT_s32 slWritten = hBlock->pHost->pOps->pfnWrite(hBlock->pHost->hHost, ulCount, pBuffer);

	if (hBlock->pHost->pOps->pfnDeselect)
		hBlock->pHost->pOps->pfnDeselect(hBlock->pHost->hHost);

	return slWritten;
}

static const BT_IF_BLOCK sdcard_blockdev_interface = {
	.pfnReadBlocks 	= sdcard_blockread,
	.pfnWriteBlocks = sdcard_blockwrite,
};

static BT_ERROR sdcard_blockdev_cleanup(BT_HANDLE hHandle) {
	// Whenever this is called, its probably just the application layer saying.... i'm done for now!

	// However.... we must only close the device if the actual reference count drops to 0 in the inode handle,

	return BT_ERR_NONE;
}

static const BT_IF_DEVICE oDeviceInterface = {
	.pBlockIF = &sdcard_blockdev_interface,
};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.pfnCleanup = sdcard_blockdev_cleanup,
	.eType = BT_HANDLE_T_DEVICE,
	.oIfs = {
		.pDevIF = &oDeviceInterface,
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
