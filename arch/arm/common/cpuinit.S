#include <bt_config.h>

		.global __cpu_init
	.align 2
__cpu_init:

	@ Initialise the CP15 registers after reset.
	mov	r1, #0x0
	mcr	p15, 0, r1, c5, c0, 0		@ DFSR
	mcr	p15, 0, r1, c5, c0, 1		@ IFSR
	mcr	p15, 0, r1, c6, c0, 0		@ DFAR
	mcr	p15, 0, r1, c6, c0, 2		@ IFAR
#ifdef BT_CONFIG_ARCH_ARM_ARMv7
	mcr	p15, 0, r1, c9, c13, 2		@ PMXEVCNTR
#endif
	mcr	p15, 0, r1, c13, c0, 2		@ TPIDRURW
	mcr	p15, 0, r1, c13, c0, 3		@ TPIDRURO
	mcr	p15, 5, r1, c15, c5, 2		@ Write Lockdown TLB VA

	@Reset and start Cycle Counter
	mov	r2, #0x80000000				@ clear overflow
	mcr	p15, 0, r2, c9, c12, 3
	mov	r2, #0xd					@ D, C, E
	mcr	p15, 0, r2, c9, c12, 0
	mov	r2, #0x80000000				@ enable cycle counter
	mcr	p15, 0, r2, c9, c12, 1

	bx	lr

.end
