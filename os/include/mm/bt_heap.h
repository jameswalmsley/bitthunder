/**
 *	@brief		Kernel Heap Allocator
 *
 *	Provides the Kernel with a simple HEAP for allocating small or irregular sizes of memory.
 *	The is the most commonly used memory allocation API that should be used in the kernel,
 *	it works exactly like the standard malloc() and free() functions.
 *
 *	@note	Application code that is linked to the Kernel directly (rather than compiled for userspace)
 *	@note	should can make use of these functions, but doing so reduces the portability of your code.
 *	@note	Simply use malloc() and free() in "application" code, allowing it to be used in kernel and 
 *	@note	userspace without issues.
 *
 *	@file		bt_heap.h
 *	@author		James Walmsley <james@fullfat-fs.co.uk>
 *	@ingroup	MM
 *
 *	@copyright	(c) 2013 James Walmsley
 **/

#ifndef _BT_HEAP_H_
#define _BT_HEAP_H_

#ifdef BT_CONFIG_TRACE_MALLOC
#define BT_TRACE_MALLOC
#endif

/**
 *	@kernel
 *	@public
 *	@brief	Allocates (atleast) the specified number of bytes of memory.
 *
 *	Be aware, that the actual memory overhead of the allocation will be greater than the requested
 *	allocation size. The speed, and memory efficiency of this heap depends upon which memory
 *	management implementation your kernel is configured with.
 *
 *	@param[in]	ulSize	The size in bytes of the requested memory allocation.
 *
 *	@return	Pointer to the allocated memory, this is always a usable address.
 *	@return NULL (0) if allocation fails.
 **/
void *BT_kMalloc(BT_u32 ulSize);
#ifdef BT_TRACE_MALLOC
#define BT_kMalloc(_size) \
	({\
		void *p = BT_kMalloc(_size);\
		BT_kPrint("%s:%d :: BT_kMalloc(%d) : %p", __func__, __LINE__, _size, p);\
		p;\
	})
#endif

/**
 *	@kernel
 *	@public
 *	@brief	Free's a previously allocated memory via its original pointer.
 *
 *	@warn	Passing an invalid pointer (other than NULL) or a double free WILL cause a Kernel
 *	@warn	panic in BitThunder. (When using the SLAB allocator, the simple heap will just corrupt).
 *
 *	@param[in]	ptr	Pointer to the memory to be free'd.
 **/
void BT_kFree(void *ptr);
#ifdef BT_TRACE_MALLOC
#define BT_kFree(_p)\
	({\
		BT_kPrint("%s:%d :: BT_kFree(%p)", __func__, __LINE__, _p);\
		BT_kFree(_p);\
	})
#endif
/**
 *	@kernel
 *	@public
 * 	@brief	Re-allocates / allocates memory.
 *
 **/
void *BT_kRealloc(void *p, BT_u32 ulSize);

#endif
