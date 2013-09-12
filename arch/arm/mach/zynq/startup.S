#include <bt_config.h>

.globl _bt_startup_init_hook

_bt_startup_init_hook:

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_INIT_GT
	ldr	r0,=1
	ldr	r1,=0xf8f00208		@ Start the global timer.
	str	r0,[r1]
#endif


#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_PS7INIT

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_DDR_RELOCATE
	ldr sp,=0x30000
#endif

	stmdb sp!, {lr}
	bl	ps7_init			@ Run the clock configurator, as exported from Xilinx HW generator.
	bl 	ps7_post_config
	ldmfd sp!, {lr}

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_DDR_RELOCATE
	ldr sp,=_stack
#endif

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
