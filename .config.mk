#
#	Automated build configuration switches.
#
ifeq ($(BT_CONFIG_OS),y)
BT_CONFIG_LIB=n
else
BT_CONFIG_LIB=y
endif


include $(BASE)/os/objects.mk
include $(BASE)/lib/objects.mk
include $(BASE)/kernel/objects.mk
