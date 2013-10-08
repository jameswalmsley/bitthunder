#ifndef _V7_MMU_H_
#define _V7_MMU_H_

#define MMU_L1TBL_SIZE 		0x4000		//	16 	KB	(Entries cover 1MB ranges, and 4096 * 1MB = 32-bit address space [4GB]).
#define MMU_L2TBL_SIZE		0x400		// 	 1	KB	(Entries cover 4KB ranges, and 256 * 4K = 1MB)

#define	MMU_FAULT_MASK		0x00000003
#define MMU_FAULT			0x00000000

/**
 *	Defines the BitMask patterns for a section permissions.
 **/
#define MMU_SECTION_MASK	0x00040002
#define MMU_SECTION			0x00040002
#define MMU_SECTION_BASE	0xFFF00000
#define MMU_SECTION_NS		0x00080000	///< Shareable Bit.
#define MMU_SECTION_nG		0x00020000	///< Non-global section, e.g. Global and Process attributes.
#define MMU_SECTION_S		0x00010000
#define MMU_SECTION_AP_2	0x00008000
#define MMU_SECTION_TEX		0x00007000
#define MMU_SECTION_AP_1_0	0x00000C00
#define MMU_SECTION_IMP 	0x00000200
#define MMU_SECTION_DOMAIN	0x000001E0
#define MMU_SECTION_XN		0x00000010
#define MMU_SECTION_C		0x00000008
#define MMU_SECTION_B		0x00000004
#define MMU_SECTION_PXN		0x00000001

#define MMU_PDE_PRESENT		0x00000001
#define MMU_PDE_ADDRESS		0xFFFFFC00

/**
 *	L2 Entry masks: Defines the BitMask patterns required to point to a PageTable.
 **/
#define MMU_PTE_PRESENT		0x00000002
#define MMU_PTE_WBUF 		0x00000004
#define MMU_PTE_CACHE		0x00000008

#define MMU_PTE_AP_MASK		0x00000030
#define MMU_PTE_SYSTEM		0x00000010
#define MMU_PTE_USER_RO		0x00000020
#define MMU_PTE_USER_RW 	0x00000030
#define MMU_PTE_ADDRESS		0xFFFFFC00	///< Page table must appear at a 1Kb offset.

#define MMU_PT_IMP			0x00000200
#define MMU_PT_DOMAIN		0x000001E0
#define MMU_PT_SBZ			0x00000010
#define MMU_PT_NS			0x00000080	///< Non-secure bit.
#define MMU_PT_PXN			0x00000040	///< Execure Never bit.

/*
 *	Get index into the PAGE DIRECTORY
 *	1MB sections, each PGD has upto 4096 entries (0 .. 0xFFF).
 */
#define PAGE_DIR(virt)			(BT_u32)((((bt_vaddr_t)(virt)) >> 20) & 0xfff)

/*
 *	Get index into a PAGE TABLE.
 *	4KB pages, each PT has up to 256 entries, (0 .. 0xFF).
 */
#define PAGE_TABLE(virt)		(BT_u32)((((bt_vaddr_t)(virt)) >> 12) & 0xff)

#define pte_present(pgd, virt)	(pgd[PAGE_DIR(virt)] & MMU_PDE_PRESENT)
#define page_present(pte, virt)	(pte[PAGE_TABLE(virt)] & MMU_PTE_PRESENT)

#define virt_to_pte(pgd, virt)	(bt_pte_t) bt_phys_to_virt((pgd)[PAGE_DIR(virt)] & MMU_PDE_ADDRESS)
#define pte_to_phys(pte, virt)	((pte)[PAGE_TABLE(virt)] & MMU_PTE_ADDRESS)

#endif
