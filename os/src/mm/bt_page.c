/**
 *	Page Allocator for BitThunder.
 *
 *	@author			James Walmsley
 *	@copyright		2013 James Walmsley
 *
 **/

#include <bt_error.h>
#include <mm/bt_page.h>
#include <string.h>

#define BT_TOTAL_PAGES	(BT_CONFIG_LINKER_RAM_LENGTH/BT_PAGE_SIZE)

/**
 *	Global array of page descriptors,
 *	Allows pages to be found based on memory addresses in O(1) complexity.
 *
 *	Within the array, a linked list of free blocks is formed,
 *	all pages making up a free block have the size field set to be the distance from the block HEAD.
 *
 *	This means that given any page within a free region, you can find the HEAD in constant O(1).
 *	(This is important when finding free regions around a block during page_free()).
 *
 *	Similarly all pages within an allocated block keep reference to the block HEAD.
 *	This allows the bt_page_free() api to be called with a pointer from anywhere within an
 *	allocated block.
 *
 **/

static BT_u32 	total_size 	= 0;
static BT_u32	used_size	= 0;

static BT_PAGE	g_oPages[BT_TOTAL_PAGES];

static struct bt_list_head free_head;
//static BT_PAGE	*used_head = NULL;

//static BT_PAGE	*free_size[13] = { NULL };	// Array of log2 of free blocks by size.

#define BLOCK_INDEX(x)	(x - &g_oPages[0])
#define BLOCK_PADDR(x)	(BT_CONFIG_RAM_PHYS + (BLOCK_INDEX(x) * BT_PAGE_SIZE))

#define PHYS_INDEX(x)	(((x  - BT_CONFIG_RAM_PHYS) & ~BT_PAGE_MASK) / BT_PAGE_SIZE)
#define PHYS_BLOCK(x)	(&g_oPages[PHYS_INDEX(x)])

#define LOCK_PAGES()
#define UNLOCK_PAGES()

static void set_head_distance(BT_PAGE *pBlockHead, BT_u32 flags) {
	BT_u32 i;
	for(i = 1; i < (pBlockHead->size / BT_PAGE_SIZE); i++) {
		pBlockHead[i].size = i;
		pBlockHead[i].flags = flags;
	}
}

/**
 *	Allocate's PAGES of continuous physical memory of the specified size.
 *
 *
 **/
BT_PHYS_ADDR bt_page_alloc(BT_u32 psize) {

	BT_u32 size;
	BT_PAGE *block = NULL;			// Pointer to a free block suitable for this allocation.
	struct bt_list_head *pos;

	size = BT_PAGE_ALIGN(psize);	// Round up the requested size to a multiple of PAGE_SIZE

	LOCK_PAGES();
	{

		bt_list_for_each(pos, &free_head) {
			BT_PAGE *item = (BT_PAGE *) pos;
			if(item->size >= size) {
				block = item;
				break;
			}
		}

		if(!block) {
			UNLOCK_PAGES();
			return 0;					// Out of free pages, no-suitable candidate found!
		}


		if(block->size == size) {
			/*
			 *	If the allocation was the exact size, the simply remove this block from the list!
			 */
			bt_list_del(&block->list);

		} else {
			/*
			 *	Here we must remove the block from the list, and split it up!
			 *
			 *	The lower half becomes the allocated page-block.
			 *	The upper is placed back in the free pool.
			 */

			bt_list_del(&block->list);

			BT_PAGE *new 	= &block[size/BT_PAGE_SIZE];
			new->size 		= block->size - size;
			new->flags 		= 0;

			bt_list_add(&new->list, &free_head);
		}
	}
	UNLOCK_PAGES();

	block->size 	= size;
	block->flags 	= BT_PAGE_USED | BT_PAGE_HEAD;

	set_head_distance(block, BT_PAGE_USED);

	used_size += size;

	return BLOCK_PADDR(block);
}



