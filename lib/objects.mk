BT_LIB_OBJECTS += $(BUILD_DIR)lib/src/handles/bt_handles.o
BT_LIB_OBJECTS += $(BUILD_DIR)lib/src/collections/bt_linked_list.o


#
#	Include the correct Memory Manager implementation.
#
BT_LIB_OBJECTS-$(BT_CONFIG_LIB) += $(BUILD_DIR)lib/src/mm/bt_mm.o

BT_LIB_OBJECTS += $(BT_LIB_OBJECTS-y)

$(BT_LIB_OBJECTS): MODULE_NAME="BitThunder"

OBJECTS += $(BT_LIB_OBJECTS)
