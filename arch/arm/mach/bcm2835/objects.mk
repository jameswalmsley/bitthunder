#
#	BCM2835 Platform objects
#
MACH_BCM2835_OBJECTS += $(BUILD_DIR)arch/arm/mach/bcm2835/bcm2835.o			# Provides machine description.

MACH_BCM2835_OBJECTS += $(MACH_BCM2835_OBJECTS-y)

$(MACH_BCM2835_OBJECTS): MODULE_NAME="HAL"

OBJECTS += $(MACH_BCM2835_OBJECTS)
