#
#	Zynq Platform objects
#
OBJECTS += $(BUILD_DIR)arch/arm/mach/zynq/zynq.o
OBJECTS-$(BT_CONFIG_MACH_ZYNQ_USE_STARTUP) += $(BUILD_DIR)arch/arm/mach/zynq/startup.o
