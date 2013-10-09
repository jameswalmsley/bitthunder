#include <bitthunder.h>
#include <collections/bt_list.h>

void *g_page_mutex = NULL;

#define BT_PAGE_LOCK()		if(g_page_mutex) BT_kMutexPend(g_page_mutex, 0)
#define BT_PAGE_UNLOCK()	if(g_page_mutex) BT_kMutexRelease(g_page_mutex)


struct bt_page {
	struct bt_list_head list;
	BT_u32	size;
};

static struct bt_list_head page_head;

static BT_u32 total_size;
static BT_u32 used_size;

bt_paddr_t bt_page_alloc(BT_u32 psize) {

	struct bt_page *blk = NULL, *tmp;
	struct bt_list_head *pos;

	BT_u32 size;

	BT_PAGE_LOCK();

	size = BT_PAGE_ALIGN(psize);

	bt_list_for_each(pos, &page_head) {
		struct bt_page *page = (struct bt_page *) pos;
		if(page->size >= size) {
			blk = page;
			break;
		}
	}

	if(!blk) {
		BT_PAGE_UNLOCK();
		return 0;	// OOM
	}

	if(blk->size == size) {
		bt_list_del(&blk->list);
	} else {
		tmp = (struct bt_page *) ((bt_vaddr_t) blk + size);
		tmp->size = blk->size - size;
		bt_list_add(&tmp->list, &blk->list);
		bt_list_del(&blk->list);
	}

	used_size += size;

	BT_PAGE_UNLOCK();

	return (bt_paddr_t) bt_virt_to_phys(blk);
}

void bt_page_free(bt_paddr_t paddr, BT_u32 size) {

	struct bt_page *blk, *prev;
	struct bt_list_head *pos;

	BT_PAGE_LOCK();

	size 	= BT_PAGE_TRUNC(size);
	paddr 	= BT_PAGE_ALIGN(paddr);

	blk = bt_phys_to_virt(paddr);


	bt_list_for_each(pos, &page_head) {
		prev = (struct bt_page *) pos;
		if((struct bt_page *) prev->list.next >= blk) {
			break;
		}
	}

	blk->size = size;
	bt_list_add(&blk->list, &page_head);

	if(blk->list.next != &page_head &&
	   ((bt_vaddr_t) blk + blk->size) == (bt_vaddr_t) blk->list.next) {
		// Block can be merged with next block.
		struct bt_page *next = (struct bt_page *) blk->list.next;
		blk->size += next->size;
		bt_list_del(&next->list);
	}

	prev = (struct bt_page *) blk->list.prev;
	if(prev != (struct bt_page *) &page_head &&
	   (bt_vaddr_t) prev + prev->size == (bt_vaddr_t) blk) {
		prev->size += blk->size;
		bt_list_del(&blk->list);
	}

	used_size -= size;

	BT_PAGE_UNLOCK();
}

BT_ERROR bt_page_reserve(bt_paddr_t paddr, BT_u32 psize) {

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

	bt_list_for_each(pos, &page_head) {
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

	used_size += size;

	BT_PAGE_UNLOCK();

	return BT_ERR_NONE;
}

extern bt_paddr_t __bt_init_start;
extern bt_paddr_t __bss_end;
extern bt_paddr_t _heap_end;
extern bt_paddr_t __absolute_end;

void bt_initialise_pages(void) {

	BT_LIST_INIT_HEAD(&page_head);

	bt_paddr_t start 	= (bt_paddr_t) bt_virt_to_phys(&__bt_init_start);
	BT_u32 len  		= (bt_paddr_t) (bt_virt_to_phys(&__bss_end)) - start;

	total_size 	= BT_PAGE_TRUNC((BT_TOTAL_PAGES * BT_PAGE_SIZE) - len);
	used_size 	= total_size;

	// Initialise the free list to total size of ram!
	bt_page_free(BT_PAGE_ALIGN(start+len), (BT_TOTAL_PAGES * BT_PAGE_SIZE) - len);

	start = (bt_paddr_t) bt_virt_to_phys(&_heap_end);
	len = &__absolute_end - &_heap_end;

	bt_page_reserve(start, len);
}

void bt_initialise_pages_second_stage() {
	g_page_mutex = BT_kMutexCreate();
}
