#
#	BitThunder OS
#

#
#	If we are building a fully fledged OS, then include the real memory manager.
#
OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/mm/bt_mm.o

