#ifndef _BT_MM_H_
#define _BT_MM_H_

#define BT_MM_USERLIMIT		0xC0000000
#define BT_TOTAL_PAGES		(BT_CONFIG_LINKER_RAM_LENGTH/BT_PAGE_SIZE)

#define BT_SECTION_SIZE		0x00100000
#define BT_SECTION_MASK		(BT_SECTION_SIZE-1)
#define BT_SECTION_ALIGN(x)	(((x) + BT_SECTION_MASK) & ~BT_SECTION_SIZE)
#define BT_PAGE_SIZE		4096
#define BT_PAGE_MASK		(BT_PAGE_SIZE-1)

#define BT_PAGE_TRUNC(x)	((x) & ~BT_PAGE_MASK)					// Round down to nearest PAGE alignment
#define BT_PAGE_ALIGN(x)	(((x) + BT_PAGE_MASK) & ~BT_PAGE_MASK)	// Round up to nearest PAGE alignment

#define bt_phys_to_virt(phys_addr)	(bt_vaddr_t) ((bt_vaddr_t) (phys_addr) - BT_CONFIG_RAM_PHYS + BT_CONFIG_RAM_VIRT)
#define bt_virt_to_phys(virt_addr)	(bt_paddr_t) ((bt_paddr_t) (virt_addr) - BT_CONFIG_RAM_VIRT + BT_CONFIG_RAM_PHYS)

#endif
