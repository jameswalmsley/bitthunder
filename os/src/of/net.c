#include <bitthunder.h>
#include <of/bt_of.h>

const void *bt_of_get_mac_address(struct bt_device_node *device) {
	struct bt_device_property *property;
	property = bt_of_find_property(device, "mac-address", NULL);
	if(!property) {
		property = bt_of_find_property(device, "local-mac-address", NULL);
	}
	if(!property) {
		property = bt_of_find_property(device, "address", NULL);
	}

	if(property && property->length == 6) {
		return property->value;
	}

	return NULL;
}
BT_EXPORT_SYMBOL(bt_of_get_mac_address);
