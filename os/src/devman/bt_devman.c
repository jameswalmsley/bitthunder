/**
 *	BitThunder Device Manager
 *
 *
 **/


#include <bitthunder.h>
#include <string.h>
#include <stdio.h>

#ifdef BT_CONFIG_OF
#include <collections/bt_list.h>
#include <of/bt_of.h>
#endif


extern const BT_MACHINE_DESCRIPTION * __bt_arch_init_start;
extern const BT_MACHINE_DESCRIPTION * __bt_arch_init_end;

extern const BT_u32					  __bt_arch_init_size;

extern const BT_INTEGRATED_DEVICE * __bt_arch_devices_start;
extern const BT_INTEGRATED_DEVICE * __bt_arch_devices_end;

extern const BT_INTEGRATED_DRIVER * __bt_arch_drivers_start;
extern const BT_INTEGRATED_DRIVER * __bt_arch_drivers_end;

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

BT_MACHINE_DESCRIPTION *BT_GetMachineDescription(BT_ERROR *pError) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_init_end - (BT_u32) &__bt_arch_init_start);

	if(size != sizeof(BT_MACHINE_DESCRIPTION)) {
		if(pError) {
			*pError = BT_ERR_GENERIC;
		}
		return NULL;
	}

	return	(BT_MACHINE_DESCRIPTION *) &__bt_arch_init_start;
}

BT_INTEGRATED_DEVICE *BT_GetIntegratedDeviceByName(const BT_i8 *szpName) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_devices_end - (BT_u32) &__bt_arch_devices_start);
	BT_u32 i;

	size /= sizeof(BT_INTEGRATED_DEVICE);

	BT_INTEGRATED_DEVICE *pDevice = (BT_INTEGRATED_DEVICE *) &__bt_arch_devices_start;

	for(i = 0; i < size; i++) {
		if(!strcmp(szpName, pDevice->name)) {
			return pDevice;
		}

		pDevice++;
	}

	return NULL;
}

BT_INTEGRATED_DRIVER *BT_GetIntegratedDriverByName(const BT_i8 *szpName) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_drivers_end - (BT_u32) &__bt_arch_drivers_start);
	BT_u32 i;

	size /= sizeof(BT_INTEGRATED_DRIVER);

	for(i = 0; i < size; i++) {
		BT_INTEGRATED_DRIVER *pDriver = (BT_INTEGRATED_DRIVER *) &__bt_arch_drivers_start;
		pDriver += i;

		if(!strcmp(szpName, pDriver->name)) {
			return pDriver;
		}
	}

	return NULL;
}

BT_u32 BT_GetTotalIntegratedDriversByType(BT_DRIVER_TYPE eType) {
	BT_u32 total = 0;
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_drivers_end - (BT_u32) &__bt_arch_drivers_start);
	BT_u32 i;

	size /= sizeof(BT_INTEGRATED_DRIVER);

	for(i = 0; i < size; i++) {
		BT_INTEGRATED_DRIVER *pDriver = (BT_INTEGRATED_DRIVER *) &__bt_arch_drivers_start;
		pDriver += i;

		if(pDriver->eType == eType) {
			total += 1;
		}
	}

	return total;
}

BT_INTEGRATED_DRIVER *BT_GetIntegratedDriverByType(BT_DRIVER_TYPE eType, BT_u32 i) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_drivers_end - (BT_u32) &__bt_arch_drivers_start);
	size /= sizeof(BT_INTEGRATED_DRIVER);

	BT_u32 y;
	for(y = 0; y < size; y++) {
		BT_INTEGRATED_DRIVER *pDriver = (BT_INTEGRATED_DRIVER *) &__bt_arch_drivers_start;
		pDriver += y;

		if(pDriver->eType == eType) {
			if(!i--) {
				return pDriver;
			}
		}
	}

	return NULL;
}

const BT_RESOURCE *BT_GetIntegratedResource(const BT_INTEGRATED_DEVICE *pDevice, BT_u32 ulType, BT_u32 ulNum) {
	return BT_GetResource(pDevice->pResources, pDevice->ulTotalResources, ulType, ulNum);
}


BT_ERROR BT_ProbeIntegratedDevices(BT_HANDLE hLogDevice) {

	BT_ERROR Error;
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_devices_end - (BT_u32) &__bt_arch_devices_start);
	BT_u32 i;

	size /= sizeof(BT_INTEGRATED_DEVICE);

	for(i = 0; i < size; i++) {
		BT_INTEGRATED_DEVICE *pDevice = (BT_INTEGRATED_DEVICE *) &__bt_arch_devices_start;
		pDevice += i;
		if((pDevice->eType & BT_DEVICE_TYPE_CODE_MASK) != BT_DEVICE_INTEGRATED) {
			continue;
		}

		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(pDevice->name);
		if(pDriver && (pDriver->eType & BT_DRIVER_TYPE_CODE_MASK) == BT_DRIVER_INTEGRATED) {
			BT_HANDLE hDevice = pDriver->pfnProbe(pDevice, &Error);
			if(Error) {
				BT_kPrint("Error probing: %s device.", pDevice->name);
			}

			if(hDevice) {
				BT_kPrint("Registered %s with %s driver", pDevice->name, hDevice->h.pIf->oInfo.szpModuleName);
			}
		} else {
			BT_kPrint("Error: No driver to match %s device", pDevice->name);
		}
	}

#ifdef BT_CONFIG_OF

	BT_kPrint("Probing integrated devices from device tree:");

	struct bt_device_node *root = bt_of_find_node_by_path("/");
	struct bt_list_head *pos;
	bt_list_for_each(pos, &root->children) {
		struct bt_device_node *node = (struct bt_device_node *) pos;
		BT_kPrint("devman: %s", node->full_name);
		bt_of_integrated_probe(node);
	}

	BT_kPrint("Completing probing of OF devies");
#endif

	return BT_ERR_NONE;
}
