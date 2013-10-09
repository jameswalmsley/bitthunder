/**
 *	ARM MMU
 *	-- MMUTable
 **/

#include <bitthunder.h>
#include <string.h>
#include <asm/barrier.h>
#include <mm/slab.h>
#include <mm/bt_vm.h>
#include "v7-mmu.h"

#define MMU_L1TBL_MASK	(MMU_L1TBL_SIZE - 1)
#define PGD_ALIGN(x)	((((bt_paddr_t)(x)) + MMU_L1TBL_MASK) & ~MMU_L1TBL_MASK)

static BT_CACHE g_ptCache;	// Slab cache for page tables.


static void flush_tlb(void) {
	__asm volatile("mcr p15, 0, r0, c8, c7, 0");
	dsb();
}

static void switch_ttb(bt_paddr_t pgd) {
	// flush I cache?
	// flush D cache?
	__asm volatile("mcr p15, 0, r0, c2, c0, 0");
	__asm volatile("mcr p15, 0, r0, c8, c7, 0");
	flush_tlb();
}

/**
 *	The actual MMU Table.
 **/

BT_ATTRIBUTE_SECTION(".bt.mmu.table") static bt_pgd_t g_MMUTable[4096];
BT_ATTRIBUTE_SECTION(".bt.mmu.table") static bt_pte_t kernel_pages[1024][256];	// Kernel pages. - 1GB of 1MB regions, with 256 pages per mb.

/*void bt_arch_mmu_setsection(BT_IOMAP *pMapping) {
	BT_u32 section = (BT_u32) pMapping->phys;
	section |= 0x0c02;

	BT_u32 index = (BT_u32) pMapping->addr / 0x00100000;
	g_MMUTable[index] = section;

	//BT_DCacheFlush();

	__asm volatile("mcr	p15, 0, r0, c8, c7, 0");	// invalidate TLBs

	dsb();

	// Flush TLB!
}*/

static bt_paddr_t create_pgd(void) {
	bt_paddr_t pg, pgd;

	pg = bt_page_alloc(MMU_L1TBL_SIZE * 2);
	if(!pg) {
		return 0;
	}

	pgd = PGD_ALIGN(pg);
	// Here we should free the uneeded (unaligned part), but this requires
	// some small changes to the page allocator api.

	return pgd;
}

int bt_mmu_map(bt_pgd_t pgd, bt_paddr_t pa, bt_vaddr_t va, BT_u32 size, int type) {
	BT_u32 flag = 0;
	bt_pte_t pte;
	bt_paddr_t pg;

	pa = BT_PAGE_TRUNC(pa);		// Ensure correct alignments.
	va = BT_PAGE_TRUNC(va);
	size = BT_PAGE_ALIGN(size);

	switch(type) {				// Build up the ARM MMU flags from BT page types.
	case BT_PAGE_UNMAP:
		flag = 0;
		break;

	case BT_PAGE_READ:
		flag = (BT_u32) (MMU_PTE_PRESENT | MMU_PTE_WBUF | MMU_PTE_CACHE | MMU_PTE_USER_RO);
		break;

	case BT_PAGE_WRITE:
		flag = (BT_u32) (MMU_PTE_PRESENT | MMU_PTE_WBUF | MMU_PTE_CACHE | MMU_PTE_USER_RW);
		break;

	case BT_PAGE_SYSTEM:
		flag = (BT_u32) (MMU_PTE_PRESENT | MMU_PTE_WBUF | MMU_PTE_CACHE | MMU_PTE_SYSTEM);
		break;

	case BT_PAGE_IOMEM:
		flag = (BT_u32) (MMU_PTE_PRESENT | MMU_PTE_SYSTEM);
		break;

	default:
		//do_kernel_panic("bt_mmu_map");
		return -1;
		break;
	}

	flush_tlb();

	while(size > 0) {
		if(pte_present(pgd, va)) {
			pte = virt_to_pte(pgd, va);		// Get the page table from PGD.
		} else {
			pg = (bt_paddr_t) BT_CacheAlloc(&g_ptCache);
			if(!pg) {
				return -1;
			}

			pgd[PAGE_DIR(va)] = (BT_u32) pg | MMU_PDE_PRESENT;
			pte = bt_phys_to_virt(pg);
			memset(pte, 0, MMU_L2TBL_SIZE);
		}

		pte[PAGE_TABLE(va)] = (BT_u32) pa | flag;

		pa += BT_PAGE_SIZE;
		va += BT_PAGE_SIZE;
		size -= BT_PAGE_SIZE;
	}

	flush_tlb();

	return 0;
}

