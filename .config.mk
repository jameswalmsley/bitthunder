#
#	Automated build configuration switches.
#

include $(BASE).config

CONFIG_:=BT_CONFIG_

ARCH:=$(shell echo $(BT_CONFIG_ARCH))
SUBARCH:=$(shell echo $(BT_CONFIG_SUBARCH))
TOOLCHAIN:=$(shell echo $(BT_CONFIG_TOOLCHAIN))

include $(BASE)kernel/objects.mk
include $(BASE)os/objects.mk
include $(BASE)lib/objects.mk

PYTHON:=$(BT_CONFIG_DBUILD_PYTHON)

BSP_DIR:=$(shell echo $(BT_CONFIG_BSP_DIR))

GIT_DESCRIBE:=$(shell git --git-dir=$(BASE).git describe)

test_dir:
	echo $(BT_CONFIG_BSP_NAME)
	echo $(BT_CONFIG_BSP_DIR)

test_git:
	echo $(GIT_DESCRIBE)

include $(BASE)$(BSP_DIR)/objects.mk

CC_MARCH 		:= $(shell echo $(BT_CONFIG_ARCH_ARM_FAMILY))
CC_MTUNE 		:= $(shell echo $(BT_CONFIG_TOOLCHAIN_CPU))
CC_TCFLAGS 		:= $(shell echo $(BT_CONFIG_TOOLCHAIN_FLAGS))
CC_TCDEBUGFLAGS := $(shell echo $(BT_CONFIG_TOOLCHAIN_DEBUG_FLAGS))
CC_OPTIMISE 	:= $(shell echo $(BT_CONFIG_TOOLCHAIN_OPTIMISATION))
CC_MACHFLAGS 	:= $(shell echo $(BT_CONFIG_TOOLCHAIN_MACH_FLAGS))
CC_MFPU			:= $(shell echo $(BT_CONFIG_TOOLCHAIN_MFPU))
CC_FPU_ABI		:= $(shell echo $(BT_CONFIG_TOOLCHAIN_FPU_ABI))

$(OBJECTS) $(OBJECTS-y): CFLAGS += -Wall -I $(BASE)/lib/include/ -I $(BASE)/arch/arm/include/
$(OBJECTS) $(OBJECTS-y): CFLAGS += $(CC_TCDEBUGFLAGS)
$(OBJECTS) $(OBJECTS-y): CFLAGS += -march=$(CC_MARCH)
$(OBJECTS) $(OBJECTS-y): CFLAGS += -mtune=$(CC_MTUNE) $(CC_TCFLAGS) $(CC_OPTIMISE) $(CC_MFPU) $(CC_FPU_ABI)
$(OBJECTS) $(OBJECTS-y): CFLAGS += $(CC_MACHFLAGS)
$(OBJECTS) $(OBJECTS-y): CFLAGS += -D BT_VERSION_SUFFIX="\"$(GIT_DESCRIBE)\""

$(OBJECTS) $(OBJECTS-y): CFLAGS += -nostdlib -fno-builtin -fdata-sections -ffunction-sections
$(OBJECTS) $(OBJECTS-y): CFLAGS += -I $(BASE)${BSP_DIR}/
$(OBJECTS) $(OBJECTS-y): CFLAGS += -I $(BASE)include/
$(OBJECTS) $(OBJECTS-y): CFLAGS += -I $(BASE)os/src/net/lwip/src/include/ -I $(BASE)os/include/net/lwip/ -I $(BASE)os/src/net/lwip/src/include/ipv4/

$(LINKER_SCRIPTS): CFLAGS += -I $(BASE)/${BSP_DIR}/
