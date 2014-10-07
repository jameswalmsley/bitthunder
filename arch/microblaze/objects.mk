LINKER_SCRIPTS += $(BUILD_DIR)/arch/$(ARCH)/bitthunder.lds

OBJECTS += $(BUILD_DIR)/arch/$(ARCH)/kernel/port.o
OBJECTS += $(BUILD_DIR)/arch/$(ARCH)/kernel/portasm.o


$(BUILD_DIR)/arch/$(ARCH)/kernel/port.o: CFLAGS += -I $(BASE)/kernel/FreeRTOS/Source/include/
$(BUILD_DIR)/arch/$(ARCH)/kernel/port.o: CFLAGS += -I $(BASE)/arch/$(ARCH)/include/arch/common/
