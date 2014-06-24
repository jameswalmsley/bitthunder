#include <bitthunder.h>
#include <of/bt_of.h>

struct bt_device_node *bt_of_get_bootlogger() {
	struct bt_device_node *root;
	struct bt_device_node *p;
	const BT_be32 *phandle_logger;

	root = bt_of_find_node_by_path("/");
	if(!root) {
		return NULL;
	}

	phandle_logger = bt_of_get_property(root, "boot-logger", NULL);
	if(!phandle_logger) {
		return NULL;
	}

	p = bt_of_find_node_by_phandle(bt_be32_to_cpu(*((BT_u32 *) (phandle_logger))));
	if(!p) {
		return NULL;
	}

	return p;
}
BT_EXPORT_SYMBOL(bt_of_get_bootlogger);
