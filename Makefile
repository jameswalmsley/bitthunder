#
#	BitThunder Top-Level Makefile
#

MAKEFLAGS += -rR --no-print-directory

-include .config

Q=@

all:

ifeq ($(BT_CONFIG_CONFIGURED),y)
ifeq ($(BT_CONFIG_BSP_DIR),)
	$(Q)echo "BSP has not configured BT_CONFIG_BSP_DIR"
else
all: scripts/kconfig/mkconfig
	$(Q)echo " Building BitThunder for $(BT_CONFIG_BSP_NAME)"
	$(Q)$(MAKE) -C $(BT_CONFIG_BSP_DIR)
endif
else
all:
	$(Q)make .config
endif


menuconfig: scripts/mkconfig/mkconfig
	$(Q)CONFIG_=BT_CONFIG_ APP_DIR=$(APP_DIR) kconfig-mconf Kconfig
	$(Q)scripts/mkconfig/mkconfig ./ > $(BT_CONFIG_BSP_DIR)/bt_bsp_config.h
	$(Q)cp .config $(BT_CONFIG_BSP_DIR)/.config


scripts/mkconfig/mkconfig: scripts/mkconfig/mkconfig.c
	$(Q)gcc scripts/mkconfig/mkconfig.c scripts/mkconfig/cfgparser.c scripts/mkconfig/cfgdefine.c -o scripts/mkconfig/mkconfig

ifneq ($(BT_CONFIG_BSP_DIR),)
clean:
	$(Q)echo " Cleaning $(BT_CONFIG_BSP_NAME) Board Support Package"
	$(Q)$(MAKE) -C $(BT_CONFIG_BSP_DIR) clean
endif

.PHONY: menuconfig
