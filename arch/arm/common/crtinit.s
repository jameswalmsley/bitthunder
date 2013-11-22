.globl _bt_start

	.text
.Lsbss_start:
	.long	__sbss_start

.Lsbss_end:
	.long	__sbss_end

.Lbss_start:
	.long	__bss_start

.Lbss_end:
	.long	__bss_end



	.globl	_start
_start:
_bt_start:
	bl      __cpu_init			/* Initialize the CPU first (BSP provides this) */

	mov	r0, #0

	/* clear sbss */
	ldr 	r1,.Lsbss_start		/* calculate beginning of the SBSS */
	ldr	r2,.Lsbss_end			/* calculate end of the SBSS */

.Lloop_sbss:
	cmp	r1,r2
	bge	.Lenclsbss				/* If no SBSS, no clearing required */
	str	r0, [r1], #4
	b	.Lloop_sbss

.Lenclsbss:
	/* clear bss */
	ldr	r1,.Lbss_start		/* calculate beginning of the BSS */
	ldr	r2,.Lbss_end		/* calculate end of the BSS */

.Lloop_bss:
	cmp	r1,r2
	bge	.Lenclbss		/* If no BSS, no clearing required */
	str	r0, [r1], #4
	b	.Lloop_bss

.Lenclbss:

	/* set stack pointer */
	ldr	r13,=_stack

#ifdef PROFILING			/* defined in Makefile */
	/* Setup profiling stuff */
	#bl	_profile_init
#endif /* PROFILING */


	/* make sure argc and argv are valid */

	mov	r0, r7		// First argument is the machine id.
	mov	r1,	r8		// Second argument is the fdt address.

	/* Let her rip */
	bl	bt_main

#ifdef PROFILING
	/* Cleanup profiling stuff */
	#bl	_profile_clean
#endif /* PROFILING */

        /* All done */
	#bl	exit

.Lexit:	/* should never get here */
	b .Lexit

.Lstart:
	.size	_start,.Lstart-_start
