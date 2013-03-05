#
#	BitThunder OS
#

#
#	If we are building a fully fledged OS, then include the real memory manager.
#
include $(BASE)os/.config.mk

BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/bt_main.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/mm/bt_mm.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/mm/bt_heap.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/process/bt_process.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/process/bt_threads.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/interrupts/bt_interrupts.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/interrupts/bt_softirq.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/interrupts/bt_tasklets.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/gpio/bt_gpio.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/module/bt_module_init.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/devman/bt_devman.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/devman/bt_resources.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/machines/bt_machines.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/timers/bt_timers.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/process/bt_mutex.o



include $(BASE)os/src/interfaces/objects.mk


include $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/.config.mk
include $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/objects.mk
include $(BASE)arch/$(ARCH)/objects.mk
#SUB_OBJDIRS += $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/
#SUB_OBJDIRS += $(BASE)arch/$(ARCH)/

include $(BASE)drivers/objects.mk

test2:
	echo $(BT_CONFIG_KERNEL)
ifeq ($(BT_CONFIG_KERNEL), FreeRTOS)
$(BUILD_DIR)os/src/mm/bt_heap.o: CFLAGS += -DBT_CONFIG_KERNEL_FREERTOS
$(BUILD_DIR)os/src/mm/bt_mm.o: CFLAGS += -DBT_CONFIG_KERNEL_FREERTOS
endif

$(BUILD_DIR)os/src/interrupts/bt_interrupts.o: CFLAGS += -DBT_CONFIG_MAX_INTERRUPT_CONTROLLERS=$(BT_CONFIG_MAX_INTERRUPT_CONTROLLERS)
$(BUILD_DIR)os/src/interrupts/bt_interrupts.o: CFLAGS += -DBT_CONFIG_MAX_IRQ=$(BT_CONFIG_MAX_IRQ)
$(BUILD_DIR)os/src/gpio/bt_gpio.o: CFLAGS += -DBT_CONFIG_MAX_GPIO_CONTROLLERS=$(BT_CONFIG_MAX_GPIO_CONTROLLERS)
$(BUILD_DIR)os/src/process/bt_process.o: CFLAGS += -DBT_CONFIG_MAX_PROCESS_NAME=$(BT_CONFIG_MAX_PROCESS_NAME)

BT_OS_OBJECTS += $(BT_OS_OBJECTS-y)
$(BT_OS_OBJECTS): MODULE_NAME="BitThunder"

OBJECTS += $(BT_OS_OBJECTS)
