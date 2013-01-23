#SUB_OBJDIRS += $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/

BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_GIC) 	+= $(BUILD_DIR)arch/arm/common/gic.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-A9)	+= $(BUILD_DIR)arch/arm/common/cortex-a9-cpu-timers.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-A9)	+= $(BUILD_DIR)arch/arm/common/freertos-arm.o

BT_ARCH_ARM_OBJECTS += $(BT_ARCH_ARM_OBJECTS-y)

$(BUILD_DIR)arch/arm/common/gic.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS)

$(BUILD_DIR)arch/arm/common/freertos-arm.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)arch/arm/common/freertos-arm.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/

$(BUILD_DIR)os/src/bt_main.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)os/src/bt_main.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/

$(BT_ARCH_ARM_OBJECTS): MODULE_NAME="HAL"
OBJECTS += $(BT_ARCH_ARM_OBJECTS)
