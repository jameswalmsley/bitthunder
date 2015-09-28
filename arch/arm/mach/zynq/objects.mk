#
#	Zynq Platform objects
#
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)/arch/arm/mach/zynq/zynq.o			# Provides machine description.

MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_USE_STARTUP) += $(BUILD_DIR)/arch/arm/mach/zynq/startup.o

MACH_ZYNQ_OBJECTS += $(BUILD_DIR)/arch/arm/mach/zynq/headsmp.o
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)/arch/arm/mach/zynq/slcr.o
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)/arch/arm/mach/zynq/early_console.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_UART) += $(BUILD_DIR)/arch/arm/mach/zynq/uart.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_TIMER) += $(BUILD_DIR)/arch/arm/mach/zynq/timer.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_GPIO) += $(BUILD_DIR)/arch/arm/mach/zynq/gpio.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_SDIO) += $(BUILD_DIR)/arch/arm/mach/zynq/sdio.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_DEVCFG) += $(BUILD_DIR)/arch/arm/mach/zynq/devcfg.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_QSPI) += $(BUILD_DIR)/arch/arm/mach/zynq/qspi.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_I2C) += $(BUILD_DIR)/arch/arm/mach/zynq/i2c.o

MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_GEM) += $(BUILD_DIR)/arch/arm/mach/zynq/gem.o


$(BUILD_DIR)/arch/arm/mach/zynq/zynq.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_BASE=$(BT_CONFIG_ARCH_ARM_GIC_BASE)
$(BUILD_DIR)/arch/arm/mach/zynq/zynq.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_DIST_BASE=$(BT_CONFIG_ARCH_ARM_GIC_DIST_BASE)
$(BUILD_DIR)/arch/arm/mach/zynq/zynq.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS)
$(BUILD_DIR)/arch/arm/mach/zynq/early_console.o: CFLAGS += -fPIC

include $(BASE)/arch/arm/mach/zynq/boards/objects.mk

MACH_ZYNQ_OBJECTS += $(MACH_ZYNQ_OBJECTS-y)

$(MACH_ZYNQ_OBJECTS): MODULE_NAME="HAL"

OBJECTS += $(MACH_ZYNQ_OBJECTS)

#
#	ZYNQ Platform Specific targets
#
.PHONY:BOOT.BIN.info
BOOT.BIN.info:
	@echo "ZYNQ FSBL Boot Image generator"
	@echo "Generates a BOOT.BIN image with the BitThunder vmthunder.img file as the [BOOTLOADER] target."
	@echo "This allows the Zynq BOOT ROM to load BitThunder (Usually BootThunder) as the main bootloader."

.PHONY:BOOT.BIN
BOOT.BIN: $(PROJECT_DIR)/BOOT.BIN
$(PROJECT_DIR)/BOOT.BIN: $(PROJECT_DIR)/vmthunder.elf
	$(Q)$(PRETTY) BOOTGEN Zynq $(subst $(PROJECT_DIR)/,"",$@)
	$(Q)cp $(PROJECT_DIR)/vmthunder.elf $(BASE)/arch/arm/mach/zynq/tools/
	$(Q)echo "the_ROM_image:" 					>  $(BASE)/arch/arm/mach/zynq/tools/temp.bif
	$(Q)echo "{"								>> $(BASE)/arch/arm/mach/zynq/tools/temp.bif
	$(Q)echo "	[bootloader]vmthunder.elf"		>> $(BASE)/arch/arm/mach/zynq/tools/temp.bif
	$(Q)echo "	[init]BOOT.BIN.init"			>> $(BASE)/arch/arm/mach/zynq/tools/temp.bif
	$(Q)echo "}"								>> $(BASE)/arch/arm/mach/zynq/tools/temp.bif
	$(Q)touch $(PROJECT_DIR)/BOOT.BIN.init
	$(Q)cp $(PROJECT_DIR)/BOOT.BIN.init $(BASE)/arch/arm/mach/zynq/tools/
	$(Q)cd $(BASE)/arch/arm/mach/zynq/tools/ && TOOLCHAIN=$(BT_CONFIG_TOOLCHAIN) python ./bootgen.py temp.bif
	$(Q)cp $(BASE)/arch/arm/mach/zynq/tools/BOOT.BIN $(PROJECT_DIR)/

clean: zynq_clean
zynq_clean: | dbuild_splash
	$(Q)rm $(PRM_FLAGS) $(PROJECT_DIR)/BOOT.BIN $(PRM_PIPE)

all: $(PROJECT_DIR)/BOOT.BIN
