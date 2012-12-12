#OBJECTS += $(BUILD_DIR)bsp/arm/zynq/zed-board/crtinit.o
OBJECTS += $(BUILD_DIR)bsp/arm/zynq/zed-board/startup.o
OBJECTS += $(BUILD_DIR)bsp/arm/zynq/zed-board/test.o
OBJECTS += $(BUILD_DIR)bsp/arm/zynq/zed-board/ps7_init.o

#
#	Compile in the BitThunder library code.
#
#	The actual objects included depends on your .config.mk file.
#
SUB_OBJDIRS += $(BASE)lib/

#
#	Compile in the BitThunder as an Operating System code.
#
#
#	This is found in your .config.mk file...
#	Note almost all if this will be excluded if BT_CONFIG_OS != y
#
SUB_OBJDIRS += $(BASE)os/
