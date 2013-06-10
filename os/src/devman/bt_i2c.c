#include <bitthunder.h>
#include <interrupts/bt_tasklets.h>

BT_DEF_MODULE_NAME			("BT I2C Bus Manager")
BT_DEF_MODULE_DESCRIPTION	("Manages I2C bus adapters, and handles I2C device probing")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_LIST 		g_oI2CBusses = { NULL };
static BT_TASKLET 	sm_tasklet;

typedef struct _BT_I2C_BUS {
	BT_LIST_ITEM 	oItem;
	BT_HANDLE 		hBus;
	BT_u32			ulID;
	const BT_I2C_BOARD_INFO *pInfo;
	BT_u32			ulNum;
	BT_u32			ulStateFlags;
	#define SM_PROBE_DEVICES 0x00000001
} BT_I2C_BUS;

static BT_I2C_BUS *getBusByID(BT_u32 ulID) {
	if(!BT_ListInitialised(&g_oI2CBusses)) {
		return NULL;
	}

	BT_I2C_BUS *pBus = (BT_I2C_BUS *) BT_ListGetHead(&g_oI2CBusses);
	while(pBus) {
		if(pBus->ulID == ulID) {
			return pBus;
		}
		pBus = (BT_I2C_BUS *) BT_ListGetNext(&pBus->oItem);
	}

	return NULL;
}

BT_ERROR BT_I2C_RegisterBusWithID(BT_HANDLE hBus, BT_u32 ulBusID, BT_I2C_BOARD_INFO *pInfo, BT_u32 ulNum) {
	BT_I2C_BUS *pBus = getBusByID(ulBusID);
	if(pBus) {
		return BT_ERR_GENERIC;	// Computer says no! Bus with same ID already exist!
	}

	pBus = BT_kMalloc(sizeof(BT_I2C_BUS));
	if(!pBus) {
		return BT_ERR_NO_MEMORY;
	}

	pBus->ulID 	= ulBusID;
	pBus->hBus 	= hBus;
	pBus->pInfo = pInfo;
	pBus->ulNum = ulNum;
	pBus->ulStateFlags = SM_PROBE_DEVICES;

	BT_TaskletSchedule(&sm_tasklet);

	return BT_ERR_NONE;
}

BT_ERROR BT_I2C_RegisterDevices(BT_u32 ulBusID, BT_I2C_BOARD_INFO *pInfo, BT_u32 ulNum) {


	return BT_ERR_NONE;
}


static void i2c_probe_devices(BT_I2C_BUS *pBus) {
	BT_u32 i;
	BT_ERROR Error;

	for(i = 0; i < pBus->ulNum; i++) {
		const BT_I2C_BOARD_INFO *pInfo = &pBus->pInfo[i];
		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(pInfo->name);
		if(!pDriver || pDriver->eType != BT_DRIVER_I2C) {
			continue;
		}

		pDriver->pfnI2CProbe(pBus->hBus, pInfo, &Error);
	}
}

static void i2c_sm(void *pData) {
	BT_I2C_BUS *pBus = (BT_I2C_BUS *) BT_ListGetHead(&g_oI2CBusses);
	while(pBus) {

		if(pBus->ulStateFlags & SM_PROBE_DEVICES) {
			i2c_probe_devices(pBus);
			pBus->ulStateFlags &= ~SM_PROBE_DEVICES;
		}

		pBus = (BT_I2C_BUS *) BT_ListGetNext(&pBus->oItem);
	}
}

static BT_TASKLET sm_tasklet = {NULL, BT_TASKLET_IDLE, i2c_sm, NULL};
