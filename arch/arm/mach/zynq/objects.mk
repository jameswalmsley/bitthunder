#
#	Zynq Platform objects
#
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/zynq.o			# Provides machine description.

MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_USE_STARTUP) += $(BUILD_DIR)arch/arm/mach/zynq/startup.o

MACH_ZYNQ_OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/headsmp.o
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/slcr.o
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/uart.o
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/timer.o
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/gpio.o
MACH_ZYNQ_OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/sdio.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_DEVCFG) += $(BUILD_DIR)arch/arm/mach/zynq/devcfg.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_QSPI) += $(BUILD_DIR)arch/arm/mach/zynq/qspi.o
MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_I2C) += $(BUILD_DIR)arch/arm/mach/zynq/i2c.o

MACH_ZYNQ_OBJECTS-$(BT_CONFIG_MACH_ZYNQ_GEM) += $(BUILD_DIR)arch/arm/mach/zynq/gem.o


$(BUILD_DIR)arch/arm/mach/zynq/zynq.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_BASE=$(BT_CONFIG_ARCH_ARM_GIC_BASE)
$(BUILD_DIR)arch/arm/mach/zynq/zynq.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_DIST_BASE=$(BT_CONFIG_ARCH_ARM_GIC_DIST_BASE)
$(BUILD_DIR)arch/arm/mach/zynq/zynq.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS)

MACH_ZYNQ_OBJECTS += $(MACH_ZYNQ_OBJECTS-y)

$(MACH_ZYNQ_OBJECTS): MODULE_NAME="HAL"


OBJECTS += $(MACH_ZYNQ_OBJECTS)
