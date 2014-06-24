#include <bitthunder.h>
#include <of/bt_of.h>


struct bt_device_node *bt_of_irq_find_parent(struct bt_device_node *child) {

	struct bt_device_node *p;
	const BT_be32 *phandle_intc;

	if(!bt_of_node_get(child)) {
		return NULL;
	}

	do {
		phandle_intc = bt_of_get_property(child, "interrupt-parent", NULL);
		if(!phandle_intc) {
			p = bt_of_get_parent(child);
		} else {
			p = bt_of_find_node_by_phandle(bt_be32_to_cpu(*((BT_u32 *) (phandle_intc))));
		}

		bt_of_node_put(child);
		child = p;
	} while(p && !bt_of_get_property(p, "#interrupt-cells", NULL));

	return p;
}
BT_EXPORT_SYMBOL(bt_of_irq_find_parent);

BT_ERROR bt_of_irq_map_raw(struct bt_device_node *parent, const BT_be32 *intspec, BT_u32 ointsize,
						   const BT_be32 *addr, struct bt_of_irq *out_irq) {

	struct bt_device_node *ipar, *tnode, *old = NULL, *newpar = NULL;
	const BT_be32 *tmp, *imap, *imask;
	BT_u32 intsize = 1, addrsize, newintsize = 0, newaddrsize = 0;
	BT_u32 imaplen, match, i;

	ipar = bt_of_node_get(parent);

	do {
		tmp = bt_of_get_property(ipar, "#interrupt-cells", NULL);
		if(tmp) {
			intsize = bt_be32_to_cpu(*tmp);
			break;
		}
		tnode = ipar;
		ipar = bt_of_irq_find_parent(ipar);
		bt_of_node_put(tnode);
	} while(ipar);

	if(!ipar) {
		goto fail;
	}

	if(ointsize != intsize) {
		return BT_ERR_GENERIC;
	}

	old = bt_of_node_get(ipar);
	do {
		tmp = bt_of_get_property(old, "#address-cells", NULL);
		tnode = bt_of_get_parent(old);
		bt_of_node_put(old);
		old = tnode;
	} while(old && !tmp);

	bt_of_node_put(old);

	old = NULL;

	addrsize = (!tmp) ? 2 : bt_be32_to_cpu(*tmp);

	while(ipar) {
		if(bt_of_get_property(ipar, "interrupt-controller", NULL)) {
			for(i = 0; i < intsize; i++) {
				out_irq->specifier[i] = bt_of_read_number(intspec + i, 1);
				out_irq->size = intsize;
				out_irq->controller = ipar;
				bt_of_node_put(old);
				return BT_ERR_NONE;
			}
		}

		// look for interrupt map and parse.

		imap = bt_of_get_property(ipar, "interrupt-map", &imaplen);
		if(!imap) {
			newpar = bt_of_irq_find_parent(ipar);
			goto skiplevel;
		}

		imaplen /= sizeof(BT_u32);

		imask = bt_of_get_property(ipar, "interrupt-map-mask", NULL);

		if(!addr && addrsize != 0) {
			goto fail;
		}

		match = 0;
		while(imaplen > (addrsize + intsize + 1) && !match) {
			match = 1;
			for(i = 0; i < addrsize && match; ++i) {
				BT_be32 mask = imask ? imask[i] : bt_cpu_to_be32(0xffffffffu);
				match = ((addr[i] ^ imap[i]) & mask) == 0;
			}

			for(; i < (addrsize + intsize) && match; ++i) {
				BT_be32 mask = imask ? imask[i] : bt_cpu_to_be32(0xffffffffu);
				match = ((intspec[i-addrsize] ^ imap[i]) & mask) == 0;
			}

			imap += addrsize + intsize;
			imaplen -= addrsize + intsize;

			// Get the int parent.
			newpar = bt_of_find_node_by_phandle(bt_be32_to_cpu(*imap));
			imap++;
			--imaplen;

			if(!newpar) {
				goto fail;
			}

			tmp = bt_of_get_property(newpar, "#interrupt-cells", NULL);
			if(!tmp) {
				goto fail;
			}

			newintsize = bt_be32_to_cpu(*tmp);
			tmp = bt_of_get_property(newpar, "#address-cells", NULL);
			newaddrsize = (!tmp) ? 0 : bt_be32_to_cpu(*tmp);

			if(imaplen < (newaddrsize + newintsize)) {
				goto fail;
			}

			imap += newaddrsize + newintsize;
			imaplen -= newaddrsize + newintsize;
		}

		if(!match) {
			goto fail;
		}

		bt_of_node_put(old);
		old = bt_of_node_get(newpar);
		addrsize = newaddrsize;
		intsize = newintsize;
		intspec = imap - intsize;
		addr = intspec - addrsize;

	skiplevel:
		bt_of_node_put(ipar);
		ipar = newpar;
		newpar = NULL;
	}


fail:
	bt_of_node_put(ipar);
	bt_of_node_put(old);
	bt_of_node_put(newpar);

	return BT_ERR_GENERIC;
}
BT_EXPORT_SYMBOL(bt_of_irq_map_raw);

