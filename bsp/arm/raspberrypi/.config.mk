ARCH=arm
SUBARCH=raspberrypi

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

#
#	Automated build configuration switches.
#
ifeq ($(BT_CONFIG_OS),y)
BT_CONFIG_LIB=n
BT_CONFIG_KERNEL=y
SUB_OBJDIRS += $(BASE)kernel/
else
BT_CONFIG_LIB=y
endif
