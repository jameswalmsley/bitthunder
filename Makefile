#
#	BitThunder Top-Level Makefile
#

BASE:=$(shell pwd)/
BUILD_BASE:=$(BASE)
MODULE_NAME:="BitThunder"

TARGETS:=vmthunder.img
TARGET_DEPS:=vmthunder.elf

BUILD_DIR:=$(shell pwd)/build/

all: .config
CONFIG_:=BT_CONFIG_
CONFIG_HEADER_NAME:="bt_bsp_config.h"
CONFIG_HEADER_PATH:=$(BASE)lib/include/
include $(BASE).dbuild/dbuild.mk

all: vmthunder.elf vmthunder.list vmthunder.img vmthunder.syms
	$(Q)$(SIZE) vmthunder.elf

.config:
	$(Q)$(MAKE) menuconfig

vmthunder.img: vmthunder.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) vmthunder.elf -O binary $@

vmthunder.elf: $(OBJECTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(CC) -march=$(CC_MARCH) -mtune=$(CC_MTUNE) $(CC_TCFLAGS) $(CC_MACHFLAGS) $(CC_MFPU) $(CC_FPU_ABI) -o $@ -T $(LINKER_SCRIPT) -Wl,-Map=kernel.map -Wl,--gc-sections $(OBJECTS) -nostdlib $(LDLIBS) -lc -lm -lgcc

vmthunder.list: vmthunder.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D -S vmthunder.elf > $@

vmthunder.syms: vmthunder.elf
	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -t vmthunder.elf > $@
