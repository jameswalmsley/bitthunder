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
#TOOLCHAIN=/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/bin/arm-xilinx-eabi-

include $(BASE).dbuild/dbuild.mk

test:
	@echo $(PATH)

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




kernel.elf: LDFLAGS += -L"c:/yagarto-20121222/lib/gcc/arm-none-eabi/4.7.2/thumb/v6m/"
kernel.elf: LDFLAGS += -L"c:/yagarto-20121222/arm-none-eabi/lib//thumb/v6m/"
#kernel.elf: LDFLAGS += -L"/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/lib/gcc/arm-xilinx-eabi/4.6.1/"
#kernel.elf: LDFLAGS += -L"/opt/Xilinx/14.2/ISE_DS/EDK/gnu/arm/lin64/arm-xilinx-eabi/lib/"
kernel.elf: LDLIBS += -lc -lgcc
kernel.elf: $(OBJECTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(LD) $(LDFLAGS) --gc-sections -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(OBJECTS) $(LDLIBS)

