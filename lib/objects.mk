OBJECTS += $(BUILD_DIR)lib/src/handles/bt_handles.o


#
#	Include the correct Memory Manager implementation.
#
OBJECTS-$(BT_CONFIG_LIB) += $(BUILD_DIR)lib/src/mm/bt_mm.o

