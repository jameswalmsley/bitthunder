/**
 *	IO / Memory Map Tracking for BitThunder
 *
 **/

#include <bitthunder.h>
#include <mm/bt_mm.h>
#include <mm/bt_vm.h>
#include <mm/bt_ioremap.h>

BT_DEF_MODULE_NAME			("IO/VM Mappings")
BT_DEF_MODULE_DESCRIPTION	("Memory Map Manager")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

void *bt_ioremap(void *phys, BT_u32 size) {
	struct bt_vm_map *map = bt_vm_get_kernel_map();
	BT_u32 diff = (bt_paddr_t) phys - BT_PAGE_TRUNC((bt_paddr_t) phys);

	bt_vaddr_t mapping = bt_vm_map_region(map, (bt_paddr_t) BT_PAGE_TRUNC((bt_paddr_t) phys), 0, size+diff, BT_PAGE_IOMEM);

	return (void *) mapping + diff;
}

void bt_iounmap(volatile void *iomem) {
	struct bt_vm_map *map = bt_vm_get_kernel_map();
	bt_vm_unmap_region(map, (bt_vaddr_t) iomem);
	return;
}
