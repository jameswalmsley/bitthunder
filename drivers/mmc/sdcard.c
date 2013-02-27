/**
 *	SDCard Protocol Implementation.
 *
 *	This driver provides a block device wrapper for sd host I/O devices.
 *
 **/

#include <bitthunder.h>
#include "core.h"

BT_DEF_MODULE_NAME			("BitThunder SDCARD Manager")
BT_DEF_MODULE_DESCRIPTION	("SDCARD abstraction layer for BitThunder")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

typedef struct _MMC_HOST {
	BT_LIST_ITEM	oItem;
	BT_HANDLE 		hHost;
	const BT_MMC_OPS		*pOps;
} MMC_HOST;

static BT_LIST g_oSDHosts = {0};

static void card_event_handler(MMC_HOST *pHost, BT_MMC_CARD_EVENT eEvent) {
	switch(eEvent) {

	case BT_MMC_CARD_DETECTED:
		// Signal SD-Card manager to attempt to initialise this host.
		break;

	case BT_MMC_CARD_REMOVED:
		// Signal SD-Card manager to invalidate all handles dependent on this host.
		break;

	default:
		break;
	}

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


static BT_ERROR bt_sdcard_manager_init() {

	BT_ListInit(&g_oSDHosts);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_sdcard_manager_init,
};
