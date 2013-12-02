#include <bitthunder.h>
#include <of/bt_of.h>

BT_ERROR bt_of_i2c_populate_device(struct bt_device_node *device) {

	BT_RESOURCE *res;
	BT_u32 len;

	if(device->ulFlags & BT_DEVICE_POPULATED) {
		return BT_ERR_NONE;
	}

	const BT_be32 *paddr = bt_of_get_property(device, "reg", &len);
	if(len != 4) {
		return BT_ERR_INVALID_VALUE;
	}

	device->dev.ulTotalResources = 1;
	device->dev.pResources = BT_kMalloc(sizeof(BT_RESOURCE) * device->dev.ulTotalResources);
	device->dev.name = bt_of_get_property(device, "compatible", NULL);

	res = (BT_RESOURCE *) device->dev.pResources;

	res->ulStart 	= bt_be32_to_cpu(*paddr);
	res->ulEnd 		= device->dev.pResources->ulStart;
	res->ulFlags 	= BT_RESOURCE_ADDR;

	device->dev.eType = BT_DEVICE_I2C | BT_DEVICE_TYPE_OF_FLAG;
	device->ulFlags |= BT_DEVICE_POPULATED;

	return BT_ERR_NONE;
}
