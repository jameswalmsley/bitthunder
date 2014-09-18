BT_OS_OBJECTS-$(BT_CONFIG_SYSCALL)	+= $(BUILD_DIR)/os/src/syscall/bt_syscall.o


#
#	Below are the implementations of the various POSIX system calls.
#	If CONFIG_SYSCALL is not enabled then these are still built as they maybe used directly
#	from the libc porting layer in BTDK or otherwise.
#
#	The linker will remove them all in case they aren't used.
#
#	However, compiling bt_syscall in will force each syscall implementation to remain in the compiled
#	kernel.
#

BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/getpid.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/yield.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/open.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/close.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/read.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/write.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/lseek.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/klog.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/sleep.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/gpio.o
BT_OS_OBJECTS 	+= $(BUILD_DIR)/os/src/syscall/calls/time.o
