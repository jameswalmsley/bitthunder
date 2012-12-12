OBJECTS += $(BUILD_DIR)bsp/arm/raspberrypi/startup.o
OBJECTS += $(BUILD_DIR)bsp/arm/raspberrypi/init.o



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