BT_ERROR bt_of_irq_map_one(struct bt_device_node *device, BT_u32 index, struct bt_of_irq *out_irq) {
	BT_u32 intlen, intsize;
	const BT_be32 *intspec, *tmp, *addr;
	struct bt_device_node *p;


	intspec = bt_of_get_property(device, "interrupts", &intlen);
	if(!intspec) {
		return BT_ERR_GENERIC;
	}

	intlen /= sizeof(*intspec);

	addr = bt_of_get_property(device, "reg", NULL);

	p = bt_of_irq_find_parent(device);
	if(!p) {
		return BT_ERR_GENERIC;
	}

	tmp = bt_of_get_property(p, "#interrupt-cells", NULL);
	if(!tmp) {
		return BT_ERR_GENERIC;
	}

	intsize = bt_be32_to_cpu(*tmp);

	if((index + 1) * intsize > intlen) {
		return BT_ERR_GENERIC;
	}

	BT_ERROR retval = bt_of_irq_map_raw(p, intspec + index * intsize, intsize, addr, out_irq);

	bt_of_node_put(p);

	return retval;
}
BT_EXPORT_SYMBOL(bt_of_irq_map_one);

BT_u32 bt_of_irq_parse_and_map(struct bt_device_node *device, BT_u32 index) {
	struct bt_of_irq oirq;

	if(bt_of_irq_map_one(device, index, &oirq)) {
		return 0;
	}

	return oirq.specifier[0];
}
BT_EXPORT_SYMBOL(bt_of_irq_parse_and_map);

BT_u32 bt_of_irq_to_resource(struct bt_device_node *dev, BT_u32 index, BT_RESOURCE *r) {
	BT_u32 irq = bt_of_irq_parse_and_map(dev, index);

	if(r && irq) {
		r->ulStart = r->ulEnd = irq;
		r->ulFlags = BT_RESOURCE_IRQ;
	}

	return irq;
}
BT_EXPORT_SYMBOL(bt_of_irq_to_resource);

BT_u32 bt_of_irq_count(struct bt_device_node *dev) {
	BT_u32 nr = 0;

	while(bt_of_irq_to_resource(dev, nr, NULL)) {
		nr++;
	}

	return nr;
}
BT_EXPORT_SYMBOL(bt_of_irq_count);

BT_u32 bt_of_irq_to_resource_table(struct bt_device_node *device, BT_RESOURCE *r, BT_u32 nr_irqs) {
	BT_u32 i;

	for(i = 0; i < nr_irqs; i++, r++) {
		if(!bt_of_irq_to_resource(device, i, r)) {
			break;
		}
	}

	return i;
}
BT_EXPORT_SYMBOL(bt_of_irq_to_resource_table);