void bt_page_free(BT_PHYS_ADDR paddr) {

	BT_u32 index = PHYS_INDEX(paddr);
	if(index >= BT_TOTAL_PAGES) {
		// Invalid pointer!
		return;
	}

	LOCK_PAGES();
	{
		BT_PAGE *block = &g_oPages[index];

		if(!block->flags & BT_PAGE_USED || block->flags & BT_PAGE_RESERVED) {
			// !!! Double free, or invalid pointer!!	// Or trying to free reserved region.
			UNLOCK_PAGES();
			return;
		}

		if(!(block->flags & BT_PAGE_HEAD)) {
			block -= block->size;
			// Skip back to the head of this allocation!
		}

		block->flags = 0;
		BT_u32 i;
		for(i = 1; i < (block->size / BT_PAGE_SIZE); i++) {
			//block[i].size 	= 0;	// Only perform the distance adjustment if we ad-join with adjacent free blocks!
			block[i].flags 	= 0;
		}

		used_size -= block->size;
		BT_u32 size_index = block->size / BT_PAGE_SIZE;

		BT_BOOL in_free_list = BT_FALSE;

		if(index && !(g_oPages[index-1].flags & BT_PAGE_USED)) {
			// Previous block is also free, we should join the blocks together in the free list!
			BT_PAGE *prev_block = &g_oPages[index-1];
			if(!(prev_block->flags & BT_PAGE_HEAD)) {
				prev_block -= prev_block->size;			// Move back to the head of the block.
			}

			prev_block->size += block->size;
			in_free_list = BT_TRUE;	// Previous block MUST have been in the free list already!
			set_head_distance(prev_block, 0);
			block = prev_block;
			//index = BLOCK_INDEX(block);
		}

		if(index < BT_TOTAL_PAGES && !(g_oPages[index+size_index].flags & BT_PAGE_USED)) {
			// Next block is free... join them!
			BT_PAGE *next_block = &g_oPages[index+size_index];
			bt_list_del(&next_block->list);
			block->size += next_block->size;
			set_head_distance(block, 0);
		}

		if(!in_free_list) {
			bt_list_add(&block->list, &free_head);
		}
	}
	UNLOCK_PAGES();
}


BT_ERROR bt_page_reserve(BT_PHYS_ADDR paddr, BT_u32 psize) {

	BT_PHYS_ADDR start, end;
	BT_u32 size;

	if(!psize) {
		return 0;
	}

	start = BT_PAGE_TRUNC(paddr);
	end = BT_PAGE_ALIGN(paddr + psize);
	size = end - start;

	LOCK_PAGES();
	{
		BT_PAGE *block;
		BT_u32 index_start = PHYS_INDEX(start);
		BT_u32 index_end = PHYS_INDEX(end);
		BT_u32 index_len = index_end - index_start;

		/*
		 *	We must ensure the entire requested range is actually available!
		 *	Then we can remove it from the allocator permanently.
		 */
		BT_u32 i;
		for(i = 0; i < index_len; i++) {
			if(g_oPages[index_start + i].flags & BT_PAGE_USED) {
				UNLOCK_PAGES();
				return BT_ERR_GENERIC;
			}
		}

		// Remove / split the block from within :P
		block = &g_oPages[index_start];

		if(!(block->flags & BT_PAGE_HEAD)) {
			block -= block->size;
			// Skip back to the head of this allocation!
		}

		bt_list_del(&block->list);
		BT_u32 index_block_a = BLOCK_INDEX(block);			// Index of the block beginning the reserved section.
		BT_u32 index_block_b = index_start + index_len;		// Index of the block after the reserved section.

		BT_u32 orig_size = block->size;

		if(index_block_a != index_start) {
			block->size = (index_block_a - index_start) * BT_PAGE_SIZE;
			block->flags = 0;

			set_head_distance(block, 0);

			bt_list_add(&block->list, &free_head);
		}

		block = &g_oPages[index_block_b];
		block->size = orig_size - (index_len * BT_PAGE_SIZE);
		block->flags = BT_PAGE_HEAD;

		set_head_distance(block, 0);

		bt_list_add(&block->list, &free_head);

		BT_PAGE *new = &g_oPages[index_start];
		new->size = index_len * BT_PAGE_SIZE;
		new->flags = BT_PAGE_RESERVED | BT_PAGE_USED  | BT_PAGE_HEAD;
		set_head_distance(new, BT_PAGE_RESERVED | BT_PAGE_USED);

	}
	UNLOCK_PAGES();

	return BT_ERR_NONE;
}

void bt_initialise_pages(void) {

	BT_LIST_INIT_HEAD(&free_head);

	total_size = BT_TOTAL_PAGES * BT_PAGE_SIZE;

	BT_PAGE *block = &g_oPages[0];
	block->size = total_size;
	block->flags = BT_PAGE_HEAD;

	set_head_distance(block, 0);

	bt_list_add(&block->list, &free_head);

	// Reserve already used pages!
	bt_page_reserve(0x00100000, 1024*1024*2);	// Reserve 2MB for BT Kernel
}
