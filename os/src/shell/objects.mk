BT_OS_OBJECTS-$(BT_CONFIG_SHELL) += $(BUILD_DIR)os/src/shell/bt_shell.o


# Commands
BT_OS_OBJECTS-$(BT_CONFIG_SHELL_CMD_BOOT) 		+= $(BUILD_DIR)os/src/shell/commands/boot.o
BT_OS_OBJECTS-$(BT_CONFIG_SHELL_CMD_BOOT_JTAG) 	+= $(BUILD_DIR)os/src/shell/commands/boot_jtag.o
BT_OS_OBJECTS 									+= $(BUILD_DIR)os/src/shell/commands/echo.o
BT_OS_OBJECTS-$(BT_CONFIG_SHELL_CMD_HELP) 		+= $(BUILD_DIR)os/src/shell/commands/help.o
BT_OS_OBJECTS-$(BT_CONFIG_SHELL_CMD_LOAD) 		+= $(BUILD_DIR)os/src/shell/commands/load.o
BT_OS_OBJECTS-$(BT_CONFIG_SHELL_CMD_LOAD_FPGA) 	+= $(BUILD_DIR)os/src/shell/commands/load_fpga.o
