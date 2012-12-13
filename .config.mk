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





include $(BASE)/os/objects.mk
include $(BASE)/lib/objects.mk
