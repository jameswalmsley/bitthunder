#include <bitthunder.h>
#include <collections/bt_list.h>
#include <of/bt_of.h>
#include <string.h>

struct bt_device_node *bt_of_mdio_get_node(struct bt_device_node *device) {
	struct bt_list_head *pos;
	bt_list_for_each(pos, &device->children) {
		struct bt_device_node *node = bt_container_of(pos, struct bt_device_node, item);
		if(!strcmp(node->name, "mdio")) {
			return node;
		}
	}

	return NULL;
}


BT_ERROR bt_of_mdio_populate_device(struct bt_device_node *device) {

	device->dev.eType = BT_DEVICE_BUS | BT_DEVICE_TYPE_OF_FLAG;
	device->ulFlags |= BT_DEVICE_POPULATED;
	device->dev.name = device->name;

	struct bt_list_head *pos;
	bt_list_for_each(pos, &device->children) {
		struct bt_device_node *phy = bt_container_of(pos, struct bt_device_node, item);
		phy->dev.name = bt_of_get_property(device, "compatible", NULL);
		phy->dev.eType = BT_DEVICE_MDIO | BT_DEVICE_TYPE_OF_FLAG;
		phy->ulFlags |= BT_DEVICE_POPULATED;
	}

	return BT_ERR_NONE;
}
