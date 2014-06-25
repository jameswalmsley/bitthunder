/**
 *	@brief		Memory Manager	- Definitions and Macros
 *
 *	Provides a scalable Memory Manager to the BitThunder kernel.
 *
 *	@file		bt_mm.h
 *	@author		James Walmsley	<james@fullfat-fs.co.uk>
 *	@ingroup	MM
 *
 *	@copyright	(c) 2013 James Walmsley
 **/
/**
 *	@defgroup	MM	Memory Manager
 *	@brief		Kernel-level API for Memory Management.
 *
 *	The memory manager in BitThunder differs based on configuration. In its simplest
 *	form, it provides a basic heap as a linear list. This is a good memory manager
 *	for small devices like micro-controllers.
 *
 *	In its most advanced form the memory manager provides a fully featured page-based
 *	virtual memory manager. This is a good memory manager for applications replacing
 *	a monolithic kernel like linux.
 **/
#ifndef _BT_MM_H_
#define _BT_MM_H_

#define BT_PAGE_SIZE		4096

#define BT_MM_USERLIMIT		0xC0000000
#define BT_TOTAL_PAGES		(BT_CONFIG_LINKER_RAM_LENGTH/BT_PAGE_SIZE)

#define BT_SECTION_SIZE		0x00100000
#define BT_SECTION_MASK		(BT_SECTION_SIZE-1)

#define BT_SECTION_TRUNC(x)	((x) & ~BT_SECTION_MASK)
#define BT_SECTION_ALIGN(x)	(((x) + BT_SECTION_MASK) & ~BT_SECTION_MASK)

#define BT_PAGE_MASK		(BT_PAGE_SIZE-1)

#define BT_PAGE_TRUNC(x)	((x) & ~BT_PAGE_MASK)					// Round down to nearest PAGE alignment
#define BT_PAGE_ALIGN(x)	(((x) + BT_PAGE_MASK) & ~BT_PAGE_MASK)	// Round up to nearest PAGE alignment

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
#define bt_phys_to_virt(phys_addr)	(bt_vaddr_t) ((bt_vaddr_t) (phys_addr) - BT_CONFIG_RAM_PHYS + BT_CONFIG_RAM_VIRT)
#define bt_virt_to_phys(virt_addr)	(bt_paddr_t) ((bt_paddr_t) (virt_addr) - BT_CONFIG_RAM_VIRT + BT_CONFIG_RAM_PHYS)
#else
#define bt_phys_to_virt(phys_addr)	(phys_addr)
#define bt_virt_to_phys(virt_addr)	(virt_addr)
#endif

#endif
