#
#	Top-Level Makefile for BitThunder on the Zynq/Zedboard platform.
#

BASE=$(shell pwd)/../../../../
BUILD_BASE=$(BASE)
MODULE_NAME="lpc-bsp"

TARGETS=kernel.img
LINKER_SCRIPT=lpc.ld

TARGET_DEPS=kernel.elf


BUILD_DIR=$(shell pwd)/build/

TOOLCHAIN=C:/yagarto-20121222/bin/arm-none-eabi-

include $(BASE).dbuild/dbuild.mk

test:
	@echo $(SUBARCH)
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

kernel.elf: LDFLAGS += -L"c:/yagarto-20121222/lib/gcc/arm-none-eabi/4.7.2/"
kernel.elf: LDFLAGS += -L"c:/yagarto-20121222/arm-none-eabi/lib/"
kernel.elf: LDLIBS += -lc -lgcc
kernel.elf: $(OBJECTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(LD) $(LDFLAGS) -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(OBJECTS) $(LDLIBS)

