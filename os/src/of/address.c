#include <bitthunder.h>
#include <of/bt_of.h>
#include <string.h>

#define BT_OF_MAX_ADDR_CELLS	4
#define BT_OF_CHECK_ADDR_COUNT(na)	((na) > 0 && (na) <= BT_OF_MAX_ADDR_CELLS)
#define BT_OF_CHECK_COUNTS(na, ns)	(BT_OF_CHECK_ADDR_COUNT(na) && (ns) > 0)

struct bt_of_bus {
	const char *name;
	const char *addresses;
	BT_BOOL 	(*match)		(struct bt_device_node *parent);
	void 		(*count_cells)	(struct bt_device_node *child, BT_u32 *addrc, BT_u32 *sizec);
	BT_u64		(*map)			(BT_be32 *addr, const BT_be32 *range, BT_u32 na, BT_u32 ns, BT_u32 pna);
	BT_ERROR 	(*translate)	(BT_be32 *addr, BT_u64 offset, BT_u32 na);
	BT_u32 		(*get_flags)	(const BT_be32 *addr);
};

static void default_count_cells(struct bt_device_node *child, BT_u32 *addrc, BT_u32 *sizec) {
	if(addrc) {
		*addrc = bt_of_n_addr_cells(child);
	}

	if(sizec) {
		*sizec = bt_of_n_size_cells(child);
	}
}


static BT_u32 default_get_flags(const BT_be32 *addr) {
	return BT_RESOURCE_MEM;
}

static BT_u64 default_map(BT_be32 *addr, const BT_be32 *range, BT_u32 na, BT_u32 ns, BT_u32 pna) {
	BT_u64 cp, s, da;

	cp 	= bt_of_read_number(range, na);
	s 	= bt_of_read_number(range + na + pna, ns);
	da 	= bt_of_read_number(addr, na);

	if(na > 2 && memcmp(range, addr, na * 4) != 0) {
		return (BT_u64) -1;
	}

	if(da < cp || da >= (cp + s)) {
		return (BT_u64) -1;
	}

	return da - cp;
}

static BT_ERROR default_translate(BT_be32 *addr, BT_u64 offset, BT_u32 na) {
	BT_u64 a = bt_of_read_number(addr, na);
	memset(addr, 0, na * 4);
	a += offset;
	if(na > 1) {
		addr[na - 2] = bt_cpu_to_be32(a >> 32);
	}
	addr[na - 1] = bt_cpu_to_be32(a & 0xFFFFFFFFu);

	return BT_ERR_NONE;
}

static struct bt_of_bus bt_of_busses[] = {
	{
		.name = "default",
		.addresses = "reg",
		.match = NULL,
		.map = default_map,
		.translate = default_translate,
		.count_cells = default_count_cells,
		.get_flags = default_get_flags,
	},
};

static struct bt_of_bus *bt_of_match_bus(struct bt_device_node *node) {
	BT_u32 i;
	for(i = 0; i < BT_ARRAY_SIZE(bt_of_busses); i++) {
		if(!bt_of_busses[i].match || bt_of_busses[i].match(node)) {
			return &bt_of_busses[i];
		}
	}

	return NULL;
}

const BT_be32 *bt_of_get_address(struct bt_device_node *dev, int index, BT_u64 *size, BT_u32 *flags) {

	BT_u32 na, ns, onesize, i;

	struct bt_device_node *parent = dev->parent;
	if(!parent) {
		return NULL;	// A device with an address must be on a bus, and have a parent!
	}

	// Match the bus type ... the compatible name of the parent.
	struct bt_of_bus *bus = bt_of_match_bus(parent);
	bus->count_cells(dev, &na, &ns);

	BT_u32 psize = 0;
	const BT_be32 *prop = bt_of_get_property(dev, bus->addresses, &psize);
	if(!prop) {
		return NULL;
	}

	psize /= 4;
	onesize = na + ns;

	for(i = 0; psize >= onesize; psize -= onesize, prop += onesize, i++) {
		if(i == index) {
			if(size) {
				*size = bt_of_read_number(prop + na, ns);
			}
			if(flags) {
				*flags = bus->get_flags(prop);
			}
			return prop;
		}
	}

	return NULL;
}
BT_EXPORT_SYMBOL(bt_of_get_address);

