/**
 *	@brief		Memory Manager - IO Mapping API.
 *
 *	Provides the Kernel with a simple API for mapping IO regions into the kernels VM mappings.
 *
 *	@file		bt_ioremap.h
 *	@author		James Walmsley <james@fullfat-fs.co.uk>
 *	@ingroup	MM
 *
 *	@copyright	(c) 2013 James Walmsley
 **/
#ifndef _BT_IOREMAP_H_
#define _BT_IOREMAP_H_

#include <bt_config.h>
#include <bt_types.h>

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
/**
 *	@kernel
 *	@public
 *	@brief	Maps the specified Physical address into the Kernel's memory space.
 *
 *	The IO mapper remaps any "physical" address region into the Kernel's upper virtual address space.
 *	That is that the mappings appear above the Kernel's physical PAGE regions.
 *	The mappings returned are uncached, and should only be used for MMIO (Memory Mapped IO) like
 *	access to peripheral registers etc.
 *
 *	@note	To get uncached access to large memory regions, use a coherent memory pool.
 *	@note	On bare-metal configurations (virtual memory is disabled) use of bt_ioremap() simply returns paddr.
 *
 *	@param[in]	paddr	The Physical address to be remapped.
 *	@param[in]	size	The size in bytes of the region that the mapping must contain.
 *
 *	@return	A pointer to the virtual address used to access the mapped physical region.
 *	@return	NULL if mapping failed.
 **/
void *bt_ioremap(void *paddr, BT_u32 size);

/**
 *	@kernel
 *	@public
 *	@brief	Unmaps the an IO mapping using the given Virtual address (as returned from bt_ioremap).
 *
 *	All mappings returned from bt_ioremap() utilise the kernel's limited virtual address space region.
 *	ioremapped regions MUST be free'd after use. Forgetting to unmap is similar to a memory leak.
 *
 *	@note 	On bare-metal configurations (virtual memory is disabled) use of bt_iounmap() is optimised away.
 *
 *	@param[in]	iomem	The Virtual address of the mapped region to be unmapped.
 *
 *	@void
 **/
void bt_iounmap(volatile void *iomem);
#else
#define bt_ioremap(x, y)	x
#define bt_iounmap(x)
#endif

#endif
