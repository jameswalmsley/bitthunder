BT_OS_OBJECTS-$(BT_CONFIG_OS) 						+= $(BUILD_DIR)os/src/mm/bt_mm.o
BT_OS_OBJECTS-$(BT_CONFIG_MEM_PAGE_ALLOCATOR) 		+= $(BUILD_DIR)os/src/mm/bt_page.o
BT_OS_OBJECTS-$(BT_CONFIG_MEM_KHEAP) 				+= $(BUILD_DIR)os/src/mm/bt_heap.o

BT_OS_OBJECTS-$(BT_CONFIG_USE_VIRTUAL_ADDRESSING) 	+= $(BUILD_DIR)os/src/mm/bt_map.o
BT_OS_OBJECTS-$(BT_CONFIG_MEM_SLAB_ALLOCATOR) 		+= $(BUILD_DIR)os/src/mm/slab.o
