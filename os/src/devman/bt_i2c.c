#include <bitthunder.h>
#include <interrupts/bt_tasklets.h>

BT_DEF_MODULE_NAME			("BT I2C Bus Manager")
BT_DEF_MODULE_DESCRIPTION	("Manages I2C bus adapters, and handles I2C device probing")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_LIST_HEAD(g_i2c_busses);

static BT_TASKLET 	sm_tasklet;

static const BT_IF_HANDLE oHandleInterface;

static BT_HANDLE i2c_bus_open(struct bt_devfs_node *node, BT_ERROR *pError) {

	BT_I2C_BUS *pBus = (BT_I2C_BUS *) bt_container_of(node, BT_I2C_BUS, node);
	if(!pBus->ulReferenceCount) {
		pBus->ulReferenceCount += 1;
		BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &pBus->h);
		return (BT_HANDLE) &pBus->h;
	}

	return NULL;
}

static const BT_DEVFS_OPS i2c_devfs_ops = {
	.pfnOpen = i2c_bus_open,
};

static BT_ERROR i2c_bus_cleanup(BT_HANDLE h) {
	BT_I2C_BUS *pBus = (BT_I2C_BUS *) h;
	pBus->ulReferenceCount -= 1;
	return BT_ERR_NONE;
}

BT_I2C_BUS *BT_I2C_GetBusByID(BT_u32 ulID) {
	if (bt_list_empty(&g_i2c_busses)) {
		return NULL;
	}

	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_i2c_busses) {
		BT_I2C_BUS *pBus = (BT_I2C_BUS *) bt_container_of(pos, BT_I2C_BUS, item);
		if(pBus->ulID == ulID) {
			return pBus;
		}
	}

	return NULL;
}

BT_ERROR BT_I2C_RegisterBusWithID(BT_HANDLE hBus, BT_u32 ulBusID) {
	BT_I2C_BUS *pBus = BT_I2C_GetBusByID(ulBusID);
	// @@MS: char name[16];
	if(pBus) {
		return BT_ERR_GENERIC;	// Computer says no! Bus with same ID already exist!
	}

	pBus = BT_kMalloc(sizeof(BT_I2C_BUS));
	if(!pBus) {
		return BT_ERR_NO_MEMORY;
	}

	pBus->ulID 	= ulBusID;
	pBus->hBus 	= hBus;
	pBus->ulStateFlags = BT_I2C_SM_PROBE_DEVICES;
	pBus->pMutex = BT_kMutexCreate();

	bt_list_add(&pBus->item, &g_i2c_busses);
	// @@MS: start
	// @@MS: pBus->node.pOps = &i2c_devfs_ops;
	// @@MS: snprintf(name,16,"_i2c%u",(unsigned int)ulBusID);
	// @@MS: BT_DeviceRegister(&pBus->node, name);
	// @@MS: start

	BT_TaskletSchedule(&sm_tasklet);

	return BT_ERR_NONE;
}

static void i2c_probe_devices(BT_I2C_BUS *pBus) {
	BT_u32 i;
	BT_ERROR Error;

	BT_u32 ulTotalDevices = BT_GetTotalDevicesByType(BT_DEVICE_I2C);
	for(i = 0; i < ulTotalDevices; i++) {
		const BT_DEVICE *pDevice = BT_GetDeviceByType(BT_DEVICE_I2C, i);
		if(!pDevice) {
			continue;
		}

		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(pDevice->name);
		if(!pDriver || pDriver->eType != BT_DRIVER_I2C) {
			continue;
		}

		const BT_RESOURCE *pResource = BT_GetResource(pDevice->pResources, pDevice->ulTotalResources, BT_RESOURCE_BUSID, 0);
		if(!pResource || pResource->ulStart != pBus->ulID) {
			continue;
		}

		pDriver->pfnI2CProbe((BT_HANDLE) pBus, pDevice, &Error);
	}
}

static void i2c_sm(void *pData) {

	struct bt_list_head *pos;
	
	bt_list_for_each(pos, &g_i2c_busses) {
		BT_I2C_BUS *pBus = (BT_I2C_BUS *) bt_container_of(pos, BT_I2C_BUS, item);
		if(pBus->ulStateFlags & BT_I2C_SM_PROBE_DEVICES) {
			i2c_probe_devices(pBus);
			pBus->ulStateFlags &= ~BT_I2C_SM_PROBE_DEVICES;
		}
	}
}

BT_I2C_BUS *BT_I2C_GetBusObject(BT_HANDLE hBus) {
	 return (BT_I2C_BUS *) hBus;
}

static BT_TASKLET sm_tasklet = {NULL, BT_TASKLET_IDLE, i2c_sm, NULL};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.ulFlags = BT_HANDLE_FLAGS_NO_DESTROY,
	.eType = BT_HANDLE_T_I2C_BUS,
	.pfnCleanup = i2c_bus_cleanup,
};
