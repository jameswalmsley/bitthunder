#SUB_OBJDIRS += $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/


#
#	Interrupt Controller Implementations
#

# GIC
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_GIC) 	+= $(BUILD_DIR)arch/arm/common/gic.o
$(BUILD_DIR)arch/arm/common/gic.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS)

## NVIC
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_NVIC)	+= $(BUILD_DIR)arch/arm/common/nvic.o

## Set default NVIC base unless overriden in BSP config
BT_CONFIG_ARCH_ARM_NVIC_BASE ?= 0xE000E100

## Configure the NVIC CFLAGS
$(BUILD_DIR)arch/arm/common/nvic.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS)
$(BUILD_DIR)arch/arm/common/nvic_vectors.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS)

## Include the NVIC default vector table with weak symbols
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_NVIC)	+= $(BUILD_DIR)arch/arm/common/nvic_vectors.o

#
#	Timer Devices
#
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-A9)	+= $(BUILD_DIR)arch/arm/common/cortex-a9-cpu-timers.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-M0)	+= $(BUILD_DIR)arch/arm/common/cortex/systick.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-M3)	+= $(BUILD_DIR)arch/arm/common/cortex/systick.o



#
#	Kernel Scheduler Ports
#
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_ARM11)		+= $(BUILD_DIR)arch/arm/common/freertos-arm11.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_ARM11)		+= $(BUILD_DIR)arch/arm/common/freertos-arm11-portisr.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-A9)	+= $(BUILD_DIR)arch/arm/common/freertos-arm.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-M0) += $(BUILD_DIR)arch/arm/common/freertos-m0.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX-M3) += $(BUILD_DIR)arch/arm/common/freertos-m3.o

BT_ARCH_ARM_OBJECTS += $(BT_ARCH_ARM_OBJECTS-y)


$(BUILD_DIR)arch/arm/common/freertos-arm11.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)arch/arm/common/freertos-arm11.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/
$(BUILD_DIR)arch/arm/common/freertos-arm11-portisr.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)arch/arm/common/freertos-arm11-portisr.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/

$(BUILD_DIR)arch/arm/common/freertos-arm.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)arch/arm/common/freertos-arm.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/
$(BUILD_DIR)arch/arm/common/freertos-arm.o: CFLAGS += -D $(BT_CONFIG_FREERTOS_PORT_ARCH)
$(BUILD_DIR)arch/arm/common/freertos-m0.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)arch/arm/common/freertos-m0.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/
$(BUILD_DIR)arch/arm/common/freertos-m0.o: CFLAGS += -D $(BT_CONFIG_FREERTOS_PORT_ARCH)
$(BUILD_DIR)arch/arm/common/freertos-m3.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)arch/arm/common/freertos-m3.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/
$(BUILD_DIR)arch/arm/common/freertos-m3.o: CFLAGS += -D $(BT_CONFIG_FREERTOS_PORT_ARCH)



$(BUILD_DIR)os/src/bt_main.o: CFLAGS += -I $(BASE)kernel/FreeRTOS/Source/include/
$(BUILD_DIR)os/src/bt_main.o: CFLAGS += -I $(BASE)arch/arm/include/arch/common/

$(BT_ARCH_ARM_OBJECTS): MODULE_NAME="HAL"
OBJECTS += $(BT_ARCH_ARM_OBJECTS)
