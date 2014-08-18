BT_OS_OBJECTS-$(BT_CONFIG_OS) 		+= $(BUILD_DIR)/os/src/fs/bt_devfs.o
BT_OS_OBJECTS-$(BT_CONFIG_FS)	 	+= $(BUILD_DIR)/os/src/fs/bt_mountfs.o
BT_OS_OBJECTS-$(BT_CONFIG_FS) 		+= $(BUILD_DIR)/os/src/fs/bt_fs.o
BT_OS_OBJECTS-$(BT_CONFIG_FILE) 	+= $(BUILD_DIR)/os/src/fs/bt_file.o
BT_OS_OBJECTS-$(BT_CONFIG_DIR) 		+= $(BUILD_DIR)/os/src/fs/bt_dir.o
BT_OS_OBJECTS-$(BT_CONFIG_INODE) 	+= $(BUILD_DIR)/os/src/fs/bt_inode.o


# FullFAT Objects
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/bt_fullfat.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_blk.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_crc.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_dir.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_error.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_fat.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_file.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_hash.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_ioman.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_memory.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_safety.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_string.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_time.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)/os/src/fs/fullfat/ff_unicode.o


BT_CC_CFLAGS := $(shell echo $(BT_CONFIG_CFLAGS))

$(FF_OBJECTS-y): CFLAGS += -D FF_BITTHUNDER_CONFIG $(BT_CC_CFLAGS)
$(FF_OBJECTS-y): MODULE_NAME="FullFAT"

OBJECTS += $(FF_OBJECTS-y)


# Ext2 Objects
EXT2_OBJECTS-$(BT_CONFIG_FS_EXT2) += $(BUILD_DIR)/os/src/fs/bt_ext2.o
EXT2_OBJECTS-$(BT_CONFIG_FS_EXT2) += $(BUILD_DIR)/os/src/fs/ext2/ext2fs.o
EXT2_OBJECTS-$(BT_CONFIG_FS_EXT2) += $(BUILD_DIR)/os/src/fs/ext2/dev.o


$(EXT2_OBJECTS-y): MODULE_NAME="Ext2"

OBJECTS += $(EXT2_OBJECTS-y)
