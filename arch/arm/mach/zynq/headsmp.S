/**
 *	Trampoline code for Zynq platform.
 *
 *	This trampoline code allows a jump address, and upto 4 parameters to be specified.
 *	To use the code must copied to the cpu boot address, usually 0x0, and the addresses
 * 	and values replaced.
 *
 *	Obviously this piece of code must remain position independent.
 *
 *	You can see an example of its use in the zynq.c zynq_boot_core() function.
 *
 * 	@author	James Walmsley	<james@fullfat-fs.co.uk>
 **/

.section .bt.init.vectors.trampoline
.globl zynq_trampoline
zynq_trampoline:
	ldr	r4,	zynq_trampoline_jump		// 	Load the jump address into r4
	ldr	r0, zynq_trampoline_r0			// 	Load the r0 value.
	ldr	r1, zynq_trampoline_r1			// 	Load the r1 value.
	ldr	r2, zynq_trampoline_r2			// 	Load the r2 value.
	ldr	r3, zynq_trampoline_r3			// 	Load the r3 value.
	mov	r5, #-1
	str	r5,	zynq_trampoline_jump		// 	Signal that jump has occured.
	bx	r4								// 	Branch to the jump address (now in r4).
.globl zynq_trampoline_jump
zynq_trampoline_jump:		.word 	0	// 	The jump address!	(If 0 is used this will cause an infinite loop until the address is written).
zynq_trampoline_r0:			.word 	0	//	The r0 value.
zynq_trampoline_r1:			.word	0	//	The r1 value.
zynq_trampoline_r2:			.word	0	//	The r2 value.
zynq_trampoline_r3:			.word	0	//	The r3 value.
.globl zynq_trampoline_end
zynq_trampoline_end:
