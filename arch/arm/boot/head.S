/**
 *	BitThunder Kernel Entry for ARM architectures.
 */

#include <btlinker_config.h>
#include <asm/assembly.h>

.section .bt.init					// The HEAD of the kernel is placed by the .bt.init section.
.globl bt_reset

.extern __bt_mmu_table_start
.extern bt_do_bug

	/*
	 *	This defines the ARM vector table, it is repeated twice, so that it may be used on initial
	 *	boot-up from address 0x0, but also a trampoline can be injected within the space of
     * 	the first copy of the table for starting other processors.
	 */
_bt_vector_table:
	ldr pc, =(BT_LINKER_KERNEL_ENTRY + 0x80)	// Reset vector
	ldr	pc, [pc, #24]							// undefined instruction vector
	ldr pc, [pc, #24]							// SVC handler
	ldr pc, [pc, #24]							// Prefetch abort handler
	ldr pc, [pc, #24]							// Data abort handler
	ldr pc, [pc, #24]							// Historical Handler - Was invalid address from the old 26-bit ARM CPUs
	ldr pc, [pc, #24]							// IRQ handler
	ldr pc, [pc, #24]							// FIQ handler

	.word 	bt_reset
	.word	undefined
	.word	_bt_syscall_entry
	.word	prefetch
	.word	data
	.word	hang
#ifndef BT_CONFIG_KERNEL_NONE
	.word 	vFreeRTOS_IRQInterrupt
#else
	.word	BT_ARCH_ARM_GIC_IRQHandler
#endif
	.word	fiq

	.align	5									// Guarantee that 2nd table falls on a 32byte boundary.

_bt_vector_second_table:
	ldr pc, =(BT_LINKER_KERNEL_ENTRY + 0x80)	// Reset vector
	ldr	pc, [pc, #24]							// undefined instruction vector
	ldr pc, [pc, #24]							// SVC handler
	ldr pc, [pc, #24]							// Prefetch abort handler
	ldr pc, [pc, #24]							// Data abort handler
	ldr pc, [pc, #24]							// Historical Handler - Was invalid address from the old 26-bit ARM CPUs
	ldr pc, [pc, #24]							// IRQ handler
	ldr pc, [pc, #24]							// FIQ handler

	.word 	bt_reset
	.word	undefined
	.word	_bt_syscall_entry
	.word	prefetch
	.word	data
	.word	hang
#ifndef BT_CONFIG_KERNEL_NONE
	.word 	vFreeRTOS_IRQInterrupt
#else
	.word	BT_ARCH_ARM_GIC_IRQHandler
#endif
	.word	fiq
_bt_vector_table_end:

bt_reset:
	/*
	 * Disable the IRQs as soon as possible, just in-case the boot-loader was naughty!
	 */
	mrs	r9,cpsr
	orr	r9,r9,#0x80
	msr	cpsr_c,r9

	mov r7,	r1	// Save the architecture ID.	// According to ARM abi, r7 and r8 must be preserved by a function if used.
	mov r8, r2	// Save the fdt pointer.

	ldr		r0, =_bt_vector_second_table
	mcr		p15, 0, r0, c12, c0, 0				// Set the VBAR (Vector base address register).

	/*
	 *	Setup the initial kernel boot process stack pointer.
	 *	This should be the Physical Address!
	 */
	bl	_bt_startup_init_hook
	ldr sp,=_stack

#ifdef BT_CONFIG_ARCH_ARM_FULL_INIT

	/*
	 *	Enable the snoop control unit.
	 */
	stmdb sp!, {r7}					// Preserve r7

	ldr	r7, =0xf8f00000
	ldr	r0, [r7]
	orr	r0, r0, #0x1
	str	r0, [r7]

	/*
	 *	Invalidate the snoop control unit.
	 */
	ldr	r7, =0xf8f0000c
	ldr	r6, =0xffff
	str	r6, [r7]

	ldmfd sp!, {r7}					// Restore R7

	mrc	p15, 0, r0, c1, c0, 0		// Read SCTLR
	bic	r0, r0, #(0x1 << 25)		// Ensure EE bit is cleared, reset to LITTLE_ENDIAN data.
	mcr	p15, 0, r0, c1, c0, 0		// Write SCTLR

	/* Write to ACTLR */
	mrc	p15, 0, r0, c1, c0, 1		// Read ACTLR
	orr	r0, r0, #(0x01 << 6)		// set SMP bit
	orr	r0, r0, #(0x01 )
	mcr	p15, 0, r0, c1, c0, 1		// Write ACTLR


	/*
	 *	Disable the MMU! - Just to make sure its disabled.
	 */
	mrc	p15, 0, r0, c1, c0, 0	// Read the MMU control register
	bic	r0, r0, #0x1			// Clear the enable bit
	mcr	p15, 0, r0, c1, c0, 0	// Write back to control register

	/*
	 *	Invalidate all caches, and the TLB, ensure MMU is disabled!
	 */
	mov	r0,#0
	mcr	p15, 0, r0, c8, c7, 0		// invalidate TLBs
	mcr	p15, 0, r0, c7, c5, 0		// invalidate icache
	mcr	p15, 0, r0, c7, c5, 6		// Invalidate branch predictor array
	bl	invalidate_dcache			/* invalidate dcache */

.set PSS_L2CC_BASE_ADDR,	 0xF8F02000
.set L2CCWay,				(PSS_L2CC_BASE_ADDR + 0x077C)
.set L2CCSync,				(PSS_L2CC_BASE_ADDR + 0x0730)
.set L2CCCrtl,				(PSS_L2CC_BASE_ADDR + 0x0100)
.set L2CCAuxCrtl,			(PSS_L2CC_BASE_ADDR + 0x0104)
.set L2CCTAGLatReg,			(PSS_L2CC_BASE_ADDR + 0x0108)
.set L2CCDataLatReg,		(PSS_L2CC_BASE_ADDR + 0x010C)
.set L2CCIntClear,			(PSS_L2CC_BASE_ADDR + 0x0220)
.set L2CCIntRaw,			(PSS_L2CC_BASE_ADDR + 0x021C)
.set L2CCAuxControl,		0x72360000						/* Enable all prefetching, Cache replacement policy, Parity enable,
    														 *	Event monitor bus enable and Way Size (64 KB) */

.set L2CCControl,			0x01							// Enable L2CC
.set L2CCTAGLatency,		0x0111							// latency for TAG RAM
.set L2CCDataLatency,		0x0121							// latency for DATA RAM

	ldr	r0,=L2CCCrtl		// Load L2CC base address base + control register
	mov	r1, #0				// force the disable bit
	str	r1, [r0]			// disable the L2 Caches

	ldr	r0,=L2CCAuxCrtl		// Load L2CC base address base + Aux control register
	ldr	r1,[r0]				// read the register
	ldr	r2,=L2CCAuxControl	// set the default bits
	orr	r1,r1,r2
	str	r1, [r0]			// store the Aux Control Register

	ldr	r0,=L2CCTAGLatReg	// Load L2CC base address base + TAG Latency address
	ldr	r1,=L2CCTAGLatency	// set the latencies for the TAG
	str	r1, [r0]			// store the TAG Latency register Register

	ldr	r0,=L2CCDataLatReg	// Load L2CC base address base + Data Latency address
	ldr	r1,=L2CCDataLatency	// set the latencies for the Data
	str	r1, [r0]			// store the Data Latency register Register

	ldr	r0,=L2CCWay			// Load L2CC base address base + way register
	ldr	r2, =0xFFFF
	str	r2, [r0]			// force invalidate

	ldr	r0,=L2CCSync		// need to poll 0x730, PSS_L2CC_CACHE_SYNC_OFFSET
							// Load L2CC base address base + sync register
							// poll for completion
Sync:	ldr	r1, [r0]
	cmp	r1, #0
	bne	Sync

	ldr	r0,=L2CCIntRaw		// clear pending interrupts
	ldr	r1,[r0]
	ldr	r0,=L2CCIntClear
	str	r1,[r0]

#endif


	/*
	 *	Initialise the initial MMU table, and jump to kernel startup routine!
	 */

	/*
	 *	Disable the MMU! - Just to make sure its disabled.
	 */
	mrc	p15, 0, r0, c1, c0, 0	// Read the MMU control register
	bic	r0, r0, #0x1			// Clear the enable bit
	mcr	p15, 0, r0, c1, c0, 0	// Write back to control register

#define CACHED_MEM		0x00015de6
#define UNCACHED_MEM 	0x00000c02

.set MMUCRVAL,	0b01000000000101	/* Enable IDC, and MMU */

#ifndef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	/*
	 *	In the case we are using normal memory, we need a reasonable identity mapping.
	 *	We shall loop over the entire address space, and if the current address falls within
	 *	any of the defined memory regions, then we shall enable caching.
	 *
	 *	In all other cases, it should be accessible BUT NOT cached. i.e. BARE-METAL!
	 */
	ldr		r3, =0xFFF	// 0 upto 0xFFFF_FFFF in MB sections (<= loop)
	mov		r2, #0x00000000		// Start at address 0
	ldr		r0, =__bt_mmu_table_start
__mmu_init_loop:
	ldr		r5,	=0x000FFFFF
	bic		r2, r5
	ldr		r4,	=UNCACHED_MEM	// Flags to OR with SECTION address
#ifdef	BT_CONFIG_LINKER_SECTION_FLASH					// Check if in a FLASH section.
	ldr		r5,	=BT_CONFIG_LINKER_FLASH_START_ADDRESS
	cmp		r2, r5
	blt		skip_flash
	ldrge	r5, =(BT_CONFIG_LINKER_FLASH_START_ADDRESS + BT_CONFIG_LINKER_FLASH_LENGTH)
	cmpge	r2, r5
	ldrle	r4, =CACHED_MEM
skip_flash:
#endif
#ifdef	BT_CONFIG_LINKER_SECTION_SRAM					// Check if in a SRAM section.
	ldr		r5,	=BT_CONFIG_LINKER_SRAM_START_ADDRESS
	cmp		r2, r5
	blt		skip_sram
	ldrge	r5, =(BT_CONFIG_LINKER_SRAM_START_ADDRESS + BT_CONFIG_LINKER_SRAM_LENGTH)
	cmpge	r2, r5
	ldrle	r4, =CACHED_MEM
skip_sram:
#endif
#ifdef	BT_CONFIG_LINKER_SECTION_RAM					// Check if in a RAM section.
	ldr		r5,	=BT_CONFIG_LINKER_RAM_START_ADDRESS
	cmp		r2, r5
	blt		skip_ram
	ldrge	r5, =(BT_CONFIG_LINKER_RAM_START_ADDRESS + BT_CONFIG_LINKER_RAM_LENGTH)
	cmpge	r2, r5
	ldrle	r4, =CACHED_MEM
skip_ram:
#endif

	orr		r2,	r4
	str		r2,	[r0]
	add		r0,	r0, #0x4
	add		r2,	r2, #0x00100000
	subs	r3, r3, #1
	bge		__mmu_init_loop

	mov		r0, #0x3
	mcr		p15, 0, r0, c3, c0, 0


	/*
		Set the MMU TLB level 1 table base address.
	*/
	ldr		r0, =__bt_mmu_table_start
	orr		r0, r0, #0x5B				// Outer Cacheable, WB
	mcr 	p15, 0, r0, c2, c0, 0		// TTB0

	mvn		r0,#0
	mcr		p15,0,r0,c3,c0,0

	ldr		r0,=MMUCRVAL
	mcr		p15,0,r0,c1,c0,0			// Enable cache and MMU!
	mcr		p15, 0, r0, c8, c7, 0	// Invalidate the TLBs

	data_sync
	instr_sync

#endif

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	ldr 	r3, =0xBFF
	ldr		r2, =0x15de6
	ldr		r0, =((__bt_mmu_table_start - BT_CONFIG_RAM_VIRT) + BT_CONFIG_RAM_PHYS)
_mmu_init_loop:
	str		r2, [r0]
	add		r0, r0, #0x4
	add 	r2, r2, #0x100000
	subs	r3, r3, #1
	bge		_mmu_init_loop

//	ldr		r3,=0xE
	ldr		r3,=((BT_CONFIG_LINKER_RAM_LENGTH/0x00100000)-1)
	ldr		r2,	=(0x15de6 + BT_CONFIG_RAM_PHYS)
_mmu_init_loop2:
	str		r2,	[r0]
	add		r0, r0, #0x4
	add		r2, r2,	#0x100000
	subs	r3, r3, #1
	bge		_mmu_init_loop2

	ldr		r3,=0x3F0
	mov		r2,	#0
_mmu_init_loop3:
	str		r2,	[r0]
	add		r0, r0, #0x4
	subs	r3, r3, #1
	bge		_mmu_init_loop3

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
	mcr		p15, 0, r0, c8, c7, 0	// Invalidate the TLBs

	data_sync
	instr_sync

	// Jump to the virtual address space

	ldr		pc, jump_target
	jump_target:	.word	jump_to_virtual
jump_to_virtual:
#endif

#ifdef BT_CONFIG_ARCH_ARM_FULL_INIT
	ldr	r0,=L2CCCrtl
	ldr	r1,[r0]
	mov	r2, #L2CCControl
	orr	r1,r1,r2
	str	r1,[r0]

	mov	r0, r0
	mrc	p15, 0, r1, c1, c0, 2		/* read cp access control register (CACR) into r1 */
	orr	r1, r1, #(0xf << 20)		/* enable full access for p10 & p11 */
	mcr	p15, 0, r1, c1, c0, 2		/* write back into CACR */
#endif

#ifdef BT_CONFIG_ARCH_ARM_HAS_NEON
	mrc	p15,0,r0,c1,c0,2			// Enable NEON
	orr r0, r0, #3<<10
	bic	r0, r0, #3<<14
	mcr	p15, 0, r0, c1, c1, 2

	ldr r0, =(0xF << 20)
	mcr p15, 0, r0, c1, c0, 2

	.set FPEXC_EN,		0x40000000		/* FPU enable bit, (1 << 30) */
	/* enable vfp */
	fmrx	r1, FPEXC			/* read the exception register */
	orr	r1,r1, #FPEXC_EN		/* set VFP enable bit, leave the others in orig state */
	fmxr	FPEXC, r1			/* write back the exception register */

	mrc	p15,0,r0,c1,c0,0		/* flow prediction enable */
	orr	r0, r0, #(0x01 << 11)		/* #0x8000 */
	mcr	p15,0,r0,c1,c0,0

	mrc	p15,0,r0,c1,c0,1		/* read Auxiliary Control Register */
	orr	r0, r0, #(0x1 << 2)		/* enable Dside prefetch */
	orr	r0, r0, #(0x1 << 1)		/* enable L2 Prefetch hint */
	mcr	p15,0,r0,c1,c0,1		/* write Auxiliary Control Register */
#endif

.set Abort_stack,	__abort_stack
.set SPV_stack,		__supervisor_stack
.set IRQ_stack,		__irq_stack
.set SYS_stack,		__stack

	mrs	r0, cpsr					/* get the current PSR */
	mvn	r1, #0x1f					/* set up the irq stack pointer */
	and	r2, r1, r0
	orr	r2, r2, #0x12				/* IRQ mode */
	msr	cpsr, r2
	msr	spsr_cfx, r2
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

	/*
	 *	Now we are running in virtual space, lets cause any access currently below 0xC000_0000
	 *	to cause a seg-fault.
	 */
#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	ldr 	r3, =0xBFF
	ldr		r2, =0
	ldr		r0, =((__bt_mmu_table_start - BT_CONFIG_RAM_VIRT) + BT_CONFIG_RAM_PHYS)
_mmu_loop:
	str		r2, [r0]
	add		r0, r0, #0x4
	subs	r3, r3, #1
	bge		_mmu_loop

	mcr		p15, 0, r0, c8, c7, 0	// Invalidate the TLBs
	data_sync
#endif

	/*
	 *	Now we are running in a "safe" environment.
	 */
	bl		_bt_startup_boot
	b		_bt_start

wait:
	b wait

undefined:
	b 	undefined

prefetch:
	mrc p15, 0, r0, c5, c0, 0	// Fault status code
	mrc p15, 0, r1, c6, c0, 0	// Fault address
prefetch_loop:
	b 	prefetch_loop

data:
	mov r0, lr
	sub r0, r0, #8
	bl bt_do_bug
data_loop:
	b 	data_loop

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
	ldr	r0,=0xDEADBEEF
	b 	_bt_start

/**
 *	BitThunder System Call handling -- assuming modified FreeRTOS kernel.
 *
 * 	BitThunder patches freertos to include a seperate kernel space stack. This
 * 	stack is the 2nd item in the TCB.
 *
 *	A task only enter an swi once, and must not recurse from Kernel mode.
 **/
_bt_syscall_entry:
	/*
	 *	Handle FreeRTOS Yielding
	 */
#ifndef BT_CONFIG_KERNEL_NONE
	stmdb	sp!, {r4,r5,r6}			// Lets not clobber r4-5.
#ifndef BT_CONFIG_ARCH_ARM_THUMB
	ldr		r6, [lr,#-4]			// Instruction behind LR is SWI ($0) instruction.
	bic		r6, #0xFF000000			// Mask out the opcode, to leave the syscall vector.
	cmp		r6, #0x000000FF			// Special yield SWI to break into FreeRTOS!
#else
	ldr 	r6,	[lr,#-2]
	bic 	r6,	#0xFFFFFF00
	cmp 	r6, #0xFF
#endif
	ldmeqfd	sp!, {r4,r5,r6}			// Restore context for FreeRTOS yield handler.
	beq		vPortYieldProcessor

#ifdef BT_CONFIG_SYSCALL
	/*
	 *	Get Kernel stack pointer.
	 */
	ldr		r4,=pxCurrentTCB
	ldr		r4,[r4]					// Kernel stack pointer is 2nd item in struct!
	add		r4, #4
	ldr		r4, [r4]

	/*
	 *	Save the user-mode stack and program state (PSR) onto kernel mode stack.
	 */
	stmdb	r4,{lr}^				// Push the USER mode LR to stack.
	sub 	r4, r4, #4
	stmdb	r4!,{lr}				// Store the return address
	mrs		r5,	spsr				// Copy the user mode program state reg to r1 for pushing!
	stmdb	r4, {r5,sp}^ 			// Push User stack pointer onto kernel stack! and cpsr!
	nop
	sub		r4, r4, #8

	// Push Syscall number onto the kernel mode stack
	stmdb	r4!, {r6}				// Push SWI onto kernel stack! I.e. ARG 5 of function call.

	/*
	 *	Update the user-SP (used also in SYS mode) to the kernel stack pointer.
	 */
	stmdb	sp!, {r4}				// Push stack addr to SVC stack.
	ldmfd	sp,{sp}^				// Remove it from SVC stack into usermode SP.
	add		sp, sp, #4				// Move stack pointer back.

	mov		r6, sp					// Use this to restore register from stack once in sys mode.
	add		sp, sp, #12				// SVC stack pointer back to origin.

	// Switch to SYS mode, set SP to be new SP!
	msr		cpsr_c, #0x9F			// Switch to SYS mode

	// Restore regs 4/5/6
	ldmfd	r6,	{r4, r5, r6}

	stmdb	sp!, {r4}

	// Now we can re-enable interrupts! :)
	mrs		r4, cpsr
	bic		r4, r4,	#0xC0	// Enable Interrupts
	msr		cpsr_c, r4

	ldmfd	sp!, {r4}

	bl		bt_syscall_handler
	// r0 should now contain the result, so we can now return to the user stack and return

_bt_syscall_return:
	/*
	 *	Disable Interrupts.
	 */
	msr		cpsr_c,	#0x9F			// Disable interrupts, remaining in SYS mode.
	msr		cpsr_c,	#0x93			// Switch back to SVC mode.

	stmdb	sp!, {r4,r5,r6}
	stmdb	sp,	{sp}^				// Push Kernel mode stack pointer onto SVC stack
	nop
	sub		sp, sp, #4
	ldmfd	sp!, {r4}				// Place kernel stack into r4.

	ldmfd	r4!, {r5}				// Pop off the SWI number
	ldmfd	r4!, {r5}				// Pop the saved PSR.
	ldmfd	r4!, {r6}				// Pop the USR stack pointer.
	ldmfd	r4!, {lr}				// Restore the link register.

	msr		spsr, r5				// Restore the SPSR

	stmdb	sp!, {r6}				// Push USER stack pointer to be restored.
	ldmfd	sp, {sp}^				// Restore the user stack pointer onto user SP reg.
	nop
	add		sp, sp, #4

	ldmfd	r4, {lr}^				// POP the user mode LR from stack.

	ldmfd	sp!, {r4, r5, r6}
#endif
#endif
	// Return for SVC handler
	subs	pc, lr, #0

/*
 *************************************************************************
 *
 * invalidate_dcache - invalidate the entire d-cache by set/way
 *
 * Note: for Cortex-A9, there is no cp instruction for invalidating
 * the whole D-cache. Need to invalidate each line.
 *
 *************************************************************************
 */
invalidate_dcache:
	mrc	p15, 1, r0, c0, c0, 1		/* read CLIDR */
	ands	r3, r0, #0x7000000
	mov	r3, r3, lsr #23			/* cache level value (naturally aligned) */
	beq	finished
	mov	r10, #0				/* start with level 0 */
loop1:
	add	r2, r10, r10, lsr #1		/* work out 3xcachelevel */
	mov	r1, r0, lsr r2			/* bottom 3 bits are the Cache type for this level */
	and	r1, r1, #7			/* get those 3 bits alone */
	cmp	r1, #2
	blt	skip				/* no cache or only instruction cache at this level */
	mcr	p15, 2, r10, c0, c0, 0		/* write the Cache Size selection register */
	instr_sync
	mrc	p15, 1, r1, c0, c0, 0		/* reads current Cache Size ID register */
	and	r2, r1, #7			/* extract the line length field */
	add	r2, r2, #4			/* add 4 for the line length offset (log2 16 bytes) */
	ldr	r4, =0x3ff
	ands	r4, r4, r1, lsr #3		/* r4 is the max number on the way size (right aligned) */
	clz	r5, r4				/* r5 is the bit position of the way size increment */
	ldr	r7, =0x7fff
	ands	r7, r7, r1, lsr #13		/* r7 is the max number of the index size (right aligned) */
loop2:
	mov	r9, r4				/* r9 working copy of the max way size (right aligned) */
loop3:
	orr	r11, r10, r9, lsl r5		/* factor in the way number and cache number into r11 */
	orr	r11, r11, r7, lsl r2		/* factor in the index number */
	mcr	p15, 0, r11, c7, c14, 2		/* clean & invalidate by set/way */
	subs	r9, r9, #1			/* decrement the way number */
	bge	loop3
	subs	r7, r7, #1			/* decrement the index */
	bge	loop2
skip:
	add	r10, r10, #2			/* increment the cache number */
	cmp	r3, r10
	bgt	loop1

finished:
	mov	r10, #0				/* swith back to cache level 0 */
	mcr	p15, 2, r10, c0, c0, 0		/* select current cache level in cssr */
	data_sync
	instr_sync
	bx	lr
