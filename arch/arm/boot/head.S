/**
 *	BitThunder Kernel Entry for ARM architectures.
 */

#include <bt_config.h>

.section .init
.globl bt_reset

.extern __bt_mmu_table_start

.set MMUCRVAL,	0b01000000000101	/* Enable IDC, and MMU */

_bt_vector_table:
	ldr pc,=0x00100040
	ldr	pc,handler_undefined
	ldr pc,handler_swi
	ldr pc,handler_prefetch
	ldr pc,handler_data
	ldr pc,handler_unused		;@	Historical Handler - Was invalid address from the old 26-bit ARM CPUs
	ldr pc,handler_irq
	ldr pc,handler_fiq

handler_reset:		.word 	bt_reset
handler_undefined:	.word 	undefined
handler_swi:		.word 	vPortYieldProcessor
handler_prefetch:	.word	prefetch
handler_data:		.word	data
handler_unused:		.word	hang
handler_irq:		.word 	vFreeRTOS_IRQInterrupt
handler_fiq:		.word	fiq

bt_reset:
	/*
		Disable the IRQs as soon as possible, just in-case the boot-loader was naughty!
	*/

	mrc	p15,0,r1,c0,c0,5
	and r1, r1, #0xF		// Get the CPUID

	mrs	r0,cpsr
	orr	r0,r0,#0x80
	msr	cpsr_c,r0

	/*set scu enable bit in scu*/
	ldr	r7, =0xf8f00000
	ldr	r0, [r7]
	orr	r0, r0, #0x1
	str	r0, [r7]

	/*invalidate scu*/
	ldr	r7, =0xf8f0000c
	ldr	r6, =0xffff
	str	r6, [r7]

	/* Write to ACTLR */
	mrc	p15, 0, r0, c1, c0, 1		/* Read ACTLR*/
	orr	r0, r0, #(0x01 << 6)		/* set SMP bit */
	orr	r0, r0, #(0x01 )		/* */
	mcr	p15, 0, r0, c1, c0, 1		/* Write ACTLR*/

	/* Invalidate caches and TLBs */
	mov	r0,#0				/* r0 = 0  */
	mcr	p15, 0, r0, c8, c7, 0		/* invalidate TLBs */
	mcr	p15, 0, r0, c7, c5, 0		/* invalidate icache */
	mcr	p15, 0, r0, c7, c5, 6		/* Invalidate branch predictor array */

	MRC	p15,0,r0,c1,c0,2			// Enable NEON

.set PSS_L2CC_BASE_ADDR, 0xF8F02000

.set L2CCWay,		(PSS_L2CC_BASE_ADDR + 0x077C)	/*(PSS_L2CC_BASE_ADDR + PSS_L2CC_CACHE_INVLD_WAY_OFFSET)*/
.set L2CCSync,		(PSS_L2CC_BASE_ADDR + 0x0730)	/*(PSS_L2CC_BASE_ADDR + PSS_L2CC_CACHE_SYNC_OFFSET)*/
.set L2CCCrtl,		(PSS_L2CC_BASE_ADDR + 0x0100)	/*(PSS_L2CC_BASE_ADDR + PSS_L2CC_CNTRL_OFFSET)*/
.set L2CCAuxCrtl,	(PSS_L2CC_BASE_ADDR + 0x0104)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_AUX_CNTRL_OFFSET)*/
.set L2CCTAGLatReg,	(PSS_L2CC_BASE_ADDR + 0x0108)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_TAG_RAM_CNTRL_OFFSET)*/
.set L2CCDataLatReg,	(PSS_L2CC_BASE_ADDR + 0x010C)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_DATA_RAM_CNTRL_OFFSET)*/
.set L2CCIntClear,	(PSS_L2CC_BASE_ADDR + 0x0220)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_IAR_OFFSET)*/
.set L2CCIntRaw,	(PSS_L2CC_BASE_ADDR + 0x021C)	/*(PSS_L2CC_BASE_ADDR + XPSS_L2CC_ISR_OFFSET)*/
.set L2CCAuxControl,	0x72360000		/* Enable all prefetching, Cache replacement policy, Parity enable,
                                        Event monitor bus enable and Way Size (64 KB) */
.set L2CCControl,	0x01			/* Enable L2CC */
.set L2CCTAGLatency,	0x0111			/* latency for TAG RAM */
.set L2CCDataLatency,	0x0121			/* latency for DATA RAM */

	ldr	r0,=L2CCCrtl			/* Load L2CC base address base + control register */
	mov	r1, #0				/* force the disable bit */
	str	r1, [r0]			/* disable the L2 Caches */

	ldr	r0,=L2CCAuxCrtl			/* Load L2CC base address base + Aux control register */
	ldr	r1,[r0]				/* read the register */
	ldr	r2,=L2CCAuxControl		/* set the default bits */
	orr	r1,r1,r2
	str	r1, [r0]			/* store the Aux Control Register */

	ldr	r0,=L2CCTAGLatReg		/* Load L2CC base address base + TAG Latency address */
	ldr	r1,=L2CCTAGLatency		/* set the latencies for the TAG*/
	str	r1, [r0]			/* store the TAG Latency register Register */

	ldr	r0,=L2CCDataLatReg		/* Load L2CC base address base + Data Latency address */
	ldr	r1,=L2CCDataLatency		/* set the latencies for the Data*/
	str	r1, [r0]			/* store the Data Latency register Register */

	ldr	r0,=L2CCWay			/* Load L2CC base address base + way register*/
	ldr	r2, =0xFFFF
	str	r2, [r0]			/* force invalidate */

	ldr	r0,=L2CCSync			/* need to poll 0x730, PSS_L2CC_CACHE_SYNC_OFFSET */
						/* Load L2CC base address base + sync register*/
	/* poll for completion */
