ifeq ($(BT_CONFIG_DRIVERS_SDCARD), y)
include $(BASE)drivers/mmc/objects.mk
endif
