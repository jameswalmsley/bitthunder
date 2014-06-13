/**
 *	Resource helper functions.
 *
 **/

#include <bitthunder.h>

const BT_RESOURCE *BT_GetResource(const BT_RESOURCE *pResources, BT_u32 ulTotalResources, BT_u32 ulType, BT_u32 ulNum) {
	BT_u32 i;
	for(i = 0; i < ulTotalResources; i++) {
		const BT_RESOURCE *pResource = &pResources[i];
		if(BT_RESOURCE_TYPE(pResource->ulFlags) == BT_RESOURCE_TYPE(ulType) && ulNum-- == 0) {
			return pResource;
		}
	}

	return NULL;
}
BT_EXPORT_SYMBOL(BT_GetResource);
