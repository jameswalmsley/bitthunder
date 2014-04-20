/**
 *	@brief	  	Virtual Memory Manager
 *
 *	Provides the kernel with a comprehensive API to manage virtual memory mappings.
 *	The API is used within the kernel to allocate pages, and map them into various regions
 *	of a process's virtual memory space.
 *
 *	The API can also be used to share virtual memory mappings between processes, e.g. a
 *	shared memory region etc.
 *	
 *	@file		bt_vm.h
 *	@author		James Walmsley <james@fullfat-fs.co.uk>
 *	@ingroup	MM
 *
 *	@copyright	(c) 2013 James Walmsley
 **/
#ifndef _BT_VM_H_
#define _BT_VM_H_

#include <collections/bt_list.h>

/**
 *	@brief	Used to describe a segment (region) of a virtual memory space.
 *
 **/
struct bt_segment {
	struct bt_list_head list;
	struct bt_list_head	shared_list;
	bt_vaddr_t			addr;
	bt_paddr_t			phys;
	BT_u32				size;
	BT_u32				flags;
	#define				BT_SEG_READ		0x00000001
	#define 			BT_SEG_WRITE	0x00000002
	#define				BT_SEG_EXEC		0x00000004
	#define 			BT_SEG_SHARED	0x00000008
	#define 			BT_SEG_MAPPED	0x00000010
	#define 			BT_SEG_IOMAPPED	0x00000020
	#define				BT_SEG_FREE		0x80000000
};

/**
 *	@brief	Used to describe a Virtual Memory Map.
 *
 **/
struct bt_vm_map {
	struct bt_list_head segments;		///< List of segments.
	BT_u32				refcount;		///< Map reference count.
	bt_pgd_t			pgd;			///< Page directory.
	BT_u32				size;			///< Total size of mapped allocations.
	void 			   *map_mutex;
};


/*
 *	Page Attributes for MMU.
 */
#define BT_PAGE_UNMAP	0				///< Page should be unmapped.
#define BT_PAGE_READ	1				///< Page should be made read-only.
#define BT_PAGE_WRITE	2				///< Page should be made read/writable.
#define BT_PAGE_SYSTEM	3				///< System page with full permissions.
#define BT_PAGE_IOMEM	4				///< System page with no caching enabled.

#define BT_PROT_NONE	0				///< Pages cannot be accessed
#define BT_PROT_READ	1				///< Pages can be read
#define BT_PROT_WRITE	2				///< Pages can be written
#define BT_PROT_EXEC	4				///< Pages can be executed

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

/**
 *	@kernel
 *	@private
 *	@brief	Initialises the Virtual Memory Manager
 *
 **/
void bt_vm_init(void);

/**
 *	@kernel
 *	@public
 *	@brief	Creates a virtual memory map.
 *
 **/
struct bt_vm_map *bt_vm_create(void);

/**
 *	@kernel
 *	@public
 *	@brief	Destroy's a virtual memory map, and free's any allocated pages back to the page allocator.
 *
 **/
void bt_vm_destroy(struct bt_vm_map * map);

/**
 *	@kernel
 *	@public
 *	@brief	Get's a pointer to the kernel's virtual memory map.
 *
 **/
struct bt_vm_map *bt_vm_get_kernel_map(void);

/**
 *	@kernel
 *	@public
 *	@brief	Translates the address of the currently active VM map in the MMU.
 *
 *	@note This uses the MMU layer to make the translation, and so this API can only translate the active region.
 **/
bt_paddr_t bt_vm_translate(bt_vaddr_t addr, BT_u32 size);

/**
 *	@kernel
 *	@public
 *	@brief	Maps a region of memory into the specified virtual memory map.
 *
 **/
bt_vaddr_t bt_vm_map_region(struct bt_vm_map *map, bt_paddr_t pa, bt_vaddr_t va, BT_u32 size, BT_u32 type);

/**
 *	@kernel
 *	@public
 *	@brief	Unmaps a region of memory from the specified virtual memory map.
 *
 **/
void bt_vm_unmap_region(struct bt_vm_map *map, bt_vaddr_t va);

#define BT_VM_ALLOC_ANYWHERE	0x01

/**
 *	@kernel
 *	@public
 *	@brief	Allocates some memory into the specified task's virtual memory map.
 *
 **/
BT_ERROR bt_vm_allocate(struct bt_task *task, void **addr, BT_u32 size, BT_u32 flags);

/**
 *	@kernel
 *	@public
 *	@brief	Free's some memory from the specified task's virtual memory map.
 *
 **/
BT_ERROR bt_vm_free(struct bt_task *task, void *addr);

/**
 *	@kernel
 *	@private
 *	@brief	Create a new PGD map with the MMU driver.
 *
 **/
extern bt_pgd_t bt_mmu_newmap(void);

/**
 *	@kernel
 *	@private
 *	@brief	Kills/cleanup a PGD map from the MMU driver.
 *
 **/
extern void bt_mmu_terminate(bt_pgd_t pgd);

/**
 *	@kernel
 *	@private
 *	@brief	Switch the MMU's active memory map to the specified PGD.
 *
 **/
extern void bt_mmu_switch(bt_pgd_t pgd);

/**
 *	@kernel
 *	@private
 *	@brief	Flush the MMUs translation-lookaside buffers.
 *
 **/
extern void bt_mmu_flush_tlb(void);

/**
 *	@kernel
 *	@private
 *	@brief	Translate a virtual address to a physical address using the specified PGD.
 *
 **/
extern bt_paddr_t bt_mmu_extract(bt_pgd_t pgd, bt_vaddr_t, BT_u32 size);

/**
 *	@kernel
 *	@private
 *	@brief	Initialise the MMU driver for the system.
 *
 **/
extern void bt_mmu_init(struct bt_mmumap *mmumap);

/**
 *
 *
 **/
extern void bt_mmu_killmap(bt_pgd_t pgd);

/**
 *	@kernel
 *	@private
 *	@brief	Get the MMU driver to apply a virtual memory mapping on the specified PGD.
 *
 **/
extern int bt_mmu_map(bt_pgd_t pgd, bt_paddr_t pa, bt_vaddr_t va, BT_u32 size, int type);


extern bt_pgd_t bt_mmu_get_kernel_pgd(void);

#ifndef BT_CONFIG_USE_VIRTUAL_ADDRESSING
void bt_mmu_set_section(bt_paddr_t p, BT_u32 psize, int type);
#endif

#endif