static int bt_of_translate_one(struct bt_device_node *parent, struct bt_of_bus *bus, struct bt_of_bus *pbus, BT_be32 *addr,
							   BT_u32 na, BT_u32 ns, BT_u32 pna, const char *rprop) {

	const BT_be32 *ranges;
	BT_u32 rlen;
	BT_u32 rone;
	BT_u64 offset = (BT_u64) -1;

	ranges = bt_of_get_property(parent, rprop, &rlen);
	if(!ranges || !rlen) {
		offset = bt_of_read_number(addr, na);
		memset(addr, 0, pna * 4);
		// 1:1 translation -- empty ranges!
		goto finish;
	}

	rlen /= 4;
	rone = na + pna + ns;

	for(; rlen >= rone; rlen -= rone, ranges += rone) {
		offset = bus->map(addr, ranges, na, ns, pna);
		if(offset != (BT_u64) -1) {
			break;
		}
	}

	if(offset == (BT_u64) -1) {
		return 1;
	}

	memcpy(addr, ranges + na, 4 * pna);

finish:
	return pbus->translate(addr, offset, pna);
}
BT_EXPORT_SYMBOL(bt_of_translate_one);

BT_u64 bt_of_translate_address(struct bt_device_node *dev, const BT_be32 *in_addr) {
	struct bt_device_node *parent = NULL;
	struct bt_of_bus *bus, *pbus;
	BT_be32 addr[BT_OF_MAX_ADDR_CELLS];
	BT_u32 na, ns, pna, pns;
	BT_u64 result = (BT_u64) -1;
    const char *rprop = "ranges";

	bt_of_node_get(dev);

	parent = bt_of_get_parent(dev);
	if(!parent) {
		goto bail;
	}

	bus = bt_of_match_bus(parent);
	bus->count_cells(dev, &na, &ns);

	if(!BT_OF_CHECK_COUNTS(na, ns)) {
		// Bad cell count!
		goto bail;
	}

	memcpy(addr, in_addr, na * 4);

	for(;;) {
		bt_of_node_put(dev);
		dev = parent;
		parent = bt_of_get_parent(dev);

		if(!parent) {
			result = bt_of_read_number(addr, na);
			break;
		}

		pbus = bt_of_match_bus(parent);
		pbus->count_cells(dev, &pna, &pns);
		if(!BT_OF_CHECK_COUNTS(pna, pns)) {
			// Bad cell count;
			break;
		}

		// Apply the bus translation
		if(bt_of_translate_one(dev, bus, pbus, addr, na, ns, pna, rprop)) {
			break;
		}

		na = pna;
		ns = pns;
		bus = pbus;
	}

bail:
	bt_of_node_put(parent);
	bt_of_node_put(dev);

	return result;
}
BT_EXPORT_SYMBOL(bt_of_translate_address);

BT_BOOL bt_of_can_translate_address(struct bt_device_node *dev) {
	struct bt_device_node *parent;
	BT_u32 na, ns;

	parent = bt_of_get_parent(dev);
	if(!parent) {
		return BT_FALSE;
	}

	struct bt_of_bus *bus = bt_of_match_bus(parent);
	bus->count_cells(dev, &na, &ns);

	bt_of_node_put(parent);

	return BT_OF_CHECK_COUNTS(na, ns);
}
BT_EXPORT_SYMBOL(bt_of_can_translate_address);

BT_ERROR bt_of_address_to_resource(struct bt_device_node *dev, BT_u32 index, BT_RESOURCE *r) {
	const BT_be32 *addrp;
	BT_u64 size;
	BT_u32 flags;

	addrp = bt_of_get_address(dev, index, &size, &flags);
	if(!addrp) {
		return BT_ERR_GENERIC;
	}

	if(!(flags & (BT_RESOURCE_MEM | BT_RESOURCE_IO))) {
		return BT_ERR_GENERIC;
	}

	BT_u64 taddr = bt_of_translate_address(dev, addrp);
	if(taddr == (BT_u64) (-1)) {
		return BT_ERR_GENERIC;
	}

	memset(r, 0, sizeof(*r));

	r->ulStart 	= taddr;
	r->ulEnd 	= taddr + size - 1;
	r->ulFlags 	= flags;

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(bt_of_address_to_resource);
