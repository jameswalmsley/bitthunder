#ifndef _ASSEMBLY_H_
#define _ASSEMBLY_H_

#include <bt_config.h>

/*
 *	Instruction barrier.
 */

    .macro  instr_sync
#ifdef BT_CONFIG_ARCH_ARM_ARMv7
	isb
#endif
#ifdef BT_CONFIG_ARCH_ARM_ARMv6
	mcr	p15, 0, r0, c7, c5, 4
#endif
	.endm

	.macro data_sync
#ifdef BT_CONFIG_ARCH_ARM_ARMv7
	 dsb
#endif
#ifdef BT_CONFIG_ARCH_ARM_ARMv6
		 mcr p15, 0, r0, c7, c10, 4
#endif
	.endm


#endif
