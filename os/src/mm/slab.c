/**
 *	BitThunder SLAB Allocator for Kernel Memory.
 *
 *	@author	James Walmsley	<james@fullfat-fs.co.uk>
 *	@author Clifford Wolf	<clifford@clifford.at>
 *
 *	This allocator uses a generic SLAB based approach. It is based on
 *	a really neat (perfect) idea that Clifford recommended. (Thanks).
 *
 *	All operations complete in O(1), unless they hit the page-allocator,
 * 	in which case the allocation complexity is inherited from the page-allocator.
 *
 **/

#include <bitthunder.h>
#include <collections/bt_list.h>
#include <mm/bt_mm.h>
#include <mm/bt_page.h>


#define SLAB_LOCK(cache)	if(cache->slab_mutex) BT_kMutexPend(cache->slab_mutex, 0)
#define SLAB_UNLOCK(cache)	if(cache->slab_mutex) BT_kMutexRelease(cache->slab_mutex)

#define BT_CACHE_FLAGS_OBJECT		0x00000001	///< Unset if standard allocation cache.
#define BT_CACHE_FLAGS_UNALIGNED	0x00000002

#define BT_CACHE_GENERIC_MIN		16
#define BT_CACHE_GENERIC_MAX		8192

struct block;

struct block_free {
	struct	block_free *next;
};

struct block {
	struct bt_list_head 	list;			///< This is an item in a list of blocks.
	BT_u32					ulSize;
};

static BT_CACHE g_oDefault[10];		///< Array of primary caches, starting at 16bytes, upto 8192 bytes


static void init_attach_block(BT_CACHE *pCache, struct block *pBlock, BT_u32 ulSize) {
	BT_u32 i;
	BT_u32 total_objects = (ulSize) / pCache->ulObjectSize;

	pBlock->ulSize = ulSize;

	struct block_free *free = (struct block_free *) (pBlock);
	for(i = 0; i < total_objects; i++) {
		free->next = (struct block_free *) (((BT_u8 *) free) + pCache->ulObjectSize);
		free = (struct block_free *) (((BT_u8 *) free) + pCache->ulObjectSize);
	}

	free = (struct block_free *) (((BT_u8 *) free) - pCache->ulObjectSize);	// Reverse to correct the end of block.
	free->next = pCache->free;
	pCache->free = (struct block_free *) (pBlock);
}

static BT_ERROR extend_cache(BT_CACHE *pCache) {

	BT_u32 ulSize = BT_PAGE_ALIGN((pCache->ulObjectSize + sizeof(struct block)));

	void *p = (void *) bt_page_alloc(ulSize);
	if(!p) {
		return BT_ERR_NO_MEMORY;
	}

	struct block *pBlock = (struct block *) bt_phys_to_virt(p);
	init_attach_block(pCache, pBlock, ulSize);

	return BT_ERR_NONE;
}


static BT_ERROR init_cache(BT_CACHE *pCache, BT_u32 ulObjectSize) {
	pCache->ulObjectSize = ulObjectSize;
	pCache->free = NULL;
	pCache->allocated = 0;

	extend_cache(pCache);

	return BT_ERR_NONE;
}

BT_ERROR BT_CacheInit(BT_CACHE *pCache, BT_u32 ulObjectSize) {
	pCache->slab_mutex = BT_kMutexCreate();
	return init_cache(pCache, ulObjectSize);
}

/*
BT_CACHE *BT_kCreateCache(BT_u32 ulObjectSize, BT_u32 ulFlags) {

}*/
/*
BT_ERROR BT_kDestroyCache(BT_CACHE *pCache) {

}*/

static BT_CACHE *BT_GetSuitableCache(BT_u32 ulSize) {

	if(ulSize <= BT_CACHE_GENERIC_MIN) {
		return &g_oDefault[0];
	}

	if(ulSize > BT_CACHE_GENERIC_MAX) {
		return NULL;
	}

	BT_u32 highest_bit = BT_CLZ(ulSize);
	if(ulSize & ((0x80000000 >> highest_bit)-1)) {	// Use size to get the index.
		highest_bit -= 1;
	}


	BT_u32 idx = BT_CLZ(BT_CACHE_GENERIC_MIN) - highest_bit;

	return &g_oDefault[idx];
}

static struct block_free *pop_free(BT_CACHE *pCache) {
	if(pCache->free) {
		struct block_free *p = pCache->free;
		pCache->free = pCache->free->next;
		pCache->allocated += 1;
		return p;
	}

	return NULL;
}

static void push_free(BT_CACHE *pCache, struct block_free *p) {
	p->next = pCache->free;
	pCache->free = p;
	pCache->allocated -= 1;
}

void *BT_CacheAlloc(BT_CACHE *pCache) {

	SLAB_LOCK(pCache);

	struct block_free *p = pop_free(pCache);
	if(!p) {
		extend_cache(pCache);
		p = pop_free(pCache);
		if(!p) {
			SLAB_UNLOCK(pCache);
			return NULL;
		}
	}

	SLAB_UNLOCK(pCache);

	return (void *) p;
}

BT_ERROR BT_CacheFree(BT_CACHE *pCache, void *p) {
	SLAB_LOCK(pCache);
	struct block_free *free = (struct block_free *) p;
	push_free(pCache, free);
	SLAB_UNLOCK(pCache);
	return BT_ERR_NONE;
}

void *BT_kMalloc(BT_u32 ulSize) {

	void *p;

	if(!ulSize) {
		return NULL;
	}

	BT_CACHE *pCache = BT_GetSuitableCache(ulSize+sizeof(BT_CACHE *));
	if(pCache) {
		p = BT_CacheAlloc(pCache);
		BT_CACHE **tag = (BT_CACHE **) p;
		*tag = pCache;
		return ((void *) (tag + 1));
	} else {
		p = (void *) bt_phys_to_virt(bt_page_alloc(ulSize+sizeof(struct _PAGE_ALLOC)));
		if(!p) {
			return NULL;
		}
	}

	struct _PAGE_ALLOC *tag = (struct _PAGE_ALLOC *) p;
    tag->null = NULL;
	tag->size = ulSize;

	/*
	 *	Before the allocated memory we place a pointer to the pCache.
	 *	This will be 0 in the case of a page allocation!
	 */

	return ((void *) (tag + 1));
}

void BT_kFree(void *p) {
	BT_CACHE **tag = (BT_CACHE **) p;
	tag -= 1;

	BT_CACHE *pCache = *tag;
	if(pCache) {
		BT_CacheFree(pCache, tag);
	} else {
		struct _PAGE_ALLOC *alloc = (struct _PAGE_ALLOC *) p;
		alloc -= 1;
		bt_page_free((BT_PHYS_ADDR) bt_virt_to_phys(tag), alloc->size);
	}
}

void bt_initialise_slab() {

	BT_u32 i = BT_CACHE_GENERIC_MIN;
	while(i <= BT_CACHE_GENERIC_MAX) {
		BT_CACHE *pCache = BT_GetSuitableCache(i);
		if(!pCache) {
			break;
		}
		init_cache(pCache, i);
		pCache->slab_mutex = NULL;
		i = i << 1;
	}
}

void bt_initialise_slab_second_stage() {

	BT_u32 i = BT_CACHE_GENERIC_MIN;
	while(i <= BT_CACHE_GENERIC_MAX) {
		BT_CACHE *pCache = BT_GetSuitableCache(i);
		if(!pCache) {
			break;
		}
		pCache->slab_mutex = BT_kMutexCreate();
		i = i << 1;
	}
}
