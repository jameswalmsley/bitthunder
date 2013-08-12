#
#	Automated build configuration switches.
#

include $(BASE).config

ARCH:=$(shell echo $(BT_CONFIG_ARCH))
SUBARCH:=$(shell echo $(BT_CONFIG_SUBARCH))
TOOLCHAIN:=$(shell echo $(BT_CONFIG_TOOLCHAIN))

include $(BASE)os/objects.mk
include $(BASE)lib/objects.mk
include $(BASE)kernel/objects.mk

BSP_DIR:=$(shell echo $(BT_CONFIG_BSP_DIR))

test_dir:
	echo $(BT_CONFIG_BSP_NAME)
	echo $(BT_CONFIG_BSP_DIR)


include $(BASE)$(BSP_DIR)/objects.mk

$(OBJECTS): CFLAGS += -Wall -I $(BASE)/lib/include/ -I $(BASE)/arch/arm/include/
$(OBJECTS): CFLAGS += $(shell echo $(BT_CONFIG_TOOLCHAIN_DEBUG_FLAGS))
$(OBJECTS): CFLAGS += -march=$(shell echo $(BT_CONFIG_ARCH_ARM_FAMILY))
$(OBJECTS): CFLAGS += -mcpu=$(shell echo $(BT_CONFIG_TOOLCHAIN_CPU)) $(shell echo $(BT_CONFIG_TOOLCHAIN_FLAGS)) $(shell echo $(BT_CONFIG_TOOLCHAIN_OPTIMISATION))
$(OBJECTS): CFLAGS += $(shell echo $(BT_CONFIG_TOOLCHAIN_MACH_FLAGS))

$(OBJECTS): CFLAGS += -nostdlib -fno-builtin -fdata-sections -ffunction-sections
$(OBJECTS): CFLAGS += -I $(BASE)/${BSP_DIR}/

$(LINKER_SCRIPTS): CFLAGS += -I $(BASE)/${BSP_DIR}/
