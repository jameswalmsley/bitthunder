#include <bitthunder.h>
#include <libfdt.h>
#include <bt_kernel.h>
#include <of/bt_of.h>

static struct bt_device_node *bt_of_unflatten_dt_node(const void *fdt, int offset, struct bt_device_node *dad, struct bt_list_head *allitems, BT_u32 fpsize) {

	int has_name = 0;

	// Allocate structure with enough space for the full_name!
	int l, allocl;
	char *pathp = (char *) fdt_get_name(fdt, offset, &l);
	BT_BOOL new_format = BT_FALSE;

	l += 1;
	allocl = l;

	if(*pathp != '/') {
		new_format = BT_TRUE;
		if(!fpsize) {
			fpsize = 1;
			allocl = 2;
			l = 1;
			*pathp = '\0';
		} else {
			fpsize += l;
			allocl = fpsize;
		}
	}

	struct bt_device_node *node = BT_kMalloc(sizeof(*node) + allocl);
	if(!node) {
		return NULL;
	}


	char *fn;
	memset(node, 0, sizeof(*node) + allocl);

	node->full_name = fn = ((char *) node) + sizeof(*node);
	if(new_format) {
		// rebuild fullpath for new format.
		if(dad && dad->parent) {
			strcpy(fn, dad->full_name);
			fn += strlen(fn);
		}
		*(fn++) = '/';
	}

	memcpy(fn, pathp, l);

	node->phandle = fdt_get_phandle(fdt, offset);
	node->node_offset = offset;
	node->parent = dad;

	BT_LIST_INIT_HEAD(&node->children);
	BT_LIST_INIT_HEAD(&node->properties);

	// Process the nodes properties.
	int property_off = fdt_first_property_offset(fdt, offset);
	while(property_off >= 0) {
		int sz = 0;

		const struct fdt_property *prop = fdt_get_property_by_offset(fdt, property_off, &sz);
		BT_u32 noff = bt_be32_to_cpu(prop->nameoff);
		const char *pname = fdt_string(fdt, noff);
		if(!strcmp(pname, "name")) {
			has_name = 1;
		}

		if(!strcmp(pname, "phandle") || !strcmp(pname, "linux,phandle")) {
			if(!node->phandle) {
				BT_be32 *data = (BT_be32 *) prop->data;
				node->phandle = bt_be32_to_cpu(*data);
			}
		}

		struct bt_device_property *property = BT_kMalloc(sizeof(*property));
		if(!property) {
			goto next_property;
		}

		property->name = pname;
		property->length = sz;
		property->value = (void *) &prop->data[0];

		bt_list_add(&property->item, &node->properties);	// Add the property to the node.

	next_property:
		property_off = fdt_next_property_offset(fdt, property_off);
	}

	if(!has_name) {	// If no name property was provided we can create one using the unit name.
		const char *p1 = node->full_name, *ps = node->full_name, *pa = NULL;
		int sz;

		while(*p1) {	// Extract the name part of the fullname
			if((*p1) == '@') {
				pa = p1;
			}
			if((*p1) == '/') {
				ps = p1 + 1;
			}
			p1++;
		}

		if(pa < ps) {
			pa = p1;
		}

		sz = (pa - ps) + 1;

		struct bt_device_property *pp = BT_kMalloc(sizeof(*pp) + sz);
		pp->name = "name";
		pp->length = sz;
		pp->value = pp + 1;
		memcpy(pp->value, ps, sz-1);
		((char *) pp->value)[sz-1] = 0;	// ensure terminated string.
		bt_list_add(&pp->item, &node->properties);
	}

	node->name = bt_of_get_property(node, "name", NULL);
	node->type = bt_of_get_property(node, "device_type", NULL);

	if(!node->name) {
		node->name = "<NULL>";
	}

	if(!node->type) {
		node->type = "<NULL>";
	}

	int subnode = fdt_first_subnode(fdt, offset);
	while(subnode >= 0) {
		struct bt_device_node *child = bt_of_unflatten_dt_node(fdt, subnode, node, allitems, fpsize);
		bt_list_add_tail(&child->item, &node->children);
		bt_list_add_tail(&child->allitem, allitems);
		subnode = fdt_next_subnode(fdt, subnode);
	}

	return node;
}


BT_ERROR bt_of_unflatten_device_tree(const void *fdt, struct bt_list_head *devices, struct bt_list_head *allitems) {
	int node ;

	node = fdt_path_offset(fdt, "/chosen");
	if(node >= 0) {
		const struct fdt_property *prop = fdt_get_property(fdt, node, "bootargs", NULL);
		if(prop) {
			bt_kernel_params *params = bt_get_kernel_params();
			params->cmdline = &prop->data[0];
			BT_kPrint("cmdline: %s", params->cmdline);
		}
	}

	node = fdt_path_offset(fdt, "/");	// Get the root node.
	struct bt_device_node *dev = bt_of_unflatten_dt_node(fdt, node, NULL, allitems, 0);
	bt_list_add(&dev->item, devices);
	bt_list_add(&dev->allitem, allitems);

	return BT_ERR_NONE;
}

BT_ERROR bt_of_init() {

	bt_kernel_params *params = bt_get_kernel_params();

	void *virt_fdt = (void *) bt_phys_to_virt((bt_paddr_t) params->fdt);	// If its within kernel memory.

	if(fdt_check_header(virt_fdt)) {
		BT_kPrint("fdt header failed");
		return BT_ERR_GENERIC;
	}

	params->fdt = virt_fdt;
	// Place a pointer to the kernel commandline from the fdt into the kernel_params structure.


	BT_LIST_INIT_HEAD(&params->all_of_nodes);
	BT_LIST_INIT_HEAD(&params->devices);

	bt_of_unflatten_device_tree(virt_fdt, &params->devices, &params->all_of_nodes);

	bt_of_integrated_populate((struct bt_device_node *) params->devices.next);

	return BT_ERR_NONE;
}
