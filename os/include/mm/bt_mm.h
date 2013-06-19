#ifndef _BT_MM_H_
#define _BT_MM_H_

#define BT_SECTION_SIZE		0x00100000
#define BT_SECTION_MASK		~(BT_SECTION_SIZE-1)
#define BT_PAGE_SIZE		4096
#define BT_PAGE_MASK		~(BT_PAGE_SIZE-1)
#define BT_PAGE_ALIGN(addr)

extern void bt_arch_mmu_setsection(BT_IOMAP *pMapping);







#endif
