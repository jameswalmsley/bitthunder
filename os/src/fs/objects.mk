BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/fs/bt_devfs.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/fs/bt_fs.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/fs/bt_file.o
BT_OS_OBJECTS-$(BT_CONFIG_OS) += $(BUILD_DIR)os/src/fs/bt_fullfat.o


# FullFAT Objects
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_blk.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_crc.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_dir.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_error.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_fat.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_file.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_hash.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_ioman.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_memory.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_safety.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_string.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_time.o
FF_OBJECTS-$(BT_CONFIG_FS_FULLFAT) += $(BUILD_DIR)os/src/fs/fullfat/ff_unicode.o

$(FF_OBJECTS-y): CFLAGS += -D FF_BITTHUNDER_CONFIG
$(FF_OBJECTS-y): MODULE_NAME="FullFAT"


OBJECTS += $(FF_OBJECTS-y)
