#include <bitthunder.h>
#include <interrupts/bt_tasklets.h>
#include <of/bt_of.h>

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

BT_I2C_BUS *BT_I2C_GetBusByID(BT_u32 ulBusID) {
	if (bt_list_empty(&g_i2c_busses)) {
		return NULL;
	}

	struct bt_list_head *pos;
	bt_list_for_each(pos, &g_i2c_busses) {
		BT_I2C_BUS *pBus = (BT_I2C_BUS *) bt_container_of(pos, BT_I2C_BUS, item);
		if(pBus->ulBusID == ulBusID) {
			return pBus;
		}
	}

	return NULL;
}
BT_EXPORT_SYMBOL(BT_I2C_GetBusByID);

BT_ERROR BT_I2C_RegisterBus(BT_I2C_BUS *pBus) {
	if(BT_I2C_GetBusByID(pBus->ulBusID)) {
		return BT_ERR_GENERIC;	// Computer says no! Bus with same ID already exist!
	}

	pBus->ulStateFlags = BT_I2C_SM_PROBE_DEVICES;
	pBus->pMutex = BT_kMutexCreate();

	BT_AttachHandle(NULL, &oHandleInterface, (BT_HANDLE) &pBus->h);

	bt_list_add(&pBus->item, &g_i2c_busses);

	BT_TaskletSchedule(&sm_tasklet);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_I2C_RegisterBus);

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
		if(!pDriver || (pDriver->eType & BT_DRIVER_TYPE_CODE_MASK) != BT_DRIVER_I2C) {
			continue;
		}

		const BT_RESOURCE *pResource = BT_GetResource(pDevice->pResources, pDevice->ulTotalResources, BT_RESOURCE_BUSID, 0);
		if(!pResource || pResource->ulStart != pBus->ulBusID) {
			continue;
		}

		pDriver->pfnI2CProbe((BT_HANDLE) pBus, pDevice, &Error);
	}

	// Probe OF devices.
#ifdef BT_CONFIG_OF
	struct bt_device_node *dev = bt_of_integrated_get_node(pBus->pDevice);
	if(!dev) {
		return;
	}

	struct bt_list_head *pos;
	bt_list_for_each(pos, &dev->children) {
		struct bt_device_node *i2c_device = (struct bt_device_node *) pos;
		// Populate the device resources
		bt_of_i2c_populate_device(i2c_device);
		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(i2c_device->dev.name);
		if(!pDriver || (pDriver->eType & BT_DRIVER_TYPE_CODE_MASK) != BT_DRIVER_I2C) {
			continue;
		}

		pDriver->pfnI2CProbe((BT_HANDLE) pBus, &i2c_device->dev, &Error);
	}
#endif
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
BT_EXPORT_SYMBOL(BT_I2C_GetBusObject);

static BT_TASKLET sm_tasklet = {NULL, BT_TASKLET_IDLE, i2c_sm, NULL};

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.ulFlags = BT_HANDLE_FLAGS_NO_DESTROY,
	.eType = BT_HANDLE_T_I2C_BUS,
	.pfnCleanup = i2c_bus_cleanup,
};
