/**
 *	BitThunder - Virtual Memory Manager.
 *
 *	@author	James Walmsley <james@fullfat-fs.co.uk>
 **/

#include <mm/bt_vm.h>
#include <mm/bt_mm.h>
#include <mm/bt_page.h>

static struct bt_vm_map kernel_map;	// Kernels VM map.

static struct bt_segment *bt_segment_create(struct bt_segment *prev, bt_vaddr_t addr, BT_u32 size) {
	struct bt_segment *seg;

	seg = BT_kMalloc(sizeof(struct bt_segment));
	if(!seg) {
		return NULL;
	}

	seg->addr 	= addr;
	seg->size 	= size;
	seg->phys 	= 0;
	seg->flags 	= BT_SEG_FREE;

	bt_list_add(&seg->list, &prev->list);

	return seg;
}

static struct bt_segment_lookup(struct bt_vm_map *map, bt_vaddr_t addr, BT_u32 size) {
	struct bt_segment *seg;
	struct bt_list_head *pos;

	bt_list_for_each(pos, &map->segments) {
		seg = (struct bt_segment *) pos;

		if(seg->addr <= addr &&
		   seg->addr + seg->size >= addr + size) {
			return seg;
		}
	}

	return NULL;
}

static struct bt_segment *bt_segment_alloc(struct bt_vm_map *map, BT_u32 size) {

	struct bt_segment *seg;
	struct bt_list_head *pos;

	size = BT_PAGE_ALIGN(size);

	bt_list_for_each(pos, &map->segments) {
		seg = (struct bt_segment *) pos;

		if((seg->flags & BT_SEG_FREE) && seg->size >= size) {
			if(seg->size != size) {
				// split the segment.
				if(!bt_segment_create(seg, seg->addr + size, seg->size - size)) {
					return NULL;
				}

				seg->size = size;
			}

			return seg;
		}
	}

	return NULL;
}

static void bt_segment_free(struct bt_vm_map *map, struct bt_segment *seg) {
	struct bt_segment *prev, *next;

	seg->flags = BT_SEG_FREE;

	/*if(seg->flags & BT_SEG_SHARED) {
	  }*/
}

static struct bt_segment *bt_segment_reserve(struct bt_vm_map *map, bt_vaddr_t addr, BT_u32 size) {

	start 	= BT_PAGE_TRUNC(addr);
	size 	= BT_PAGE_ALIGN(size);

	struct bt_segment *seg = bt_segment_lookup(map, addr, size);
	if(!seg || !(seg->flags & BT_SEG_FREE)) {
		return NULL;
	}

	struct bt_segment *prev;
	BT_u32 diff;

	prev = NULL;
	if(seg->addr != addr) {
		prev = seg;
		diff = (BT_u32) (addr - seg->addr);
		seg = bt_seg_create(prev, addr, prev->size - diff);
		if(!seg) {
			return NULL;
		}
		prev->size = diff;
	}

	if(seg->size != size) {
		next = bt_segment_create(seg, seg->addr + size, seg->size - size);
		if(!next) {
			if(prev) {
				bt_segment_free(map, seg);
			}
			return NULL;
		}

		seg->size = size;
	}

	seg->flags = 0;

	return seg;
}

struct bt_vm_map *bt_vm_create(void) {

	struct bt_vm_map *map;

	map = BT_kMalloc(sizeof(struct bt_vm_map));
	if(!map) {
		return NULL;
	}

	map->refcount	= 1;
	map->size 		= 0;

	// Create a new page-directory.

	BT_LIST_INIT_HEAD(map->segments);
	struct bt_segment *seg = BT_kMalloc(sizeof(*seg));
	if(!seg) {
		return NULL;
	}

	seg->addr 	= BT_PAGE_SIZE;	// We want to ensure the nothing is ever mapped into a process at vaddr 0x0 (NULL pointer exceptions).
	seg->phys 	= 0;
	seg->size 	= BT_MM_USERLIMIT - BT_PAGE_SIZE;
	seg->flags 	= BT_SEG_FREE;

	bt_list_add(&map->segments, &seg->list);	// Add the initial segment.

	map->pgd = bt_mmu_newmap();

	return map;
}

void bt_vm_destroy(struct bt_vm_map *map) {
	struct bt_segment *seg, *tmp;

	if(--map->refcount > 0) {
		return;
	}

	// lock process mapper

	struct bt_list_head *pos;
	bt_list_for_each(pos, &map->segments) {
		seg = (struct bt_segment *) pos;
		if(seg->flags != SEG_FREE) {
			// Unmap this segment

			// Free underlying pages if not shared and mapped.
		}

		tmp = seg;
		// delete the segment.
	}

	// Switch to kernel map before deleting the cpd.

	// mmu_terminate(map-pgd)
	BT_kFree(map);

	// unlock process mapper.
}


extern bt_vaddr_t __bt_init_start;
extern bt_vaddr_t __bss_end;
extern bt_vaddr_t _heap_end;
extern bt_vaddr_t __absolute_end;

