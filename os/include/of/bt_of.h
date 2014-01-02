#ifndef _BT_OF_H_
#define _BT_OF_H_

#include "bt_of_irq.h"

#define BT_OF_ROOT_NODE_ADDR_CELLS_DEFAULT 1
#define BT_OF_ROOT_NODE_SIZE_CELLS_DEFAULT 1

const void *bt_of_get_fdt();

struct bt_device_node *bt_of_node_get(struct bt_device_node *node);
void bt_of_node_put(struct bt_device_node *node);

struct bt_device_node *bt_of_get_parent(const struct bt_device_node *node);

struct bt_device_property *bt_of_find_property(const struct bt_device_node *np, const BT_i8 *name, BT_u32 *lenp);
const void *bt_of_get_property(const struct bt_device_node *np, const BT_i8 *name, BT_u32 *lenp);

BT_u32 bt_of_n_addr_cells(struct bt_device_node *np);
BT_u32 bt_of_n_size_cells(struct bt_device_node *np);

BT_BOOL bt_of_can_translate_address(struct bt_device_node *dev);
BT_ERROR bt_of_address_to_resource(struct bt_device_node *dev, BT_u32 index, BT_RESOURCE *r);

const BT_be32 *bt_of_get_address(struct bt_device_node *dev, int index, BT_u64 *size, BT_u32 *flags);

BT_BOOL bt_of_is_compatible(struct bt_device_node *device, const BT_i8 *compat);

struct bt_device_node *bt_of_find_node_by_path(const BT_i8 *path);
struct bt_device_node *bt_of_find_node_by_phandle(BT_u32 phandle);

BT_ERROR bt_of_integrated_probe(struct bt_device_node *node);


BT_ERROR bt_of_integrated_populate(struct bt_device_node *root);
BT_ERROR bt_of_integrated_populate_device(struct bt_device_node *device);

BT_ERROR bt_of_unflatten_device_tree(const void *fdt, struct bt_list_head *devices, struct bt_list_head *allitems);
BT_ERROR bt_of_init();

static inline BT_u64 bt_of_read_number(const BT_be32 *cell, BT_u32 size) {
	BT_u64 r = 0;
	while (size--)
		r = (r << 32) | bt_be32_to_cpu(*(cell++));
	return r;
}

struct bt_device_node *bt_of_get_bootlogger();
struct bt_device_node *bt_of_integrated_get_node(const BT_DEVICE *device);

BT_ERROR bt_of_i2c_populate_device(struct bt_device_node *device);
BT_ERROR bt_of_spi_populate_device(struct bt_device_node *device);

const void *bt_of_get_mac_address(struct bt_device_node *device);

#endif
