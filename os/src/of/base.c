#include <bitthunder.h>
#include <libfdt.h>
#include <of/bt_of.h>

const void *bt_of_get_fdt() {
	bt_kernel_params *params = bt_get_kernel_params();
	return params->fdt;
}

struct bt_device_node *bt_of_node_get(struct bt_device_node *node) {
	if(node) {
		// increment the ref count.
	}
	return node;
}

void bt_of_node_put(struct bt_device_node *node) {
	if(node) {
		// decrement the ref count
	}
}

static struct bt_device_property *__bt_of_find_property(const struct bt_device_node *np, const BT_i8 *name, BT_u32 *lenp) {
	struct bt_list_head *pos;

	bt_list_for_each(pos, &np->properties) {
		struct bt_device_property *property = (struct bt_device_property *) pos;
		if(!strcmp(property->name, name)) {
			*lenp = property->length;
			return property;
		}
	}

	return NULL;
}

struct bt_device_property *bt_of_find_property(const struct bt_device_node *np, const BT_i8 *name, BT_u32 *lenp) {
	return __bt_of_find_property(np, name, lenp);
}

const void *bt_of_get_property(const struct bt_device_node *np, const BT_i8 *name, BT_u32 *lenp) {
	BT_u32 len;
	struct bt_device_property *property = bt_of_find_property(np, name, &len);
	if(lenp) {
		*lenp = len;
	}
	return property ? property->value : NULL;
}

BT_u32 bt_of_n_addr_cells(struct bt_device_node *np) {

	const struct BT_be32 *prop;
	do {
		if(np->parent) {
			np = np->parent;
		}

		prop = bt_of_get_property(np, "#address-cells", NULL);
		if(prop) {
			return bt_be32_to_cpu(*((BT_u32 *) (prop)));
		}

	} while(np->parent);

	return BT_OF_ROOT_NODE_ADDR_CELLS_DEFAULT;
}

BT_u32 bt_of_n_size_cells(struct bt_device_node *np) {

	const void *prop = NULL;
	do {
		if(np->parent) {
			np = np->parent;
		}

		prop = bt_of_get_property(np, "#size-cells", NULL);
		if(prop) {
			return bt_be32_to_cpu(*((BT_u32 *) (prop)));
		}

	} while(np->parent);

	return BT_OF_ROOT_NODE_SIZE_CELLS_DEFAULT;
}

struct bt_device_node *bt_of_get_parent(const struct bt_device_node *node) {
	struct bt_device_node *np;

	if(!node) {
		return NULL;
	}

	np = bt_of_node_get(node->parent);

	return np;
}

BT_BOOL bt_of_is_compatible(struct bt_device_node *device, const BT_i8 *compat) {
	const char *cp;
	BT_u32 cplen, l;

	cp = bt_of_get_property(device, "compatible", &cplen);
	if(!cp) {
		return 0;
	}

	BT_u32 compatlen = strlen(compat);

	while(cplen > 0) {
		if(!strncmp(cp, compat, compatlen)) {
			return 1;
		}

		l = strlen(cp) + 1;
		cp += l;
		cplen -= l;
	}

	return 0;
}


struct bt_device_node *bt_of_find_node_by_path(const BT_i8 *path) {

	bt_kernel_params *params = bt_get_kernel_params();
	struct bt_device_node *node = NULL;

	struct bt_list_head *pos;
	bt_list_for_each(pos, &params->all_of_nodes) {
		node = (struct bt_device_node *) bt_container_of(pos, struct bt_device_node, allitem);
		if(node->full_name && !strcmp(node->full_name, path) && bt_of_node_get(node)) {
			break;
		}
	}

	return node;
}

struct bt_device_node *bt_of_find_node_by_phandle(BT_u32 phandle) {

	bt_kernel_params *params = bt_get_kernel_params();
	struct bt_device_node *node = NULL;

	struct bt_list_head *pos;
	bt_list_for_each(pos, &params->all_of_nodes) {
		node = (struct bt_device_node *) bt_container_of(pos, struct bt_device_node, allitem);
		if(node->phandle == phandle) {
			return node;
		}
	}

	return NULL;
}
