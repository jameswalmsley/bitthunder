#include <asm/assembly.h>

.globl bt_mmu_get_ttb
bt_mmu_get_ttb:
	mcr p15, 0, r0, c2, c0, 0
	mov	pc, lr

.globl bt_mmu_switch_ttb
bt_mmu_switch_ttb:
	orr	r0, r0, #0x5B				// Outer Cacheable, WB
    mcr p15, 0, r0, c2, c0, 0   	/* set the new TTB */
    mcr p15, 0, r0, c8, c7, 0   	/* and flush the I+D tlbs */
	mcr p15, 0, r1, c13, c0, 1		// Write the ASID into contextidr
    data_sync
    mov pc, lr

.globl bt_mmu_flush_tlb
bt_mmu_flush_tlb:
    mcr p15, 0, r0, c8, c7, 0   	/* invalidate I+D TLBs */
    mov pc, lr
