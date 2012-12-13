#SUB_OBJDIRS += $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/

OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_GIC) += $(BUILD_DIR)arch/arm/common/gic.o
