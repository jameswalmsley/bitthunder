BASE=$(shell pwd)/../../
BUILD_BASE=$(shell pwd)/
BUILD_DIR=$(shell pwd)/build/

MODULE_NAME=mkimage

CFLAGS += -I $(BASE)/scripts/dtc/libfdt/

include $(BASE)/.dbuild/dbuild.mk

all: mkimage
mkimage: $(OBJECTS)
	$(CC) -o $@ $(OBJECTS)

