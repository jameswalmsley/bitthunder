#
#	Automated build configuration switches.
#

ifneq ($(PROJECT_CONFIG), y)
-include $(BASE)/.config
else
-include $(PROJECT_DIR)/.config
endif

ARCH		:=$(subst $(DB_QUOTES),,$(BT_CONFIG_ARCH))
SUBARCH		:=$(subst $(DB_QUOTES),,$(BT_CONFIG_SUBARCH))
TOOLCHAIN	:=$(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN))

include $(BASE)/kernel/objects.mk
include $(BASE)/os/objects.mk
include $(BASE)/lib/objects.mk

PYTHON:=$(BT_CONFIG_DBUILD_PYTHON)

GIT_DESCRIBE:=$(shell git --git-dir=$(BASE)/.git describe --dirty)

CC_MARCH 		:= $(subst $(DB_QUOTES),,$(BT_CONFIG_ARCH_ARM_FAMILY))
CC_MTUNE		:= $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_CPU))
CC_TCFLAGS		:= $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_FLAGS))
CC_TCDEBUGFLAGS := $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_DEBUG_FLAGS))
CC_OPTIMISE		:= $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_OPTIMISATION))
CC_WARNING		:= $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_WARNING))
CC_MACHFLAGS 	:= $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_MACH_FLAGS))
CC_MFPU			:= $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_MFPU))
CC_FPU_ABI		:= $(subst $(DB_QUOTES),,$(BT_CONFIG_TOOLCHAIN_FPU_ABI))

$(OBJECTS) $(OBJECTS-y): CFLAGS += -fstack-usage $(CC_WARNING) -I $(BASE)/lib/include/ -I $(BASE)/arch/arm/include/
$(OBJECTS) $(OBJECTS-y): CFLAGS += $(CC_TCDEBUGFLAGS)
$(OBJECTS) $(OBJECTS-y): CFLAGS += -march=$(CC_MARCH)
$(OBJECTS) $(OBJECTS-y): CFLAGS += -mtune=$(CC_MTUNE) $(CC_TCFLAGS) $(CC_OPTIMISE) $(CC_MFPU) $(CC_FPU_ABI)
$(OBJECTS) $(OBJECTS-y): CFLAGS += $(CC_MACHFLAGS)
$(OBJECTS) $(OBJECTS-y): CFLAGS += -D BT_VERSION_SUFFIX="\"$(GIT_DESCRIBE)\""

$(OBJECTS) $(OBJECTS-y): CFLAGS += -nostdlib -fno-builtin -fdata-sections -ffunction-sections -fPIC
$(OBJECTS) $(OBJECTS-y): CFLAGS += -I $(BASE)/include/
ifeq ($(PROJECT_CONFIG),y)
$(OBJECTS) $(OBJECTS-y): CFLAGS += -I $(PROJECT_DIR)/include/
endif

$(OBJECTS) $(OBJECTS-y): CFLAGS += -I $(BASE)/os/src/net/lwip/src/include/ -I $(BASE)/os/include/net/lwip/ -I $(BASE)/os/src/net/lwip/src/include/ipv4/

ifeq ($(PROJECT_CONFIG),y)
$(LINKER_SCRIPTS): CFLAGS += -I $(PROJECT_DIR)/include/
endif

#
#	Link configuration.
#
ifeq ($(BT_CONFIG_BUILD_NOSTDLIB), y)
LDFLAGS += -nostdlib
endif

ifeq ($(BT_CONFIG_BUILD_GC_UNUSED), y)
LDFLAGS += -Wl,--gc-sections
endif
