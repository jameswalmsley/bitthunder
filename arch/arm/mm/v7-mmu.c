/**
 *	ARM MMU
 *	-- MMUTable
 **/

#include <bitthunder.h>

/**
 *	The actual MMU Table.
 **/

BT_ATTRIBUTE_SECTION(".bt.mmu.table") static BT_u32 g_MMUTable[4096];

void bt_arch_mmu_setsection(BT_IOMAP *pMapping) {
	BT_u32 section = (BT_u32) pMapping->phys;
	section |= 0x0c02;

	BT_u32 index = (BT_u32) pMapping->addr / 0x00100000;
	g_MMUTable[index] = section;

	//BT_DCacheFlush();

	__asm volatile("mcr	p15, 0, r0, c8, c7, 0");		/* invalidate TLBs */

	__asm volatile("dsb");
	//__asm volatile("isb");

	// Flush TLB!
}
