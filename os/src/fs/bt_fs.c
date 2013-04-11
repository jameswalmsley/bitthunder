/**
 *	BitThunder - File-system Manager.
 *
 **/

BT_DEF_MODULE_NAME			("Filesystem Manager")
BT_DEF_MODULE_DESCRIPTION	("Filesystem Mountpoint management")
BT_DEF_MODULE_AUTHOR	  	("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_LIST oFileSystems = {0};
static BT_LIST oMountPoints = {0};



















static BT_ERROR bt_fs_init() {
	BT_ListInit(&oFileSystems);
	BT_ListInit(&oMountPoints);
	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_fs_init,
};
