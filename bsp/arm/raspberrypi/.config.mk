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
SUBARCH=bcm2835

#
#	Arch Specific configuration.
#

#
#	This enables the default BitThunder startup code,
#	You can replace this with your own code by disabling this option.
#

include $(BASE).config.mk
