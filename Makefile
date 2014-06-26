#
#	BitThunder Top-Level Makefile
#

BASE:=$(dir $(CURDIR)/$(word $(words $(MAKEFILE_LIST)),$(MAKEFILE_LIST)))
BUILD_BASE:=$(BASE)
MODULE_NAME:="BitThunder"

ifeq ($(PROJECT_CONFIG), y)
BUILD_DIR:=$(PROJECT_DIR)/build/
else
BUILD_DIR:=$(shell pwd)/build/
PROJECT_DIR:=./
endif

TARGETS:=$(PROJECT_DIR)/vmthunder.img
TARGET_DEPS:=$(PROJECT_DIR)/vmthunder.elf


CONFIG_:=BT_CONFIG_
CONFIG_HEADER_NAME:=bt_bsp_config.h

ifneq ($(PROJECT_CONFIG), y)
CONFIG_HEADER_PATH:=$(BASE)lib/include/
#CONFIG_PATH:=$(shell pwd)
else
CONFIG_HEADER_PATH:=$(PROJECT_DIR)/include
CONFIG_PATH:=$(PROJECT_DIR)
endif

include $(BASE).dbuild/dbuild.mk

ifeq ($(PROJECT_CONFIG), y)
$(PROJECT_DIR)/.config:
	$(Q)echo " >>>> No .config file found, run make menuconfig"
	echo $@
else
$(PROJECT_DIR).config:
	$(Q)echo " >>>> No .config file found, run make menuconfig"
endif

all: $(PROJECT_DIR)/vmthunder.elf $(PROJECT_DIR)/vmthunder.list $(PROJECT_DIR)/vmthunder.img $(PROJECT_DIR)/vmthunder.syms
	$(Q)$(SIZE) $(PROJECT_DIR)/vmthunder.elf

test:
	@echo $(BASE)

$(PROJECT_DIR)/vmthunder.img: $(PROJECT_DIR)/vmthunder.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) $(PROJECT_DIR)/vmthunder.elf -O binary $@

$(PROJECT_DIR)/vmthunder.elf: $(OBJECTS) $(LINKER_SCRIPTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(CC) -march=$(CC_MARCH) -mtune=$(CC_MTUNE) $(CC_TCFLAGS) $(CC_MACHFLAGS) $(CC_MFPU) $(CC_FPU_ABI) -o $@ -T $(LINKER_SCRIPT) -Wl,-Map=$(PROJECT_DIR)/vmthunder.map -Wl,--gc-sections $(OBJECTS) -nostdlib $(LDLIBS) -lc -lm -lgcc

$(PROJECT_DIR)/vmthunder.list: $(PROJECT_DIR)/vmthunder.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D -S $(PROJECT_DIR)/vmthunder.elf > $@

$(PROJECT_DIR)/vmthunder.syms: $(PROJECT_DIR)/vmthunder.elf
	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -t $(PROJECT_DIR)/vmthunder.elf > $@

ifeq ($(PROJECT_CONFIG), y)
$(OBJECTS) $(OBJECTS-y): $(PROJECT_DIR)/.config
else
$(OBJECTS) $(OBJECTS-y): $(PROJECT_DIR).config
endif

project.init:
	$(Q)touch $(PROJECT_DIR)/Kconfig
	-$(Q)mkdir $(PROJECT_DIR)/include
	$(Q)echo "PROJECT_CONFIG=y" > $(PROJECT_DIR)/Makefile
	$(Q)echo "PROJECT_DIR=$(PROJECT_DIR)" >> $(PROJECT_DIR)/Makefile
	$(Q)echo "include $(shell $(RELPATH) $(BASE) $(PROJECT_DIR))/Makefile" >> $(PROJECT_DIR)/Makefile

mrproper:
	$(Q)rm -rf $(BASE).config $(BASE)lib/include/bt_bsp_config.h
