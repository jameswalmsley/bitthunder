#
#	Top-Level Makefile for BitThunder on the stm32-discovery platform
#

BASE=$(shell pwd)/../../../../
BUILD_BASE=$(BASE)
MODULE_NAME="stm32-bsp"

TARGETS=kernel.img
LINKER_SCRIPT=stm32.ld

TARGET_DEPS=kernel.elf


BUILD_DIR=$(shell pwd)/build/

TOOLCHAIN=arm-none-eabi-

include $(BASE).dbuild/dbuild.mk

all: kernel.hex

test:
	@echo $(SUBARCH)
	@echo $(SUB_OBJDIRS)

all: kernel.elf kernel.list kernel.img kernel.syms
	$(Q)$(SIZE) kernel.elf

kernel.hex: kernel.elf
	$(Q)$(TOOLCHAIN)objcopy -O ihex $< $@

kernel.img: kernel.elf
	$(Q)$(PRETTY) IMAGE $(MODULE_NAME) $@
	$(Q)$(OBJCOPY) kernel.elf -O binary $@

kernel.list: kernel.elf
	$(Q)$(PRETTY) LIST $(MODULE_NAME) $@
	$(Q)$(OBJDUMP) -D kernel.elf > $@

kernel.syms: kernel.elf
#	$(Q)$(PRETTY) SYMS $(MODULE_NAME) $@
#	$(Q)$(OBJDUMP) -t kernel.elf > $@


kernel.elf: LDFLAGS += -L"c:/yagarto/lib/gcc/arm-none-eabi/4.7.2/thumb/v6m/"
kernel.elf: LDFLAGS += -L"c:/yagarto/arm-none-eabi/lib/thumb/v6m/"
kernel.elf: LDLIBS += -lc -lgcc

#LDFLAGS = $(MCFLAGS) -mthumb -nostartfiles -Wl,-Map=kernel.map,--cref,--no-warn-mismatch $(LIBDIR)

kernel.elf: $(OBJECTS)
	$(Q)$(PRETTY) --dbuild "LD" $(MODULE_NAME) $@
	$(Q)$(LD) $(LDFLAGS) -Map kernel.map -o $@ -T $(LINKER_SCRIPT) $(OBJECTS) $(LDLIBS)

