#ifndef _SLAB_H_
#define _SLAB_H_

typedef struct _BT_CACHE {
	struct bt_list_head		blocks;				///< List of blocks asociated with this cache.
	BT_u32					ulObjectSize;
	struct block_free  	   *free;
	BT_u32					allocated;
} BT_CACHE;

void bt_initialise_slab();

BT_ERROR BT_CacheInit(BT_CACHE *pCache);
void *BT_CacheAlloc(BT_CACHE *pCache);
BT_ERROR BT_CacheFree(BT_CACHE *pCache, void *p);

#endif
