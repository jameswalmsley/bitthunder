ifeq ($(BT_CONFIG_DRIVERS_SDCARD), y)
include $(BASE)drivers/mmc/objects.mk
endif

ifeq ($(BT_CONFIG_DRIVERS_GPIO), y)
include $(BASE)drivers/gpio/objects.mk
endif

ifeq ($(BT_CONFIG_DRIVERS_RTC), y)
include $(BASE)drivers/rtc/objects.mk
endif
