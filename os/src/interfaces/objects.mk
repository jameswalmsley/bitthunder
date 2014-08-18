BT_OS_INTERFACE_OBJECTS += $(BUILD_DIR)/os/src/interfaces/bt_if_cpu.o
BT_OS_INTERFACE_OBJECTS += $(BUILD_DIR)/os/src/interfaces/bt_if_power.o

BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_CAN) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_can.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_UART) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_uart.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_I2C) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_i2c.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_TIMER) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_timer.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_ADC) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_adc.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_DAC) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_dac.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_PWM) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_pwm.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_QEI) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_qei.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_MCPWM) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_mcpwm.o
BT_OS_INTERFACE_OBJECTS-$(BT_CONFIG_MTD) 	+= $(BUILD_DIR)/os/src/interfaces/bt_dev_if_mtd.o


BT_OS_INTERFACE_OBJECTS += $(BT_OS_INTERFACE_OBJECTS-y)

BT_OS_OBJECTS += $(BT_OS_INTERFACE_OBJECTS)