Sync:	ldr	r1, [r0]
	cmp	r1, #0
	bne	Sync

	ldr	r0,=L2CCIntRaw			/* clear pending interrupts */
	ldr	r1,[r0]
	ldr	r0,=L2CCIntClear
	str	r1,[r0]

	/*
		Setup the initial kernel boot process stack pointer.
		This should be the Physical Address!
	 */
	ldr	sp,=_stack

	/*
		Initialise the initial MMU table, and jump to kernel startup routine!
	*/

	/*
		Disable the MMU! - Just to make sure its disabled.
	*/
	mrc	p15, 0, r0, c1, c0, 0	// Read the MMU control register
	bic	r0, r0, #0x1			// Clear the enable bit
	mcr	p15, 0, r0, c1, c0, 0	// Write back to control register

	ldr 	r3, =0xBFF
	ldr		r2, =0x15de6
	ldr		r0, =((__bt_mmu_table_start - BT_CONFIG_RAM_VIRT) + BT_CONFIG_RAM_PHYS)
_mmu_init_loop:
	str		r2, [r0]
	add		r0, r0, #0x4
	add 	r2, r2, #0x100000
	subs	r3, r3, #1
	bge		_mmu_init_loop

	ldr		r3,=0x3FF
	ldr		r2,	=(0x15de6 + 0x00100000)
_mmu_init_loop2:
	str		r2,	[r0]
	add		r0, r0, #0x4
	add		r2, r2,	#0x100000
	subs	r3, r3, #1
	bge		_mmu_init_loop2

	mov		r0, #0x3
	mcr		p15, 0, r0, c3, c0, 0

	/*
		Set the MMU TLB level 1 table base address.
	*/
	ldr		r0, =((__bt_mmu_table_start - BT_CONFIG_RAM_VIRT) + BT_CONFIG_RAM_PHYS)
	orr		r0, r0, #0x5B				// Outer Cacheable, WB
	mcr 	p15, 0, r0, c2, c0, 0		// TTB0

	mvn		r0,#0
	mcr		p15,0,r0,c3,c0,0

	ldr		r0,=MMUCRVAL
	mcr		p15,0,r0,c1,c0,0			// Enable cache and MMU!

	dsb		// Allow MMU to start up
	isb		// Flush prefetch buffer.

	ldr		r0, =(BT_CONFIG_RAM_VIRT - BT_CONFIG_RAM_PHYS - 4)
	add		pc,	pc, r0

	ldr		r0, =__bt_init_start		// Set-up Initial kernel stack
	mcr		p15, 0, r0, c12, c0, 0

.set Abort_stack,	__abort_stack
.set SPV_stack,		__supervisor_stack
.set IRQ_stack,		__irq_stack
.set SYS_stack,		__stack

	mrs	r0, cpsr					/* get the current PSR */
	mvn	r1, #0x1f					/* set up the irq stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x12				/* IRQ mode */
	msr	cpsr, r2
	ldr	r13,=IRQ_stack				/* IRQ stack pointer */

	mrs	r0, cpsr					/* get the current PSR */
	mvn	r1, #0x1f					/* set up the supervisor stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x13				/* supervisor mode */
	msr	cpsr, r2
	ldr	r13,=SPV_stack				/* Supervisor stack pointer */

	mrs	r0, cpsr					/* get the current PSR */
	mvn	r1, #0x1f					/* set up the Abort  stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x17				/* Abort mode */
	msr	cpsr, r2
	ldr	r13,=Abort_stack			/* Abort stack pointer */

	mrs	r0, cpsr					/* get the current PSR */
	mvn	r1, #0x1f					/* set up the system stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x1F				/* SYS mode */
	msr	cpsr, r2
	ldr	r13,=SYS_stack				/* SYS stack pointer */

	bl 		_bt_startup_init_hook
	bl		_bt_startup_boot

wait:
	b wait

undefined:
	b 	undefined

prefetch:
	b 	prefetch

data:
	b 	data

fiq:
	b 	fiq

hang:
	b	hang

.globl _bt_startup_init_hook
.weak _bt_startup_init_hook
_bt_startup_init_hook:
	bx	lr

.globl _bt_startup_boot
.weak _bt_startup_boot
_bt_startup_boot:
	bx 	lr
