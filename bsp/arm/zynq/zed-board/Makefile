#
#	Top-Level Makefile for BitThunder on the Zynq/Zedboard platform.
#

BASE=$(shell pwd)/../../../../
BUILD_BASE=$(BASE)
MODULE_NAME="zed-bsp"

TARGETS=kernel.img
# kernel.list kernel.syms kernel.map
LINKER_SCRIPT=zed.ld

TARGET_DEPS=kernel.elf


BUILD_DIR=$(shell pwd)/build/

TOOLCHAIN=/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/bin/arm-xilinx-eabi-

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

kernel.elf: LDFLAGS += -L"/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/lib/gcc/arm-xilinx-eabi/4.6.1/"
kernel.elf: LDFLAGS += -L"/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/arm-xilinx-eabi/lib/"
kernel.elf: LDLIBS += -lc -lgcc
kernel.elf: $(OBJECTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(LD) $(LDFLAGS) -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(OBJECTS) libxil.a $(LDLIBS) libxil.a
