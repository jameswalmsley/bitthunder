#
#	BitThunder OS
#

#
#	If we are building a fully fledged OS, then include the real memory manager.
#
include $(BASE)os/.config.mk

BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/lib/string.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/lib/ctype.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/lib/bcd.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/bt_main.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/syscall/bt_syscall.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/syscall/getpid/getpid.o
BT_OS_OBJECTS-$(BT_CONFIG_PROCESS) += $(BUILD_DIR)os/src/process/bt_process.o
BT_OS_OBJECTS-$(BT_CONFIG_THREADS) += $(BUILD_DIR)os/src/process/bt_threads.o
BT_OS_OBJECTS-$(BT_CONFIG_ALIVE_LED) += $(BUILD_DIR)os/src/process/bt_alive_led.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/interrupts/bt_interrupts.o
BT_OS_OBJECTS-$(BT_CONFIG_INTERRUPTS_SOFTIRQ) += $(BUILD_DIR)os/src/interrupts/bt_softirq.o
BT_OS_OBJECTS-$(BT_CONFIG_TASKLETS) += $(BUILD_DIR)os/src/interrupts/bt_tasklets.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/gpio/bt_gpio.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/rtc/bt_rtc.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/module/bt_module_init.o
BT_OS_OBJECTS-$(BT_CONFIG_VOLUME) += $(BUILD_DIR)os/src/volumes/bt_volume.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/machines/bt_machines.o
BT_OS_OBJECTS-$(BT_CONFIG_TIMERS) += $(BUILD_DIR)os/src/timers/bt_timers.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/process/bt_mutex.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/process/bt_queue.o
BT_OS_OBJECTS-$(BT_CONFIG_LIB_PRINTF) += $(BUILD_DIR)os/src/lib/printf.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/lib/getmem.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/lib/putc.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/syslog/bt_printk.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/lib/multiplexer.o

include $(BASE)os/src/mm/objects.mk
include $(BASE)os/src/devman/objects.mk
include $(BASE)os/src/fs/objects.mk
include $(BASE)os/src/net/objects.mk
include $(BASE)os/src/shell/objects.mk

include $(BASE)os/src/interfaces/objects.mk
include $(BASE)os/src/helpers/objects.mk


ifeq ($(BT_CONFIG_OS),y)
include $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/.config.mk
include $(BASE)arch/$(ARCH)/mach/$(SUBARCH)/objects.mk
include $(BASE)arch/$(ARCH)/objects.mk
endif


include $(BASE)drivers/objects.mk

ifeq ($(BT_CONFIG_KERNEL_FREERTOS),y)
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

APP=$(BUILD_DIR)application/
-include $(APP_DIR)/objects.mk

objs += $(objs-y)

APP_OBJS := $(objs)
#$(patsubst %, $(BUILD_DIR)application/%, $(objs))
#$(APP_OBJS): MODULE_NAME="application"

appobjs:
	@echo including $(APP_DIR)
	@echo $(APP_OBJS)

OBJECTS += $(APP_OBJS)

$(TARGET_DEPS): $(LINKER_SCRIPTS)

$(LINKER_SCRIPTS):$(BASE)os/include/btlinker_config.h
$(LINKER_SCRIPTS):$(BASE)os/include/bitthunder.lds.h
$(LINKER_SCRIPTS):$(BASE)lib/include/bt_config.h
$(LINKER_SCRIPTS):$(BSP_DIR)bt_bsp_config.h


$(LINKER_SCRIPTS): CFLAGS=-I $(BASE)lib/include/ -I $(BASE)arch/$(ARCH)/include/ -I $(BASE)os/include/

linker:
	@echo $(LINKER_SCRIPTS)

LINKER_SCRIPT=$(LINKER_SCRIPTS)
