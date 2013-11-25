/**
 *	Support for Integrated devices from device tree.
 *
 **/

#include <bitthunder.h>
#include <libfdt.h>
#include <of/bt_of.h>

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

static BT_HANDLE devfs_open(struct bt_devfs_node *node, BT_ERROR *pError) {
	BT_ERROR Error = BT_ERR_NONE;
	struct bt_device_node *device = bt_container_of(node, struct bt_device_node, devfs_node);
	BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(device->dev.name);
	if(pDriver && (pDriver->eType & BT_DRIVER_TYPE_CODE_MASK) == BT_DRIVER_INTEGRATED) {
		BT_HANDLE hDevice = pDriver->pfnProbe(&device->dev, &Error);
		*pError = Error;
		return hDevice;
	}

	*pError = BT_ERR_GENERIC;
	return NULL;
}

static const BT_DEVFS_OPS oDevfsOps = {
	.pfnOpen = devfs_open,
};

static BT_ERROR bt_of_integrated_device_probe(struct bt_device_node *bus) {
	struct bt_list_head *pos;

	bt_list_for_each(pos, &bus->children) {
		struct bt_device_node *device = (struct bt_device_node *) pos;
		if(!(device->ulFlags & BT_DEVICE_POPULATED)) {
			continue;
		}

		//BT_kPrint("Probing: %s (%s)", device->full_name, device->dev.name);
		BT_INTEGRATED_DRIVER *pDriver = BT_GetIntegratedDriverByName(device->dev.name);
		if(pDriver && (pDriver->eType & BT_DRIVER_TYPE_CODE_MASK) == BT_DRIVER_INTEGRATED) {
			BT_ERROR Error = BT_ERR_NONE;
			if(pDriver->eType & BT_DRIVER_DEVFS_PROBE) {
				char *devname = BT_kMalloc(strlen(device->type) + 10);
				BT_u32 psize = 0;
				const BT_be32 *port = bt_of_get_property(device, "port-number", &psize);
				BT_u32 port_num = bt_of_read_number(port, psize/4);
				bt_sprintf(devname, "%s%d", device->name, port_num);
				device->devfs_node.pOps = &oDevfsOps;

				BT_DeviceRegister(&device->devfs_node, devname);

				// Create a devfs node!
			} else {
				BT_HANDLE hDevice = pDriver->pfnProbe(&device->dev, &Error);
				if(Error) {
					BT_kPrint("Error probing: %s device,", device->dev.name);
				}

				if(hDevice) {
					BT_kPrint("Registered %s with %s driver", device->dev.name, hDevice->h.pIf->oInfo.szpModuleName);
				}
			}
		}
	}

	BT_kPrint("Completed simple-bus probe");

	return BT_ERR_NONE;
}

BT_ERROR bt_of_integrated_probe(struct bt_device_node *node) {
	if(bt_of_is_compatible(node, "simple-bus")) {	// Its a bus, so probe its children
		// probe the devices
		BT_kPrint("Probing a simple-bus");
		bt_of_integrated_device_probe(node);
	} else {
		BT_kPrint("Recursing down the tree in search of simple-bus");
		// Are any of the nodes children simple-bus devices?
		struct bt_list_head *pos;
		bt_list_for_each(pos, &node->children) {
			struct bt_device_node *child = (struct bt_device_node *) pos;
			BT_kPrint(":: %s :: recursing for node", child->full_name);
			bt_of_integrated_probe(child);	// Recursively ensure we probe the deepest simple-bus.
			BT_kPrint(":: %s :: completed recursion for node", child->full_name);
		}
	}

    BT_kPrint("Done integrated of probe");

	return BT_ERR_NONE;
}

BT_ERROR bt_of_integrated_populate_device(struct bt_device_node *device) {
	BT_u32 num_reg = 0, num_irq = 0;
	BT_RESOURCE *res, temp_res;

	if(bt_of_can_translate_address(device)) {
		while(!bt_of_address_to_resource(device, num_reg, &temp_res)) {
			num_reg++;
		}
	}

	num_irq = bt_of_irq_count(device);

	device->dev.eType = BT_DEVICE_INTEGRATED | BT_DEVICE_TYPE_OF_FLAG;
	device->ulFlags |= BT_DEVICE_POPULATED;

	// Generate the resource table.
	if(num_reg || num_irq) {
		res = BT_kMalloc(sizeof(*res) * (num_irq + num_reg));
		if(!res) {
			device->ulFlags &= ~BT_DEVICE_POPULATED;	// Ensure device is not marked as populated, to prevent probing.
			return BT_ERR_GENERIC;
		}

		device->dev.ulTotalResources = (num_irq + num_reg);
		device->dev.pResources = res;
		device->dev.name = bt_of_get_property(device, "compatible", NULL);

		BT_u32 i;
		for(i = 0; i < num_reg; i++, res++) {
			bt_of_address_to_resource(device, i, res);
		}

		// irq's to resources.
		bt_of_irq_to_resource_table(device, res, num_irq);
	}

	return BT_ERR_NONE;
}


BT_ERROR bt_of_integrated_populate_bus(struct bt_device_node *bus) {

	struct bt_list_head *pos;
	bt_list_for_each(pos, &bus->children) {
		struct bt_device_node *child = (struct bt_device_node *) pos;
		if(!(child->ulFlags & BT_DEVICE_POPULATED)) {
			bt_of_integrated_populate_device(child);
		}
		BT_kPrint("Found %s", child->name);
	}

	return BT_ERR_NONE;
}

BT_ERROR bt_of_integrated_populate(struct bt_device_node *root) {

	struct bt_list_head *pos;
	bt_list_for_each(pos, &root->children) {
		struct bt_device_node *child = (struct bt_device_node *) pos;
		if(bt_of_is_compatible(child, "simple-bus")) {
			bt_of_integrated_populate_bus(child);
		}
	}

	return BT_ERR_NONE;
}
