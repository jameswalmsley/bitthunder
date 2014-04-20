/**
 *	@brief		SLAB Allocator
 *
 *	Provides the kernel with a very simple, but very fast SLAB memory allocator.
 *	The HEAP allocator can use this to provide a speed efficient heap implementation to the kernel.
 *
 *	@file		slab.h
 *	@author		James Walmsley <james@fullfat-fs.co.uk>
 *	@ingroup	MM
 *
 *	@copyright	(c) 2013 James Walmsley
 **/
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

/**
 *	@public
 *	@brief	Initialise a BT_CACHE object for memory allocation.
 *
 *	@pCache[in]			Pointer to the BT_CACHE object to be initialised.
 *	@ulCacheSize[in]	Size of the objects that this SLAB cache should contain.
 *
 *	@return BT_ERR_NONE on success.
 **/
BT_ERROR BT_CacheInit(BT_CACHE *pCache, BT_u32 ulCacheSize);

/**
 *	@public
 *	@brief	Allocates an object from the specified SLAB cache.
 *
 *	@pCache[in]		The SLAB cache from which to allocate the memory object.
 *
 *	@return	A pointer to the allocated memory.
 **/
void *BT_CacheAlloc(BT_CACHE *pCache);

/**
 *	@public
 *	@brief	Free's an object back to the specified SLAB cache.
 *
 *	@pCache[in]		The SLAB cache to return the memory object back to.
 *	@ptr[in]		The pointer to the memory object being returned to the SLAB cache.
 *	
 *	@return	BT_ERR_NONE on success.
 **/
BT_ERROR BT_CacheFree(BT_CACHE *pCache, void *ptr);

#endif
