CFLAGS += -march=armv6z -ggdb3 -I $(BASE)/lib/include/ -I $(BASE)/arch/arm/include/ -I $(BASE)/os/include/

TOOLCHAIN:=$(shell echo $(BT_CONFIG_TOOLCHAIN))

#TOOLCHAIN=arm-none-eabi-
