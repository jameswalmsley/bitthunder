#ifndef _BT_VM_H_
#define _BT_VM_H_

#include <collections/bt_list.h>

struct bt_segment {
	struct bt_list_head list;
	bt_vaddr_t			addr;
	bt_paddr_t			phys;
	BT_u32				size;
	BT_u32				flags;
	#define				BT_SEG_READ		0x00000001
	#define 			BT_SEG_WRITE	0x00000002
	#define				BT_SEG_EXEC		0x00000004
	#define 			BT_SEG_SHARED	0x00000008
	#define 			BT_SEG_MAPPED	0x00000010
	#define				BT_SEG_FREE		0x80000000
};

struct bt_vm_map {
	struct bt_list_head segments;		///< List of segments.
	BT_u32				refcount;		///< Map reference count.
	bt_pgd_t			pgd;			///< Page directory.
	BT_u32				size;			///< Total size of mapped allocations.
};


/*
 *	Page Attributes for MMU.
 */
#define BT_PAGE_UNMAP	0
#define BT_PAGE_READ	1
#define BT_PAGE_WRITE	2
#define BT_PAGE_SYSTEM	3
#define BT_PAGE_IOMEM	4

struct bt_mmumap {
	bt_vaddr_t         	virt;           /* virtual address */
	bt_paddr_t         	phys;           /* physical address */
	BT_u32				size;           /* size */
	BT_u32             	type;           /* mapping type */
};

/*
 * type of virtual memory mappings
 */
#define BT_VMT_NULL        0
#define BT_VMT_RAM         1
#define BT_VMT_ROM         2
#define BT_VMT_DMA         3
#define BT_VMT_IO          4

void bt_vm_init(void);
struct bt_vm_map *bt_vm_create(void);
void bt_vm_destroy(struct bt_vm_map * map);
struct bt_vm_map *bt_vm_get_kernel_map(void);
bt_paddr_t bt_vm_translate(bt_vaddr_t addr, BT_u32 size);
bt_vaddr_t bt_vm_map_region(struct bt_vm_map *map, bt_paddr_t pa, bt_vaddr_t va, BT_u32 size, BT_u32 type);
void bt_vm_unmap_region(struct bt_vm_map *map, bt_vaddr_t va);

#define BT_VM_ALLOC_ANYWHERE	0x01
BT_ERROR bt_vm_allocate(BT_HANDLE hProcess, void **addr, BT_u32 size, BT_u32 flags);


extern bt_pgd_t bt_mmu_newmap(void);
extern void bt_mmu_switch(bt_pgd_t pgd);
extern bt_paddr_t bt_mmu_extract(bt_pgd_t pgd, bt_vaddr_t, BT_u32 size);
extern void bt_mmu_init(struct bt_mmumap *mmumap);
extern void bt_mmu_killmap(bt_pgd_t pgd);
extern int bt_mmu_map(bt_pgd_t pgd, bt_paddr_t pa, bt_vaddr_t va, BT_u32 size, int type);
extern bt_pgd_t bt_mmu_get_kernel_pgd(void);

#endif
