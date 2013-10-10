/**
 *	BitThunder - Virtual Memory Manager.
 *
 *	@author	James Walmsley <james@fullfat-fs.co.uk>
 **/

#include <bitthunder.h>
#include <string.h>

#define MAP_LOCK(map)	BT_kMutexPend(map->map_mutex, 0);
#define MAP_UNLOCK(map)	BT_kMutexRelease(map->map_mutex);

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

static void bt_segment_delete(struct bt_vm_map *map, struct bt_segment *seg) {


	if(seg->flags & BT_SEG_SHARED) {
		bt_list_del(&seg->shared_list);
		if(seg->shared_list.next == seg->shared_list.prev) {
			struct bt_segment *oldseg = bt_container_of(seg->shared_list.prev, struct bt_segment, shared_list, struct bt_list_head);
			oldseg->flags &= ~BT_SEG_SHARED;
		}
	}

	seg->flags = BT_SEG_FREE;

	if(seg != (struct bt_segment *) &map->segments) {
		BT_kFree(seg);
	}
}

static struct bt_segment *bt_segment_lookup(struct bt_vm_map *map, bt_vaddr_t addr, BT_u32 size) {
	struct bt_segment *seg;
	struct bt_list_head *pos;

	bt_list_for_each(pos, &map->segments) {
		seg = (struct bt_segment *) pos;

		BT_u64 seg_end 		= ((BT_u64) seg->addr + (BT_u64)seg->size);
		BT_u64 lookup_end 	= (addr + size);

		if(seg->addr <= addr &&
		   seg_end >= lookup_end) {
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
					MAP_UNLOCK(map);
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
	struct bt_segment *next, *prev;

	// If it was shared, unlink from the shared list.
	if(seg->flags & BT_SEG_SHARED) {
		bt_list_del(&seg->list);
		if(seg->shared_list.next == seg->shared_list.prev) {
			struct bt_segment *oldseg = bt_container_of(seg->shared_list.prev, struct bt_segment, shared_list, struct bt_list_head);
			oldseg->flags &= ~BT_SEG_SHARED;
		}
	}

	seg->flags = BT_SEG_FREE;

	// If next segment is free then merge.
	next = (struct bt_segment *) seg->list.next;
	if(next != (struct bt_segment *) &map->segments && (next->flags & BT_SEG_FREE)) {
		bt_list_del(&next->list);
		seg->size += next->size;
		BT_kFree(next);
	}

	prev = (struct bt_segment *) seg->list.prev;
	if(prev != (struct bt_segment *) &map->segments && (prev->flags & BT_SEG_FREE)) {
		bt_list_del(&seg->list);
		prev->size += seg->size;
		BT_kFree(seg);
	}	
}

static struct bt_segment *bt_segment_reserve(struct bt_vm_map *map, bt_vaddr_t addr, BT_u32 size) {

	bt_vaddr_t start = BT_PAGE_TRUNC(addr);
	size = BT_PAGE_ALIGN(size);

	struct bt_segment *seg = bt_segment_lookup(map, start, size);
	if(!seg || !(seg->flags & BT_SEG_FREE)) {
		return NULL;
	}

	struct bt_segment *prev, *next;
	BT_u32 diff;

	prev = NULL;
	if(seg->addr != addr) {
		prev = seg;
		diff = (BT_u32) (addr - seg->addr);
		seg = bt_segment_create(prev, addr, prev->size - diff);
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
	map->map_mutex  = BT_kMutexCreate();

	// Create a new page-directory.

	BT_LIST_INIT_HEAD(&map->segments);
	struct bt_segment *seg = BT_kMalloc(sizeof(*seg));
	if(!seg) {
		return NULL;
	}

	seg->addr 	= BT_PAGE_SIZE;	// We want to ensure the nothing is ever mapped into a process at vaddr 0x0 (NULL pointer exceptions).
	seg->phys 	= 0;
	seg->size 	= BT_MM_USERLIMIT - BT_PAGE_SIZE;
	seg->flags 	= BT_SEG_FREE;

	bt_list_add(&seg->list, &map->segments);	// Add the initial segment.

	map->pgd = bt_mmu_newmap();

	return map;
}

void bt_vm_destroy(struct bt_vm_map *map) {
	struct bt_segment *seg, *tmp;

	if(--map->refcount > 0) {
		return;
	}

	// lock process mapper

	seg = (struct bt_segment *) map->segments.next;
	while(seg != (struct bt_segment *) &map->segments) {
		if(seg->flags != BT_SEG_FREE) {
			// Unmap this segment
			bt_mmu_map(map->pgd, seg->phys, seg->addr, seg->size, BT_PAGE_UNMAP);

			// Free underlying pages if not shared and mapped.
			if(!(seg->flags & BT_SEG_SHARED) && !(seg->flags & BT_SEG_MAPPED)) {
				bt_page_free(seg->phys, seg->size);
			}			
		}

		tmp = seg;
		seg = (struct bt_segment *) seg->list.next;
		bt_segment_delete(map, tmp);
	}

	if(map == BT_GetProcessTask(BT_GetProcessHandle())->map) {
		bt_mmu_switch(kernel_map.pgd);
	}

	// Switch to kernel map before deleting the cpd.

	bt_mmu_terminate(map->pgd);
	BT_kFree(map);

	// unlock process mapper.
}

extern bt_vaddr_t __bt_init_start;
extern bt_vaddr_t __bss_end;
extern bt_vaddr_t _heap_end;
extern bt_vaddr_t __absolute_end;

void bt_vm_init(void) {

	bt_pgd_t pgd;

	bt_mmu_init(NULL);

	pgd = bt_mmu_get_kernel_pgd();
	if(!pgd) {
		//do_kernel_panic("bt_vm_init");
		return;
	}

	kernel_map.pgd 			= pgd;
	kernel_map.refcount 	= 1;
	kernel_map.size 		= 0;
	kernel_map.map_mutex 	= BT_kMutexCreate();

	BT_LIST_INIT_HEAD(&kernel_map.segments);

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

	bt_list_add(&seg->list, &kernel_map.segments);

	bt_vaddr_t start = (bt_vaddr_t) &__bt_init_start;
	BT_u32 len   	 = (BT_u32) (BT_TOTAL_PAGES * BT_PAGE_SIZE);

	bt_segment_reserve(&kernel_map, start, len);	// Reserve RAM section.
}

BT_ERROR bt_vm_reference(struct bt_vm_map *map) {
	map->refcount += 1;
	return BT_ERR_NONE;
}

struct bt_vm_map *bt_vm_get_kernel_map(void) {
	return &kernel_map;
}

bt_paddr_t bt_vm_translate(bt_vaddr_t addr, BT_u32 size) {
	return bt_mmu_extract(curtask->map->pgd, addr, size);
}

bt_vaddr_t bt_vm_map_region(struct bt_vm_map *map, bt_paddr_t pa, bt_vaddr_t va, BT_u32 size, BT_u32 type) {

	MAP_LOCK(map);

	static struct bt_segment *seg = NULL;
	if(!va) {
		seg = bt_segment_alloc(map, size);
	} else {
		seg = bt_segment_reserve(map, va, size);
	}

	if(!seg) {
		MAP_UNLOCK(map);
		return 0;
	}

	seg->phys 	= pa;
	switch(type) {
	case BT_PAGE_READ:
		seg->flags = BT_SEG_READ;
		break;

	case BT_PAGE_WRITE:
	case BT_PAGE_SYSTEM:
		seg->flags = BT_SEG_READ | BT_SEG_WRITE;
		break;

	case BT_PAGE_IOMEM:
		seg->flags = BT_SEG_READ | BT_SEG_WRITE | BT_SEG_IOMAPPED;
		break;

	default:
		bt_segment_free(map, seg);
		MAP_UNLOCK(map);
		return 0;
		break;
	}

	seg->flags |= BT_SEG_MAPPED;

	bt_mmu_map(map->pgd, seg->phys, seg->addr, seg->size, type);

	MAP_UNLOCK(map);

	return seg->addr;
}

void bt_vm_unmap_region(struct bt_vm_map *map, bt_vaddr_t va) {

	MAP_LOCK(map);

	struct bt_segment *seg = bt_segment_lookup(map, va, 0);
	if(!seg) {
		MAP_UNLOCK(map);
		return;
	}

	if(!(seg->flags & BT_SEG_MAPPED)) {
		// Bad mapping.
		MAP_UNLOCK(map);
		return;
	}

	bt_mmu_map(map->pgd, seg->phys, seg->addr, seg->size, BT_PAGE_UNMAP);

	bt_segment_free(map, seg);

	MAP_UNLOCK(map);
}

static BT_ERROR do_allocate(struct bt_vm_map *map, void **addr, BT_u32 size, BT_u32 flags) {
	BT_ERROR Error = BT_ERR_NONE;

	struct bt_segment *seg;
	bt_vaddr_t start, end;
	bt_paddr_t pa;

	if(!size) {
		return BT_ERR_GENERIC;
	}

	MAP_LOCK(map);

	if(flags & BT_VM_ALLOC_ANYWHERE) {
		size = BT_PAGE_ALIGN(size);
		seg = bt_segment_alloc(map, size);
		if(!seg) {
			MAP_UNLOCK(map);
			return BT_ERR_NO_MEMORY;
		}
	} else {
		start = BT_PAGE_TRUNC((bt_vaddr_t) *addr);
		end = BT_PAGE_ALIGN(start + size);
		size = (BT_u32) (end - start);

		seg = bt_segment_reserve(map, start, size);
		if(!seg) {
			MAP_UNLOCK(map);
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

	memset((void *) bt_phys_to_virt(pa), 0, seg->size);
	*addr = (void *) seg->addr;

	map->size += size;

	MAP_UNLOCK(map);

	return Error;

err_free_page:
	bt_page_free(pa, size);

err_free_seg:
	bt_segment_free(map, seg);

	MAP_UNLOCK(map);

	return Error;
}

BT_ERROR bt_vm_allocate(struct bt_task *task, void **addr, BT_u32 size, BT_u32 flags) {
	return do_allocate(task->map, addr, size, flags);
}

static BT_ERROR do_free(struct bt_vm_map *map, void *addr) {
	struct bt_segment *seg;
	bt_vaddr_t va;

	va = BT_PAGE_TRUNC((bt_vaddr_t) addr);

	MAP_LOCK(map);

	seg = bt_segment_lookup(map, va, 0);
	if(!seg || seg->addr != va || (seg->flags & BT_SEG_FREE)) {
		MAP_UNLOCK(map);
		return BT_ERR_GENERIC;
	}

	bt_mmu_map(map->pgd, seg->phys, seg->addr, seg->size, BT_PAGE_UNMAP);

	if(!(seg->flags & BT_SEG_SHARED) && !(seg->flags & BT_SEG_MAPPED)) {
		bt_page_free(seg->phys, seg->size);
	}

	map->size -= seg->size;

	bt_segment_free(map, seg);

	MAP_UNLOCK(map);

	return BT_ERR_NONE;
}

BT_ERROR bt_vm_free(struct bt_task *task, void *addr) {
	return do_free(task->map, addr);
}
