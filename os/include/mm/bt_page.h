/**
 *	@brief		Page Allocator
 *
 *	Provides the Kernel with a simple API for managing PAGE pools.
 *	By default the Kernel will initialise a main PAGE pool. This is used for almost
 *	all memory allocations. Optionally a coherent PAGE pool may be created.
 *
 *	A coherent page pool can be used for DMA, i.e. uncached and aligned memory.
 *
 *	Furthermore, memory can be reserved from any of the main pools, and attached to
 *	a new page pool. Some applications or exotic drivers may find this service useful.
 *
 *	@file		bt_page.h
 *	@author		James Walmsley <james@fullfat-fs.co.uk>
 *	@ingroup	MM
 *
 *	@copyright	(c) 2013 James Walmsley
 **/
#ifndef _BT_PAGE_H_
#define _BT_PAGE_H_

#include <bt_config.h>
#include <bt_types.h>
#include <collections/bt_list.h>

/**
 *	@brief	A PAGE pool.
 **/
struct bt_page_pool {
	struct bt_list_head 	page_head;		///< List of pages within this PAGE pool.
	BT_u32					total_size;		///< Total size of the PAGE pool.
	BT_u32					used_size;		///< Total size of allocated pages from this PAGE pool.
};

/**
 *	@brief	PAGE pool information structure.
 **/
struct bt_page_info {
	BT_u32 normal_size;						///< Total size of the main PAGE pool.
	BT_u32 normal_used;						///< Total size of PAGEs allocated from the main PAGE pool.
	BT_u32 coherent_size;					///< Total size of the coherent PAGE pool.
	BT_u32 coherent_used;					///< Total size of PAGEs allocated from the coherent PAGE pool.
};

/**
 *	@kernel
 *	@public
 *	@brief	Allocates PAGES from the main PAGE pool with complete freedom.
 *
 *	This is the most commonly used PAGE allocator API. It allocates with complete
 *	freedom, the only constraint being the specified size (psize).
 *
 *	@param[in]	psize	The size in bytes that the allocation MUST contain.
 *
 *	@return	Physical address of the allocated PAGE(s).
 *	@return NULL (0) if allocation fails.
 **/
bt_paddr_t bt_page_alloc(BT_u32 psize);

/**
 *	@kernel
 *	@public
 *	@brief	Allocates PAGES from the main PAGE pool with constrained alignment.
 *
 *	This can be used to allocate pages with a particular alignment.
 *
 *	@param[in]	psize	The size in bytes that the allocation MUST contain.
 *	@param[in]	order	The alignment order. (0 = PAGE_ALIGNED, 1 = 2*PAGE_ALIGNED, 2=4*PAGE_ALIGNED etc).
 *
 *	@return	Physical address of the allocated PAGE(s).
 *	@return NULL (0) if allocation fails.
 **/
bt_paddr_t bt_page_alloc_aligned(BT_u32 psize, BT_u32 order);

/**
 *	@kernel
 *	@public
 *	@brief	Free's PAGES to the main PAGE pool.
 *
 *	This is the most commonly used PAGE free API.
 *
 *	@param[in]	paddr	Physical address of the PAGES to be free'd.
 *	@param[in] 	psize	Size in bytes of the PAGES being free'd
 *
 *	@void
 **/
void bt_page_free(bt_paddr_t paddr, BT_u32 psize);

/**
 *	@kernel
 *	@public
 *	@brief	Reserve's PAGES from the main PAGE pool.
 *
 *	@param[in]	paddr	Physical address of the PAGES to be reserved.
 *	@param[in]	psize	Size in bytes that the reservation must contain. (Rounded up to BT_PAGE_SIZE internally).
 *
 *	@return BT_ERR_NONE on success.
 *	@return	BT_ERR_NO_MEMORY if reservation fails.
 **/
BT_ERROR bt_page_reserve(bt_paddr_t paddr, BT_u32 psize);

#ifdef BT_CONFIG_MEM_PAGE_COHERENT_POOL
/**
 *	@kernel
 *	@public
 *	@brief	Allocates PAGES from the coherent PAGE pool.
 *
 *	@param[in]	psize	The size in bytes that the allocation MUST contain.
 *
 *	@return Physical address of the allocated PAGE(s).
 *	@return NULL (0) if allocation fails.
 **/
bt_paddr_t bt_page_alloc_coherent(BT_u32 psize);

/**
 *	@kernel
 *	@public
 *	@brief	Free's PAGES to the coherent PAGE pool.
 *
 *	@param[in]	paddr	Physical address of the PAGES to be free'd.
 *	@param[in]	psize	Size in bytes of the PAGES being free'd.
 *
 *	@void
 **/
void bt_page_free_coherent(bt_paddr_t paddr, BT_u32 psize);

/**
 *	@kernel
 *	@public
 *	@brief	Reserve's PAGES from the coherent PAGE pool.
 *
 *	@param[in]	paddr	Physical address of the PAGES to be reserved.
 *	@param[in]	psize	Size in bytes that reservation must contain. (Rounded up to BT_PAGE_SIZE internally).
 *
 *	@return BT_ERR_NONE on success.
 *	@return BT_ERR_NO_MEMORY if reservation fails.
 **/
