/**
 *	BitThunder - File-system Manager.
 *
 **/

BT_DEF_MODULE_NAME			("Filesystem Manager")
BT_DEF_MODULE_DESCRIPTION	("Filesystem Mountpoint management")
BT_DEF_MODULE_AUTHOR	  	("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

static BT_LIST g_oFileSystems = {0};
static BT_LIST g_oMountPoints = {0};

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

typedef struct _BT_FILESYSTEM {
	BT_LIST_ITEM oItem;
	BT_HANDLE	 hFS;
} BT_FILESYSTEM;

BT_ERROR BT_RegisterFilesystem(BT_HANDLE hFS) {
	if(hFS->h.pIf->eType != BT_HANDLE_T_FILESYSTEM) {
		return BT_ERR_GENERIC;
	}

	BT_FILESYSTEM *pFilesystem = BT_kMalloc(sizeof(BT_FILESYSTEM));
	if(!pFilesystem) {
		return BT_ERR_NO_MEMORY;
	}

	pFilesystem->hFS = hFS;

	BT_ListAddItem(&g_oFileSystems, &pFilesystem->oItem);

	return BT_ERR_NONE;
}





static BT_ERROR bt_fs_init() {
	BT_ListInit(&oFileSystems);
	BT_ListInit(&oMountPoints);
	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_fs_init,
};
