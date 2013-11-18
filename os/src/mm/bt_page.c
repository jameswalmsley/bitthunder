#include <bitthunder.h>
#include <collections/bt_list.h>

void *g_page_mutex = NULL;

#define BT_PAGE_LOCK()		if(g_page_mutex) BT_kMutexPend(g_page_mutex, 0)
#define BT_PAGE_UNLOCK()	if(g_page_mutex) BT_kMutexRelease(g_page_mutex)


struct bt_page {
	struct bt_list_head list;
	BT_u32	size;
};

static bt_page_pool default_pool;

#ifdef BT_CONFIG_MEM_PAGE_COHERENT_POOL
static bt_page_pool coherent_pool;
#endif

bt_paddr_t bt_page_pool_alloc(bt_page_pool *pool, BT_u32 psize, BT_u32 order) {

	struct bt_page *blk = NULL, *tmp;
	struct bt_list_head *pos;

	BT_u32 size;

	if(!psize) {
		return 0;
	}

	BT_PAGE_LOCK();

	size = BT_PAGE_ALIGN(psize);

	bt_list_for_each(pos, &pool->page_head) {
		struct bt_page *page = (struct bt_page *) pos;
		if(page->size >= size) {
			if(order) {
				bt_vaddr_t 	align_mask 	= ((BT_PAGE_SIZE<<order)-1);
				bt_vaddr_t 	aligned 	= (bt_vaddr_t) (((bt_vaddr_t) page) + align_mask) & ~align_mask;
				BT_u32 		diff 		= aligned - (bt_vaddr_t) page;
				if(page->size < (size + diff)) {
					continue;
				}
			}

			blk = page;
			break;
		}
	}

	if(!blk) {
		BT_PAGE_UNLOCK();
		return 0;	// OOM
	}

	if(order) {
		bt_vaddr_t 	align_mask 	= ((BT_PAGE_SIZE<<order)-1);
		bt_vaddr_t 	aligned 	= (bt_vaddr_t) (((bt_vaddr_t) blk) + align_mask) & ~align_mask;
		BT_u32 		diff 		= aligned - (bt_vaddr_t) blk;

		if(diff) {	// Split off the unaligned part.
			tmp = (struct bt_page *) ((bt_vaddr_t) blk + diff);
			tmp->size = blk->size - diff;
			blk->size = diff;
			bt_list_add(&tmp->list, &blk->list);
			blk = tmp;
		}
	}

	if(blk->size == size) {
		bt_list_del(&blk->list);
	} else {
		tmp = (struct bt_page *) ((bt_vaddr_t) blk + size);
		tmp->size = blk->size - size;
		bt_list_add(&tmp->list, &blk->list);
		bt_list_del(&blk->list);
	}

	pool->used_size += size;

	BT_PAGE_UNLOCK();

	return (bt_paddr_t) bt_virt_to_phys(blk);
}

void bt_page_pool_free(bt_page_pool *pool, bt_paddr_t paddr, BT_u32 size) {

	struct bt_page *blk, *prev;
	struct bt_list_head *pos;

	if(!size) {
		return;
	}

	BT_PAGE_LOCK();

	size 	= BT_PAGE_ALIGN(size);

	blk = (struct bt_page *) bt_phys_to_virt(paddr);

	bt_list_for_each(pos, &pool->page_head) {
		prev = (struct bt_page *) pos;
		if((struct bt_page *) prev->list.next >= blk) {
			break;
		}
	}

	blk->size = size;
	bt_list_add(&blk->list, &pool->page_head);

	if(blk->list.next != &pool->page_head &&
	   ((bt_vaddr_t) blk + blk->size) == (bt_vaddr_t) blk->list.next) {
		// Block can be merged with next block.
		struct bt_page *next = (struct bt_page *) blk->list.next;
		blk->size += next->size;
		bt_list_del(&next->list);
	}

	prev = (struct bt_page *) blk->list.prev;
	if(prev != (struct bt_page *) &pool->page_head &&
	   (bt_vaddr_t) prev + prev->size == (bt_vaddr_t) blk) {
		prev->size += blk->size;
		bt_list_del(&blk->list);
	}

	pool->used_size -= size;

	BT_PAGE_UNLOCK();
}

void bt_page_pool_attach(bt_page_pool *pool, bt_paddr_t paddr, BT_u32 size) {
	pool->total_size += size;
	pool->used_size += size;
	bt_page_pool_free(pool, paddr, size);
}

