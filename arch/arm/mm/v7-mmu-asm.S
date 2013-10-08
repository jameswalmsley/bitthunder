.globl bt_mmu_get_ttb
bt_mmu_get_ttb:
	mcr p15, 0, r0, c2, c0, 0
	mov	pc, lr

