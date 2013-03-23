#
#	Automated build configuration switches.
#

include $(BASE).config

ARCH:=$(shell echo $(BT_CONFIG_ARCH))
SUBARCH:=$(shell echo $(BT_CONFIG_SUBARCH))

include $(BASE)os/objects.mk
include $(BASE)lib/objects.mk
include $(BASE)kernel/objects.mk

BSP_DIR:=$(shell echo $(BT_CONFIG_BSP_DIR))

test_dir:
	echo $(BT_CONFIG_BSP_NAME)
	echo $(BT_CONFIG_BSP_DIR)


include $(BASE)$(BSP_DIR)/objects.mk

$(OBJECTS): CFLAGS += -nostdlib -fno-builtin -fdata-sections -ffunction-sections
$(OBJECTS): CFLAGS += -I $(BASE)/${BSP_DIR}/
