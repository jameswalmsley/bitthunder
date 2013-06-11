/**
 *
 *
 **/

#include <bitthunder.h>
#include <string.h>
#include <stdio.h>

extern const BT_DEVICE * __bt_devices_start;
extern const BT_DEVICE * __bt_devices_end;

BT_u32 BT_GetTotalDevicesByType(BT_DEVICE_TYPE eType) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_devices_end - (BT_u32) &__bt_devices_start);
	BT_u32 i=0;
	BT_u32 ulDevices=0;

	size /= sizeof(BT_DEVICE);

	BT_DEVICE *pDevice = (BT_DEVICE *) &__bt_devices_start;

	for(i = 0; i < size; i++) {
		if(pDevice->eType == eType) {
			ulDevices++;
		}

		pDevice++;
	}

	return ulDevices;
}

const BT_DEVICE *BT_GetDeviceByType(BT_DEVICE_TYPE eType, BT_u32 ulIndex) {
	BT_u32 size = (BT_u32) ((BT_u32) &__bt_devices_end - (BT_u32) &__bt_devices_start);
	BT_u32 i=0;
	BT_u32 ulDevices=0;

	size /= sizeof(BT_DEVICE);

	const BT_DEVICE *pDevice = (BT_DEVICE *) &__bt_devices_start;

	for(i = 0; i < size; i++) {
		if(pDevice->eType == eType) {
			if(ulDevices == ulIndex) {
				return pDevice;
			}

			ulDevices++;
		}

		pDevice++;
	}

	return NULL;
}

const BT_RESOURCE *BT_GetDeviceResource(const BT_DEVICE *pDevice, BT_u32 ulType, BT_u32 ulNum) {
	return BT_GetResource(pDevice->pResources, pDevice->ulTotalResources, ulType, ulNum);
}
