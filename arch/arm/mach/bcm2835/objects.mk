#
#	BCM2835 Platform objects
#
MACH_BCM2835_OBJECTS += $(BUILD_DIR)arch/arm/mach/bcm2835/bcm2835.o			# Provides machine description.
MACH_BCM2835_OBJECTS += $(BUILD_DIR)arch/arm/mach/bcm2835/irq_chip.o
MACH_BCM2835_OBJECTS += $(BUILD_DIR)arch/arm/mach/bcm2835/timer.o
MACH_BCM2835_OBJECTS += $(BUILD_DIR)arch/arm/mach/bcm2835/gpio.o
MACH_BCM2835_OBJECTS-$(BT_CONFIG_MACH_BCM2835_USE_STARTUP) += $(BUILD_DIR)arch/arm/mach/bcm2835/startup.o


MACH_BCM2835_OBJECTS += $(MACH_BCM2835_OBJECTS-y)

$(MACH_BCM2835_OBJECTS): MODULE_NAME="HAL"

OBJECTS += $(MACH_BCM2835_OBJECTS)
