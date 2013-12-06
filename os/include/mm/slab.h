#ifndef _SLAB_H_
#define _SLAB_H_

#define BT_SLAB_MAX_ORDER	BT_CONFIG_MEM_SLAB_MAX_ORDER

typedef struct _BT_CACHE {
	BT_u32					ulObjectSize;
	struct block_free  	   *free;
	BT_u32					allocated;
	BT_u32					available;
	void 				   *slab_mutex;
} BT_CACHE;

struct bt_cache_info {
	BT_u32 	ulObjectSize;
	BT_u32 	available;
	BT_u32	allocated;
};

struct bt_slab_info {
	struct bt_cache_info slabs[BT_SLAB_MAX_ORDER];
};

BT_ERROR bt_slab_info(struct bt_slab_info *pInfo);

void bt_initialise_slab();
void bt_initialise_slab_second_stage();

BT_ERROR BT_CacheInit(BT_CACHE *pCache, BT_u32 ulCacheSize);
void *BT_CacheAlloc(BT_CACHE *pCache);
BT_ERROR BT_CacheFree(BT_CACHE *pCache, void *p);

#endif
