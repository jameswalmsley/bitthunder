MACH_IMX_OBJECTS += $(BUILD_DIR)/arch/arm/mach/$(SUBARCH)/imx6.o			# Provides machine description.

MACH_IMX_OBJECTS += $(MACH_IMX_OBJECTS-y)

$(MACH_IMX_OBJECTS): MODULE_NAME="IMX"


OBJECTS += $(MACH_IMX_OBJECTS)
