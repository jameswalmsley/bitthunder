t_OBJECTS :=

t_OBJECTS += $(BUILD_DIR)Kernel/FreeRTOS/Source/croutine.o
t_OBJECTS += $(BUILD_DIR)Kernel/FreeRTOS/Source/list.o
t_OBJECTS += $(BUILD_DIR)Kernel/FreeRTOS/Source/queue.o
t_OBJECTS += $(BUILD_DIR)Kernel/FreeRTOS/Source/tasks.o

$(t_OBJECTS): MODULE_NAME="FreeRTOS"
$(t_OBJECTS): CFLAGS +=-I $(BASE)kernel/FreeRTOS/Source/include/

OBJECTS += $(t_OBJECTS)