BT_ERROR bt_page_reserve_coherent(bt_paddr_t paddr, BT_u32 psize);
#endif

/**
 *	@kernel
 * 	@public
 *	@brief	Get Information on the current state of the PAGE manager.
 *
 *	@param[out]	pInfo	A pointer to a struct bt_page_info to be filled with the current state.
 *
 *	@return BT_ERR_NONE on success.
 **/
BT_ERROR bt_page_info(struct bt_page_info *pInfo);

/**
 *	@kernel
 *	@public
 *	@brief	Initialise a PAGE pool.
 *
 *	Initialises a @ref (struct bt_page_pool) object with 0 attached pages of memory.
 *
 *	@param[in]	pool	PAGE pool object to be initialised.
 *
 *	@return	BT_ERR_NONE on sucess.
 **/
BT_ERROR bt_page_pool_init(struct bt_page_pool *pool);

/**
 *	@kernel
 *	@public
 *	@brief	Allocates some pages from a PAGE pool.
 *
 *	@param[in]	pool	PAGE pool to allocate pages from.
 *	@param[in]	psize	Size in bytes the allocation should atleast contain. (Rounded up to BT_PAGE_SIZE internally).
 *	@param[in]	order	The alignment order. (0 = PAGE_ALIGNED, 1 = 2*PAGE_ALIGNED, 2=4*PAGE_ALIGNED etc).
 *
 *	@return A physical PAGE address if allocation succeeds.
 *	@return	NULL (0) if allocation fails.
 **/
bt_paddr_t bt_page_pool_alloc(struct bt_page_pool *pool, BT_u32 psize, BT_u32 order);

/**
 *	@kernel
 *	@public
 *	@brief	Free's an allocation into the specified PAGE pool.
 *
 *	Places memory back onto the specified PAGE pool.
 *
 *	@warn The API does not ensure that pages are being placed back into the same PAGE pool from whence they were allocated.
 *	@warn The correct way to "migrate" pages is to reserve them in the originating PAGE pool, and attach them to another pool.
 *
 *	@param[in]	pool	PAGE pool to free the memory into.
 *	@param[in]	paddr	Physical address of the pages to be free'd.
 *	@param[in]	size	Size in bytes of the pages being free'd. (Rounded up to BT_PAGE_SIZE internally).
 *
 *	@void
 **/
void bt_page_pool_free(struct bt_page_pool *pool, bt_paddr_t paddr, BT_u32 size);

/**
 *	@kernel
 *	@public
 *	@brief	Attaches some PAGES to the specified PAGE pool.
 *
 *	The difference between pool_free and pool_attach is that attach operations initialise / update
 *	the PAGE pool's information on how much memory is available or in use.
 *
 *	@param[in]	pool	PAGE pool to attach the memory to.
 *	@param[in]	paddr	Physical address of the pages to be attached.
 *	@param[in]	size	Size in bytes of the pages being attached. (Rounded up to BT_PAGE_SIZE internally).
 *
 *	@void
 **/
void bt_page_pool_attach(struct bt_page_pool *pool, bt_paddr_t paddr, BT_u32 size);

/**
 *	@kernel
 *	@public
 *	@brief	Reserves some PAGES from the specified PAGE pool.
 *
 *	Reserves a linear region within a PAGE pool. After this operation it is expected that the
 *	memory will never be returned to the PAGE pool. This is not meant to be used as an allocation
 *	method, but to remove e.g. PAGES containing Kernel data or code from a PAGE pool.
 *
 *	@param[in]	pool	PAGE pool to reserve memory from.
 *	@param[in]	paddr	Physical address of the PAGES to be attached.
 *	@param[in]	psize	Size in bytes of the pages to be reserved. (Rounded up to BT_PAGE_SIZE internally).
 *
 *	@return	BT_ERR_NONE on success.
 *	@return	BT_ERR_NO_MEMORY if reservation fails.
 **/
BT_ERROR bt_page_pool_reserve(struct bt_page_pool *pool, bt_paddr_t paddr, BT_u32 psize);

/**
 *	@kernel
 *	@private
 *
 *	@brief	Initialises the PAGE manager, and creates the default PAGE pools.
 **/
void bt_initialise_pages(void);

/**
 *	@kernel
 *	@private
 *
 *	@brief	Initialises the PAGE manager's MUTEX.
 *
 *	This is done separately because there is a short period of time where the PAGE
 *	manager is required, but MUTEX's cannot be used.
 *
 *	@future	Hopefully this requirement can be removed in the future.
 **/
void bt_initialise_pages_second_stage(void);

/**
 *	@kernel
 *	@private
 *
 *	@brief	Initialises the Kernel's coherent PAGE pool.
 **/
bt_paddr_t bt_initialise_coherent_pages(void);

#endif