bt_pgd_t bt_mmu_newmap(void) {
	bt_paddr_t pg;
	bt_pgd_t pgd;

	pg = create_pgd();
	if(!pg) {
		return 0;
	}

	pgd = bt_phys_to_virt(pg);
	memset(pgd, 0, MMU_L1TBL_SIZE);

	/*
	 *	At this point the kernel page table will point to valid page tables,
	 *	that were created during the bt_mmu_initialise routine.
	 *
	 *	The user-space section should all be 0, i.e. cause page faults.
	 *	This means process VMs always match the kernel ptes correctly,
	 *	as the kernel pgd will never be updated after mmu initialisation.
	 */
	memcpy(pgd, g_MMUTable, MMU_L1TBL_SIZE);

	return pgd;
}

void bt_mmu_terminate(bt_pgd_t pgd) {

	int i;
	bt_pte_t pte;

	flush_tlb();

	// Release all user page tables.
	for(i = 0; i < PAGE_DIR(0xC0000000); i++) {
		pte = (bt_pte_t) pgd[i];
		if(pte) {
			BT_CacheFree(&g_ptCache, (void *) ((BT_u32) pte & MMU_PTE_ADDRESS));
		}
	}

	//bt_page_free(bt_virt_to_phys(pgd));	// Page allocator must accept size to do this.
}

extern bt_paddr_t bt_mmu_get_ttb(void);

static bt_paddr_t current_user_ttb(void) {
	return bt_mmu_get_ttb();
}

void bt_mmu_switch_user(bt_pgd_t pgd) {
	bt_paddr_t phys = (bt_paddr_t) bt_virt_to_phys(pgd);

	if(phys != current_user_ttb()) {
		switch_ttb(phys);
	}
}


bt_paddr_t bt_mmu_extract(bt_pgd_t pgd, bt_vaddr_t virt, BT_u32 size) {
	bt_pte_t pte;
	bt_vaddr_t start, end, pg;
	bt_paddr_t pa;

	start = BT_PAGE_TRUNC(virt);
	end = BT_PAGE_TRUNC(virt+size-1);

	// Check all pages exist.
	for(pg = start; pg <= end; pg += BT_PAGE_SIZE) {
		if(!pte_present(pgd, pg)) {
			return 0;
		}

		pte = virt_to_pte(pgd, pg);
		if(!page_present(pte, pg)) {
			return 0;
		}
	}

	pte = virt_to_pte(pgd, start);
	pa = (bt_paddr_t) pte_to_phys(pte, start);

	return pa + (bt_paddr_t) (virt - start);
}

void bt_mmu_init(struct bt_mmumap *mmumap) {
	BT_CacheInit(&g_ptCache, MMU_L2TBL_SIZE);	// Create a cache of 1K, 1K aligned page tables.
	// Set-up proper kernel page-tables, so that the super-sections will always be valid and can be copied directly to
	// The process PGD's on creation.

	bt_pgd_t 	pgd 	= (bt_pgd_t) g_MMUTable;
	BT_u32 		index;

	for(index = (0xC0000000 / 0x00100000); index < 0x1000; index++) {
		bt_pte_t pte;
		pte = (bt_pte_t) &kernel_pages[index-0xC00];
		memset(pte, 0, MMU_L2TBL_SIZE);

		// Setup all the pages with an identity mapping for this region.
		bt_paddr_t pa = (index * 0x00100000);
		BT_u32 i;
		for(i = 0; i < 0x00100000 / 4096; i++) {
			BT_u32 section = pgd[index];
			BT_u32 flag = 0;
			if(section) {
				// It must be a mapping to memory, therefore do normal kernel mode caching.
				//	IOMAPPINGs will be added later during driver probes.
				flag = MMU_PTE_PRESENT | MMU_PTE_WBUF | MMU_PTE_CACHE | MMU_PTE_SYSTEM;
			}
			pte[i] = (BT_u32) bt_virt_to_phys(pa+(4096 * i)) | flag;
		}

		// Page Table is now valid. We can make the pgd point to it for these regions.
		pgd[index] = (bt_paddr_t) bt_virt_to_phys(pte) | MMU_PDE_PRESENT;
	}

	flush_tlb();
}


bt_pgd_t bt_mmu_get_kernel_pgd(void) {
	return (bt_pgd_t) g_MMUTable;
}