BT_ERROR bt_page_pool_reserve(bt_page_pool *pool, bt_paddr_t paddr, BT_u32 psize) {

	struct bt_page *blk = NULL, *tmp;
	struct bt_list_head *pos;

	bt_vaddr_t start, end;
	BT_u32 size;

	if(!psize) {
		return BT_ERR_NONE;
	}

	BT_PAGE_LOCK();

	start 	= BT_PAGE_TRUNC((bt_vaddr_t) bt_phys_to_virt(paddr));
	end		= BT_PAGE_ALIGN((bt_vaddr_t) bt_phys_to_virt(paddr + psize));
	size	= end - start;

	bt_list_for_each(pos, &pool->page_head) {
		struct bt_page *pg  = (struct bt_page *) pos;
		if((bt_vaddr_t) pg <= start && end <= (bt_vaddr_t) pg + pg->size) {
			blk = pg;
			break;
		}
	}

	if(!blk) {
		BT_PAGE_UNLOCK();
		return BT_ERR_NO_MEMORY;
	}

	if((bt_vaddr_t) blk == start && blk->size == size) {
		bt_list_del(&blk->list);
	} else {
		// split it
		if((bt_vaddr_t) blk + blk->size != end) {
			tmp = (struct bt_page *) end;
			tmp->size = (bt_vaddr_t) blk + blk->size - end;
			blk->size -= tmp->size;

			bt_list_add(&tmp->list, &blk->list);
		}

		if((bt_vaddr_t) blk == start) {
			bt_list_del(&blk->list);
		} else {
			blk->size = start - (bt_vaddr_t) blk;
		}
	}

	pool->used_size += size;

	BT_PAGE_UNLOCK();

	return BT_ERR_NONE;
}

bt_paddr_t bt_page_alloc(BT_u32 psize) {
	return bt_page_pool_alloc(&default_pool, psize, 0);
}

bt_paddr_t bt_page_alloc_aligned(BT_u32 psize, BT_u32 order) {
	return bt_page_pool_alloc(&default_pool, psize, order);
}

void bt_page_free(bt_paddr_t paddr, BT_u32 psize) {
	bt_page_pool_free(&default_pool, paddr, psize);
}

BT_ERROR bt_page_reserve(bt_paddr_t paddr, BT_u32 psize) {
	return bt_page_pool_reserve(&default_pool, paddr, psize);
}

#ifdef BT_CONFIG_MEM_PAGE_COHERENT_POOL
bt_paddr_t bt_page_alloc_coherent(BT_u32 psize) {
	return bt_page_pool_alloc(&coherent_pool, psize, 0);
}

void bt_page_free_coherent(bt_paddr_t paddr, BT_u32 psize) {
	bt_page_pool_free(&coherent_pool, paddr, psize);
}

BT_ERROR bt_page_reserve_coherent(bt_paddr_t paddr, BT_u32 psize) {
	return bt_page_pool_reserve(&coherent_pool, paddr, psize);
}
#endif

BT_ERROR bt_page_pool_init(bt_page_pool *pool) {
	BT_LIST_INIT_HEAD(&pool->page_head);
	pool->total_size = 0;
	pool->used_size = 0;
	return BT_ERR_NONE;
}

extern bt_paddr_t __bt_init_start;
extern bt_paddr_t __bss_end;
extern bt_paddr_t _heap_end;
extern bt_paddr_t __absolute_end;

void bt_initialise_pages(void) {

	bt_page_pool_init(&default_pool);

#ifdef BT_CONFIG_MEM_PAGE_COHERENT_POOL
	bt_page_pool_init(&coherent_pool);
#endif

	bt_paddr_t start 	= (bt_paddr_t) bt_virt_to_phys(&__bt_init_start);
	BT_u32 len  		= (bt_paddr_t) (bt_virt_to_phys(&__bss_end)) - start;

	// Initialise the free list to total size of ram!
	bt_page_pool_attach(&default_pool, BT_PAGE_ALIGN(start+len), (BT_TOTAL_PAGES * BT_PAGE_SIZE) - len);

	start = (bt_paddr_t) bt_virt_to_phys(&_heap_end);
	len = &__absolute_end - &_heap_end;

	bt_page_reserve(start, len);
}

void bt_initialise_pages_second_stage() {
	g_page_mutex = BT_kMutexCreate();
}

bt_paddr_t bt_initialise_coherent_pages() {
#ifdef BT_CONFIG_MEM_PAGE_COHERENT_POOL
	bt_paddr_t coherent = bt_page_alloc_aligned(BT_CONFIG_MEM_PAGE_COHERENT_LENGTH, 8);	// Order 8 aligned for 1MB.
	bt_page_pool_attach(&coherent_pool, coherent, BT_CONFIG_MEM_PAGE_COHERENT_LENGTH);

#ifndef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	BT_u32 i;
	for(i = 0; i < BT_CONFIG_MEM_PAGE_COHERENT_LENGTH/BT_SECTION_SIZE; i++) {
		bt_mmu_set_section(coherent+(BT_SECTION_SIZE * i), BT_SECTION_SIZE, BT_PAGE_IOMEM);
	}
#endif

	return coherent;
#else
	return NULL;
#endif
}
