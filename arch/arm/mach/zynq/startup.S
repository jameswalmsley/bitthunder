#include <bt_config.h>

.globl _bt_startup_init_hook
_bt_startup_init_hook:

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_INIT_GT
	ldr	r0,=1
	ldr	r1,=0xf8f00208		@ Start the global timer.
	str	r0,[r1]
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_DDR_RELOCATE
	ldr sp,=0x30000
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_PS7INIT
	stmdb sp!, {lr}
	bl	ps7_init			@ Run the clock configurator, as exported from Xilinx HW generator.
	bl 	ps7_post_config
	ldmfd sp!, {lr}
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_RAM_TEST
	stmdb sp!, {lr}
	bl 	_bt_zynq_ram_test
	ldmfd sp!, {lr}
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_DDR_RELOCATE
	ldr sp,=_stack
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_DDR_RELOCATE
	/*
	 *	Copy the entire OCM (on-chip-memory [SRAM]) into DDR for Relocation.
	 *
	 *	Note. This assumes we're straight out of the BootROM.
	 *	  	i.e. 	0x00000000 - 0x2FFFF == OCM,
	 *				0x00100000 + == DDR.
	 *
	 * 	This DDR location depends on the SCU (snoop-control-unit) filtering, and
	 *	therefore the lower DDR region is simply masked-out.
	 *
	 * 	In-place relocation happens in the following stages:
	 *
	 *	a. Copy entire OCM to DDR base (0x0010_0000) 1MB.
	 *	b. Do a position-independent jump, to next instruction but in the DDR region.
	 *	c. Modify the SCU to remove the filtering, so DDR appears at 0x0.
	 *	d. Copy image, (now in DDR) down to 0x0.
	 * 	e. Do another PI jump back to linked-space, and we're now in DDR.
	 *
	 *	Awesome :)
	 */
	ldr		r3, =(0xC000-1)		// 192Kb / 4
	mov 	r0, #0			// OCM base (0x00).
_relocate_loop:
	ldr 	r2, [r0]				// load 4 bytes into r2
	mov 	r1, r0
	add		r1, r1, #0x00100000		// Get relative DDR location.
	str		r2, [r1]
	add		r0,	r0, #0x4			// Next word
	subs	r3, r3, #1
	bge 	_relocate_loop


	// Now we can JUMP to 0x0010_0000 relative address.
	ldr		r0, =_relocate_stage_a_target
	ldr 	r1, =0x00100000
	add		r0, r0, r1
	mov		pc, r0
_relocate_stage_a_target:

	// Remap!
	// unlock SLCR registers.
	ldr		r0, =0xf8000008
	ldr		r1, =0x0000df0d
	str		r1, [r0]				// Unlock the SLCR registers

	ldr		r0, =0xf8f00040			// Pointer to the FILTER_START reg in SLCR SCU configure regs
	mov		r1, #0
	str		r1, [r0]

	// hopefully we are now here!
	ldr		r3, =(0xC000-1)
	mov		r0, #0x00100000			// TEXT_BASE in DDR!
_relocate_loop_2:
	ldr		r2, [r0]
	mov		r1, r0
	sub		r1, r1, #0x00100000
	str		r2, [r1]
	add		r0, r0, #0x4
	subs	r3, r3, #1
	bge		_relocate_loop_2

	ldr		pc, =_relocate_stage_b_target	// Jump back into linked-space!
_relocate_stage_b_target:
	mov		r0, r0
	mov		r0, r0
#endif

	bx	lr

/*
 *	Provide a default ps7_init() function that does nothing.
 */
#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_PS7INIT
.globl ps7_init
.weak ps7_init
ps7_init:
	bx	lr

.globl ps7_post_config
.weak ps7_post_config
ps7_post_config:
	bx	lr
#endif

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_RAM_TEST

