BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)/os/src/devman/bt_devman.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)/os/src/devman/bt_device.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)/os/src/devman/bt_resources.o
BT_OS_OBJECTS-$(BT_CONFIG_I2C) += $(BUILD_DIR)/os/src/devman/bt_i2c.o
BT_OS_OBJECTS-$(BT_CONFIG_SPI) += $(BUILD_DIR)/os/src/devman/bt_spi.o
BT_OS_OBJECTS-$(BT_CONFIG_BLOCK) += $(BUILD_DIR)/os/src/devman/bt_block.o


# Device Tree / Open firmware support
BT_OS_OBJECTS-$(BT_CONFIG_OF) += $(BUILD_DIR)/scripts/dtc/libfdt/fdt.o
BT_OS_OBJECTS-$(BT_CONFIG_OF) += $(BUILD_DIR)/scripts/dtc/libfdt/fdt_ro.o
BT_OS_OBJECTS-$(BT_CONFIG_OF) += $(BUILD_DIR)/scripts/dtc/libfdt/fdt_wip.o
BT_OS_OBJECTS-$(BT_CONFIG_OF) += $(BUILD_DIR)/scripts/dtc/libfdt/fdt_sw.o
BT_OS_OBJECTS-$(BT_CONFIG_OF) += $(BUILD_DIR)/scripts/dtc/libfdt/fdt_rw.o
BT_OS_OBJECTS-$(BT_CONFIG_OF) += $(BUILD_DIR)/scripts/dtc/libfdt/fdt_strerror.o
BT_OS_OBJECTS-$(BT_CONFIG_OF) += $(BUILD_DIR)/scripts/dtc/libfdt/fdt_empty_tree.o

CFLAGS += -I $(BASE)/scripts/dtc/libfdt/
