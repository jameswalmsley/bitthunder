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
	return (void *) bt_vm_map_region(map, (bt_paddr_t) phys, NULL, size, BT_PAGE_IOMEM);
}

void bt_iounmap(void *iomem) {
	struct bt_vm_map *map = bt_vm_get_kernel_map();
	bt_vm_unmap_region(map, (bt_vaddr_t) iomem);
	return;
}
