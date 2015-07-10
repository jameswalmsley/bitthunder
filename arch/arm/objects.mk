CFLAGS += -I $(BASE)/arch/arm/include/

#
#	Interrupt Controller Implementations
#

# GIC
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_GIC) 	+= $(BUILD_DIR)/arch/arm/common/gic.o
$(BUILD_DIR)/arch/arm/common/gic.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_GIC_TOTAL_IRQS)

## NVIC
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_NVIC)	+= $(BUILD_DIR)/arch/arm/common/nvic.o

## Set default NVIC base unless overriden in BSP config
BT_CONFIG_ARCH_ARM_NVIC_BASE ?= 0xE000E100

## Configure the NVIC CFLAGS
$(BUILD_DIR)/arch/arm/common/nvic.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS)
$(BUILD_DIR)/arch/arm/common/nvic_vectors.o: CFLAGS += -DBT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS=$(BT_CONFIG_ARCH_ARM_NVIC_TOTAL_IRQS)

## Include the NVIC default vector table with weak symbols
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_USE_NVIC)	+= $(BUILD_DIR)/arch/arm/common/nvic_vectors.o

#
#	Timer Devices
#
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_A9)	+= $(BUILD_DIR)/arch/arm/common/cortex-a9-cpu-timers.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_A9)	+= $(BUILD_DIR)/arch/arm/common/gt.o
#$(BUILD_DIR)/arch/arm/common/gt.o: CFLAGS_REMOVE += $(CC_MFPU) $(CC_FPU_ABI)
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_M0)	+= $(BUILD_DIR)/arch/arm/common/cortex/systick.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_M3)	+= $(BUILD_DIR)/arch/arm/common/cortex/systick.o

#
#	Cache and CPU interfaces.
#
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_A9)	+= $(BUILD_DIR)/arch/arm/common/arm11cpu.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_A9)	+= $(BUILD_DIR)/arch/arm/common/arm-cache.o



#
#	Kernel Scheduler Ports
#
ifeq ($(BT_CONFIG_KERNEL_FREERTOS), y)
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_ARM11)		+= $(BUILD_DIR)/arch/arm/common/freertos-arm11.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_ARM11)		+= $(BUILD_DIR)/arch/arm/common/freertos-arm11-asm.o
$(BUILD_DIR)/arch/arm/common/freertos-arm11-portisr.o: CFLAGS_REMOVE += -fstack-usage
ifeq ($(BT_CONFIG_KERNEL_FREERTOS_CA9_MODERN_PORT), y)
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_A9)	+= $(BUILD_DIR)/arch/arm/common/freertos-ca9-asm.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_A9)	+= $(BUILD_DIR)/arch/arm/common/freertos-ca9.o
else
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_A9)	+= $(BUILD_DIR)/arch/arm/common/freertos-arm.o
$(BUILD_DIR)/arch/arm/common/freertos-arm.o: CFLAGS_REMOVE += -fstack-usage
endif
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_M0) += $(BUILD_DIR)/arch/arm/common/freertos-m0.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_CORTEX_M3) += $(BUILD_DIR)/arch/arm/common/freertos-m3.o
$(BUILD_DIR)/arch/arm/common/freertos-m0.o: CFLAGS_REMOVE += -fstack-usage
$(BUILD_DIR)/arch/arm/common/freertos-m3.o: CFLAGS_REMOVE += -fstack-usage
endif

#
#	MMU
#
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_HAS_MMU)			+= $(BUILD_DIR)/arch/arm/mm/v7-mmu.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_HAS_MMU)			+= $(BUILD_DIR)/arch/arm/mm/v7-mmu-asm.o

$(BUILD_DIR)/arch/arm/mm/v7-mmu.o: CFLAGS_REMOVE += -mthumb

#
#	Boot-Up
#
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_BOOT)		+= $(BUILD_DIR)/arch/arm/boot/head.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_BOOT)		+= $(BUILD_DIR)/arch/arm/common/crtinit.o
BT_ARCH_ARM_OBJECTS-$(BT_CONFIG_ARCH_ARM_BOOT)		+= $(BUILD_DIR)/arch/arm/common/cpuinit.o

BT_ARCH_ARM_OBJECTS += $(BT_ARCH_ARM_OBJECTS-y)


$(BUILD_DIR)/arch/arm/common/freertos-arm11.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/arch/arm/common/freertos-arm11.o: CFLAGS += -I $(BASE)/arch/arm/include/arch/common/
$(BUILD_DIR)/arch/arm/common/freertos-arm11-portisr.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/arch/arm/common/freertos-arm11-portisr.o: CFLAGS += -I $(BASE)/arch/arm/include/arch/common/

$(BUILD_DIR)/arch/arm/common/freertos-arm.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/arch/arm/common/freertos-arm.o: CFLAGS += -I $(BASE)/arch/arm/include/arch/common/
$(BUILD_DIR)/arch/arm/common/freertos-arm.o: CFLAGS_REMOVE += -mthumb
$(BUILD_DIR)/arch/arm/common/freertos-ca9.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/arch/arm/common/freertos-ca9.o: CFLAGS += -I $(BASE)/arch/arm/include/arch/common/
$(BUILD_DIR)/arch/arm/common/freertos-ca9.o: CFLAGS_REMOVE += -mthumb
$(BUILD_DIR)/arch/arm/common/freertos-m0.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/arch/arm/common/freertos-m0.o: CFLAGS += -I $(BASE)/arch/arm/include/arch/common/
$(BUILD_DIR)/arch/arm/common/freertos-m3.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/arch/arm/common/freertos-m3.o: CFLAGS += -I $(BASE)/arch/arm/include/arch/common/



$(BUILD_DIR)/os/src/bt_main.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/os/src/bt_main.o: CFLAGS += -I $(BASE)/arch/arm/include/arch/common/

$(BT_ARCH_ARM_OBJECTS): MODULE_NAME="HAL"
OBJECTS += $(BT_ARCH_ARM_OBJECTS)

$(BUILD_DIR)/arch/arm/bitthunder.lds: MODULE_NAME="Linker"
LINKER_SCRIPTS += $(BUILD_DIR)/arch/arm/bitthunder.lds