_bt_zynq_ram_test:
	/*
	 *	This is a really early RAM-test. We will temporarily replace the ARM vector table
	 *  so that we can catch any exceptions from the RAM test... The vector table will then
	 *  print a message with the error and the address of the fault repeatedly.
	 *
	 */

	stmdb sp!, {lr}
	bl 	bt_early_printk_init
	ldmfd sp!, {lr}

	mrc	p15, 0, r0, c12, c0, 0 					// Place the current vector table address into r0 and push to be restored.
	stmdb sp!, {r0}

	ldr	r0, =_bt_ram_test_vector_table
	mcr p15, 0, r0, c12, c0, 0 					// Change VBAR to point to our exception catching table.

	mov 	r0, #0x0 							// i = 0;
	mov 	r1, #BT_CONFIG_MACH_ZYNQ_STARTUP_RAM_TEST_LENGTH 					// i < 0x00100000
	ldr 	r3, =BT_CONFIG_MACH_ZYNQ_STARTUP_RAM_TEST_BASE
	mov 	r4, r3

	// load the memory up with an address dependent pattern.

test_loop:
	mvn 	r2, r0 								// val = ~addr
	add 	r0, r0, #1
	str 	r2, [r3]
	add 	r3, r3, #4
	cmp 	r0, r1
	bne 	test_loop

	// Verify the memory pattern is correct.
	mov 	r0, #0x0
	mov 	r1, #BT_CONFIG_MACH_ZYNQ_STARTUP_RAM_TEST_LENGTH
	mov 	r3, #BT_CONFIG_MACH_ZYNQ_STARTUP_RAM_TEST_BASE
	mov 	r4, r3
verify_loop:
	mvn 	r2, r0
	add 	r0, r0, #1
	ldr 	r5, [r3]
	cmp 	r5, r2
	bne 	verify_fail
	add 	r3, r3, #4
	cmp 	r0, r1
	beq 	verify_loop
verify_done:
	ldmfd 	sp!, {r0}								// Restore VBAR back to the original vector table.
	mcr		p15, 0, r0, c12, c0, 0 					// Place the current vector table address into r0 and push to be restored.

	bx		lr

verify_fail:
	ldr r0, =_my_early_data_mismatch_reason
	mov r1, r3
	stmdb sp!, {r3}
	bl 		bt_zynq_ram_test_error					// _bt_zynq_ram_test_error(&_my_early_string, addr (r3));
	ldmfd sp!, {r3}
	mov r0, #0
	ldr r1, =0x9FFFFF								// Delay loop and keep printing.
verify_fail_loop:
	add r0, r0, #1
	cmp r0, r1
	bne verify_fail_loop
	b	verify_fail

.align 5
_bt_ram_test_vector_table:
	ldr pc, [pc, #24]							// Reset vector.
	ldr	pc, [pc, #24]							// undefined instruction vector
	ldr pc, [pc, #24]							// SVC handler
	ldr pc, [pc, #24]							// Prefetch abort handler
	ldr pc, [pc, #24]							// Data abort handler
	ldr pc, [pc, #24]							// Historical Handler - Was invalid address from the old 26-bit ARM CPUs
	ldr pc, [pc, #24]							// IRQ handler
	ldr pc, [pc, #24]							// FIQ handler

	.word 	early_reset
	.word	early_undefined
	.word	early_syscall
	.word	early_prefetch
	.word	early_data
	.word	early_hang
	.word	early_irq
	.word	early_fiq

early_data:
	// Create a string with the address.
	ldr r0, =_my_early_data_abort_reason
	mov r1, r3
	stmdb sp!, {r3}
	bl 		bt_zynq_ram_test_error					// _bt_zynq_ram_test_error(&_my_early_string, addr (r3));
	ldmfd sp!, {r3}
	mov r0, #0
	ldr r1, =0x9FFFFF								// Delay loop and keep printing.
loop:
	add r0, r0, #1
	cmp r0, r1
	bne loop
	b	early_data

early_reset:
early_undefined:
early_syscall:
early_prefetch:
early_hang:
early_irq:
early_fiq:

_my_early_data_abort_reason:
	.asciz "Error: Data Abort accessing address: 0x"
_my_early_data_mismatch_reason:
	.asciz "Error: Data mis-match error at address: 0x"

#endif
