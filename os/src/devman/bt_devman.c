/**
 *	BitThunder Device Manager
 *
 *
 **/


#include <bitthunder.h>
#include <string.h>
#include <stdio.h>

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


BT_ERROR BT_ProbeIntegratedDevices(BT_HANDLE hLogDevice) {

	BT_ERROR Error;
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_arch_devices_end - (BT_u32) &__bt_arch_devices_start);
	BT_u32 i;

	size /= sizeof(BT_INTEGRATED_DEVICE);

	for(i = 0; i < size; i++) {
		BT_INTEGRATED_DEVICE *pDevice = (BT_INTEGRATED_DEVICE *) &__bt_arch_devices_start;
		pDevice += i;

		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(pDevice->name);
		if(pDriver) {
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

	return BT_ERR_NONE;
}
