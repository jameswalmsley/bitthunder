#
#	Top-Level Makefile for BitThunder on the Zynq/Zedboard platform.
#

BASE=$(shell pwd)/../../../../
BUILD_BASE=$(BASE)
MODULE_NAME="zed-bsp"

TARGETS=kernel.img
#kernel.list kernel.syms kernel.map
LINKER_SCRIPT=zed.ld

TARGET_DEPS=kernel.elf


BUILD_DIR=$(shell pwd)/build/

TOOLCHAIN=/opt/codesourcery/bin/arm-none-eabi-

include $(BASE).dbuild/dbuild.mk

test:
	@echo $(SUB_OBJDIRS)

all: kernel.elf kernel.list kernel.img kernel.syms
	$(Q)$(SIZE) kernel.elf

kernel.img: kernel.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

kernel.list: kernel.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D kernel.elf > $@

kernel.syms: kernel.elf
#	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
#	$(Q)$(OBJDUMP) -t kernel.elf > $@

kernel.elf: LDFLAGS += -L"/opt/codesourcery/lib/gcc/arm-none-eabi/4.7.2/"
kernel.elf: LDFLAGS += -L"/opt/codesourcery/arm-none-eabi/lib/"
kernel.elf: LDLIBS += -lc -lgcc
kernel.elf: $(OBJECTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(LD) --gc-sections $(LDFLAGS) -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(OBJECTS) libxil.a $(LDLIBS) libxil.a
