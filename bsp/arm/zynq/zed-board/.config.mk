#
#	Build the BitThunder as an operating system, or a software framework?
#
#	y = os... 		use BitThunder scheduling/process and memory management.
#	n = framework	Memory / process management is done by your underlying OS. Uses libc.
#
BT_CONFIG_OS=y

#
#	Kernel / Scheduler selection.
#
BT_CONFIG_KERNEL=FreeRTOS
#BT_CONFIG_KERNEL=BtKernel


ARCH=arm
SUBARCH=zynq

#
#	Arch Specific configuration.
#

#
#	This enables the default BitThunder startup code,
#	You can replace this with your own code by disabling this option.
#
BT_CONFIG_MACH_ZYNQ_USE_STARTUP=y
BT_CONFIG_MACH_ZYNQ_SYSCLOCK_FREQ=33333333
BT_CONFIG_MACH_ZYNQ_SYSTICK_TIMER_ID=0
BT_CONFIG_MACH_ZYNQ_BOOT_UART_ID=1
BT_CONFIG_MAX_INTERRUPT_CONTROLLERS=2


include $(BASE).config.mk
