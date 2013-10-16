#ifndef _SLAB_H_
#define _SLAB_H_

typedef struct _BT_CACHE {
	BT_u32					ulObjectSize;
	struct block_free  	   *free;
	BT_u32					allocated;
	void 				   *slab_mutex;
} BT_CACHE;

struct _PAGE_ALLOC {
	BT_u32	size;
	BT_CACHE *null;
};

void bt_initialise_slab();
void bt_initialise_slab_second_stage();

BT_ERROR BT_CacheInit(BT_CACHE *pCache, BT_u32 ulCacheSize);
void *BT_CacheAlloc(BT_CACHE *pCache);
BT_ERROR BT_CacheFree(BT_CACHE *pCache, void *p);

#endif
