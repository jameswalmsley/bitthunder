#include <bt_config.h>

.globl _bt_startup_init_hook

_bt_startup_init_hook:

#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_INIT_GT
	ldr	r0,=1
	ldr	r1,=0xf8f00208		@ Start the global timer.
	str	r0,[r1]
#endif


#ifdef BT_CONFIG_MACH_ZYNQ_STARTUP_PS7INIT
	stmdb sp!, {lr}
	bl	ps7_init			@ Run the clock configurator, as exported from Xilinx HW generator.
	bl 	ps7_post_config
	ldmfd sp!, {lr}
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