void bt_vm_init(void) {

	bt_pgd_t pgd;

	pgd = bt_mmu_get_kernel_pgd();
	if(!pgd) {
		//do_kernel_panic("bt_vm_init");
		return;
	}

	kernel_map.pgd 		= pgd;
	kernel_map.refcount = 1;
	kernel_map.size 	= 0;

	BT_LIST_INIT_HEAD(kernel_map.segments);

	struct bt_segment *seg = BT_kMalloc(sizeof(*seg));
	if(!seg) {
		//do_kernel_panic("bt_vm:");
		return;
	}

	/*
	 *	Create the kernel-space segment.
	 */
	seg->addr 	= BT_MM_USERLIMIT;
	seg->phys 	= BT_CONFIG_RAM_PHYS;
	seg->flags 	= BT_SEG_FREE;
	seg->size 	= 0x100000000 - 0xC0000000;

	bt_list_add(&kernel_map->segments, &seg->list);

	bt_vaddr_t start = &__bt_init_start;
	BT_u32 len   = &__bss_end - start;

	bt_segment_reserve(&kernel_map, start, len);	// Reserve virtual spaces used for kernel text, data and bss.

	start = &_heap_end;
	len = &__absolute_end - &_heap_end;

	bt_segment_reserve(&kenrel_map, start, len);	// Reserve the IRQ / SVC stacks at end of memory.
}

struct bt_vm_map *bt_vm_get_kernel_map(void) {
	return &kernel_map;
}

bt_paddr_t bt_vm_translate(bt_vaddr_t addr, BT_u32 size) {
	// Get current pgd, and use the mmu to extract it.
	return NULL;
}

bt_vaddr_t bt_vm_map_region(struct bt_vm_map *map, bt_paddr_t pa, bt_vaddr_t va, BT_u32 size, BT_u32 type) {

	static struct bt_segment *seg = NULL;
	if(!va) {
		seg = bt_segment_alloc(map, size);
	} else {
		seg = bt_segment_reserve(map, va, size);
	}

	if(!seg) {
		return NULL;
	}

	seg->phys 	= phys;
	seg->flags 	= BT_SEG_MAPPED | BT_SEG_READ | BT_SEG_WRITE;

	bt_mmu_map(map, seg->phys, seg->addr, seg->size, type);

	return seg->addr;
}

void bt_vm_unmap_region(struct bt_vm_map *map, bt_vaddr_t va) {
	struct bt_segment *seg = bt_segment_lookup(map, va, 0);
	if(!seg) {
		return;
	}

	if(!(seg->flags & BT_PAGE_MAPPED)) {
		// Bad mapping.
		return;
	}

	bt_mmu_map(map->pgd, seg->phys, seg->addr, seg->size, BT_PAGE_UNMAP);
}

static BT_ERROR do_allocate(struct bt_vm_map *map, void **addr, BT_u32 size, BT_u32 flags) {
	BT_ERROR Error = BT_ERR_NONE;

	struct bt_segment *seg;
	bt_vaddr_t start, end;
	bt_paddr_t pa;

	if(!size) {
		return BT_ERR_GENERIC;
	}

	if(flags & BT_VM_ALLOC_ANYWHERE) {
		size = BT_PAGE_ALIGN(size);
		seg = bt_segment_alloc(map, size);
		if(!seg) {
			return BT_ERR_NO_MEMORY;
		}
	} else {
		start = BT_PAGE_TRUNC((bt_vaddt_t) *addr);
		end = BT_PAGE_ALIGN(start + size);
		size = (BT_u32) (end - start);

		seg = bt_segment_reserve(map, start, size);
		if(!seg) {
			return BT_ERR_NO_MEMORY;
		}
	}

	seg->flags = BT_SEG_READ | BT_SEG_WRITE;

	// Allocate physical pages and map into the virtual segment just allocated.
	pa = bt_page_alloc(size);
	if(!pa) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_seg;
	}

	if(bt_mmu_map(map->pgd, pa, seg->addr, size, BT_PAGE_WRITE)) {
		Error = BT_ERR_NO_MEMORY;
		goto err_free_page;
	}

	seg->phys = pa;

	memset(bt_phys_to_virt(pa), 0, seg->size);
	*addr = (void *) seg->addr;

	map->size += size;

	return Error;

err_free_page:
	bt_page_free(pa);

err_free_seg:
	bt_segment_free(map, seg);

	return Error;
}

BT_ERROR bt_vm_allocate(BT_HANDLE hProcess, void **addr, BT_u32 size, BT_u32 flags) {
	BT_ERROR Error = BT_ERR_NONE;

	struct bt_vm_map *map = bt_process_getmap(hProcess);
	if(!map) {
		return BT_ERR_GENERIC;
	}

	return do_allocate(map, addr, size, flags);
}

static BT_ERROR do_free(struct bt_vm_map *map, void *addr) {
	struct bt_segment *seg;
	bt_vaddr_t va;

	va = BT_PAGE_TRUNC((bt_vaddr_t) addr);

	seg = bt_segment_lookup(map, va, 0);
	if(!seg || seg->addr != va || (seg->flags & BT_SEG_FREE)) {
		return BT_ERR_GENERIC;
	}

	bt_mmu_map(map->pgd, seg->phys, seg->addr, seg->size, BT_PAGE_UNMAP);

	if(!(seg->flags & BT_SEG_SHARED) && !(seg->flags & BT_SEG_MAPPED)) {
		bt_page_free(seg->phys);
	}

	map->size -= seg->size;

	bt_segment_free(map, seg);

	return BT_ERR_NONE;
}

BT_ERROR bt_vm_free(BT_HANDLE hProcess, void *addr) {
	struct bt_vm_map *map = bt_process_getmap(hProcess);
	return do_free(map, addr);
}
