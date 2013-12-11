include $(BASE)drivers/block/objects.mk

ifeq ($(BT_CONFIG_DRIVERS_NET), y)
include $(BASE)drivers/net/objects.mk
endif

ifeq ($(BT_CONFIG_DRIVERS_SDCARD), y)
include $(BASE)drivers/mmc/objects.mk
endif

ifeq ($(BT_CONFIG_DRIVERS_GPIO), y)
include $(BASE)drivers/gpio/objects.mk
endif

ifeq ($(BT_CONFIG_DRIVERS_MTD), y)
include $(BASE)drivers/mtd/objects.mk
endif

ifeq ($(BT_CONFIG_DRIVERS_RTC), y)
include $(BASE)drivers/rtc/objects.mk
endif
