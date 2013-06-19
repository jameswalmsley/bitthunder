/**
 *	IO / Memory Map Tracking for BitThunder
 *
 **/

#include <bitthunder.h>
#include <mm/bt_mm.h>
#include <mm/bt_ioremap.h>

BT_DEF_MODULE_NAME			("IO/VM Mappings")
BT_DEF_MODULE_DESCRIPTION	("Memory Map Manager")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_IOMAP *g_IOMap = NULL;


static BT_IOMAP *find_mapping(void *phys_addr) {

	if(!g_IOMap) {
		return NULL;
	}

	struct bt_list_head *p;
	BT_IOMAP *item;

	BT_PHYS_ADDR find = (BT_PHYS_ADDR) phys_addr;

	bt_list_for_each(p, &g_IOMap->list) {
		item = (BT_IOMAP *) p;

		if(find >= item->phys && find <= item->phys + item->size) {
			return item;
		}
	}

	return NULL;
}

static BT_IOMAP *bt_iomap_kernel_create(BT_u32 size) {

	BT_IOMAP *pMapping;
	BT_PHYS_ADDR addr = 0xC0000000 + BT_CONFIG_LINKER_RAM_LENGTH;

	// Round-up to nearest MB.
	addr += (addr % 0x00100000);

	if(!g_IOMap) {
		pMapping = BT_kMalloc(sizeof(BT_IOMAP));
		if(!pMapping) {
			return NULL;
		}

		pMapping->addr = (void *) addr;
		pMapping->phys = 0;
		pMapping->size = size;
		pMapping->type = BT_IOMAP_TYPE_KERNEL_MAP;

		BT_LIST_INIT_HEAD(&pMapping->list);

		g_IOMap = pMapping;
		return pMapping;
	}

	struct bt_list_head *pos;
	BT_IOMAP *pItem;
	bt_list_for_each(pos, &g_IOMap->list) {
		pItem = (BT_IOMAP *) pos;
		if((BT_u32) pItem->addr > addr) {
			addr = (BT_u32) pItem->addr;
		}
	}

	addr += 0x00100000;

	pMapping = BT_kMalloc(sizeof(BT_IOMAP));
	if(!pMapping) {
		return NULL;
	}

	pMapping->addr = (void *) addr;
	pMapping->phys = 0;
	pMapping->size = size;
	pMapping->type = BT_IOMAP_TYPE_KERNEL_MAP;

	bt_list_add(&pMapping->list, &g_IOMap->list);

	return pMapping;
}

BT_ERROR bt_iomap_add(void *addr, BT_PHYS_ADDR phys, BT_u32 size, BT_u32 ulType) {
	BT_IOMAP *pMapping = find_mapping((void *) phys);
	if(pMapping) {
		return BT_ERR_GENERIC;
	}

	pMapping = BT_kMalloc(sizeof(BT_IOMAP));
	if(!pMapping) {
		return BT_ERR_GENERIC;
	}

	pMapping->addr = addr;
	pMapping->size = size;
	pMapping->type = ulType;
	pMapping->phys = 0;



	return BT_ERR_NONE;
}

void *bt_ioremap(void *phys_addr, BT_u32 size, BT_ERROR *pError) {

	BT_ERROR Error = BT_ERR_NONE;

	/*
	 *	Find an existing mapping! -- The best case!
	 */

	BT_IOMAP *pMapping = find_mapping(phys_addr);
	if(pMapping) {
		if(pMapping->phys + size >= (BT_PHYS_ADDR) phys_addr * size) {
			BT_PHYS_ADDR offset = ((BT_PHYS_ADDR) phys_addr - pMapping->phys);
			return (void *) ((BT_PHYS_ADDR) (pMapping->addr) + offset);
		}

		// Mapping needs to be extended!
	}

	/*
	 *	Create a new mapping!
	 *	Ask the ARCH MMU api to add an uncached region for us with the desired mapping.
	 */


	BT_PHYS_ADDR phys = (BT_PHYS_ADDR) phys_addr;
	BT_u32 offset = phys & ~BT_SECTION_MASK;

	BT_u32 mapsize = (size / BT_SECTION_SIZE) * BT_SECTION_SIZE;
	if(mapsize & (BT_SECTION_SIZE-1)) {
		mapsize += BT_SECTION_SIZE;
	}

	pMapping = bt_iomap_kernel_create(mapsize);
	if(!pMapping) {
		return BT_ERR_GENERIC;
	}

	pMapping->phys = phys & BT_SECTION_MASK;

	bt_arch_mmu_setsection(pMapping);

	// Get a real address from the MMU!

	return (void *) ((BT_PHYS_ADDR) pMapping->addr + offset);
}


static BT_ERROR bt_map_init() {
	BT_ERROR Error = BT_ERR_NONE;
	return Error;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_map_init,
};
