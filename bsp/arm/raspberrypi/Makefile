#
#	Makefile for BitThunder on the Raspberry Pi
#

BASE=$(shell pwd)/../../../
BUILD_BASE=$(BASE)

MODULE_NAME="rpi-bsp"

TARGETS=kernel.img
TARGET_DEPS=kernel.elf

BUILD_DIR=$(shell pwd)/build/

include $(BASE).dbuild/dbuild.mk


all: kernel.list kernel.img kernel.syms
	$(Q)$(SIZE) kernel.elf

kernel.img: kernel.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

kernel.list: kernel.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D -S  kernel.elf > $@

kernel.syms: kernel.elf
	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -t kernel.elf > $@

kernel.elf: $(OBJECTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(CC) -o $@ -T $(LINKER_SCRIPT) -Wl,-Map=kernel.map -Wl,--gc-sections $(OBJECTS) -nostdlib -lc -lgcc
