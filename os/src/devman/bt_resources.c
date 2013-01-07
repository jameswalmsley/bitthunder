/**
 *	Resource helper functions.
 *
 **/

#include <bitthunder.h>

//BT_RESOURCE *BT_GetResource(BT_RESOURCE *pResources,


const BT_RESOURCE *BT_GetIntegratedResource(const BT_INTEGRATED_DEVICE *pDevice, BT_u32 ulType, BT_u32 ulNum) {
	BT_u32 i;
	for(i = 0; i < pDevice->ulTotalResources; i++) {
		const BT_RESOURCE *pResource = &pDevice->pResources[i];
		if(BT_RESOURCE_TYPE(pResource->ulFlags) == BT_RESOURCE_TYPE(ulType) && ulNum-- == 0) {
			return pResource;
		}
	}

	return NULL;
}
