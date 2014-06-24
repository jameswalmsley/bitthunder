/**
 *	BitThunder - Multiplexer - Combines multiple handles with a FILE_IF into a single handle.
 *
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("BT Multiplexer")
BT_DEF_MODULE_DESCRIPTION	("Multiplex FILE_IF handles into a single convenient HANDLE")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct handle_item {
	struct bt_list_head item;
	BT_HANDLE hHandle;
	BT_u32 flags;
	#define FLAG_ATTACHED	0x00000001
};

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	struct bt_list_head handles;
	BT_u32 mode_flags;
};

static BT_s32 mux_file_write(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, const void *pBuffer) {

	struct bt_list_head *pos;
	bt_list_for_each(pos, &hFile->handles) {
		struct handle_item *item = (struct handle_item *) pos;
		BT_s32 ret = BT_Write(item->hHandle, hFile->mode_flags, ulSize, pBuffer);
		if(ret < 0) {
			return ret;
		}
	}

	return ulSize;
}

static BT_s32 mux_file_read(BT_HANDLE hFile, BT_u32 ulFlags, BT_u32 ulSize, void *pBuffer) {

	struct bt_list_head *pos;
	bt_list_for_each(pos, &hFile->handles) {
		struct handle_item *item = (struct handle_item *) pos;
		BT_s32 read = BT_Read(item->hHandle, hFile->mode_flags, ulSize, pBuffer);
		if(read) {
			return read;
		}
	}

	return 0;
}

static BT_IF_FILE mux_file_ops = {
	.pfnWrite = mux_file_write,
	.pfnRead = mux_file_read,
};

static BT_u32 mux_cleanup(BT_HANDLE hFile) {
	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_FILE,
	.pFileIF = &mux_file_ops,
	.pfnCleanup = mux_cleanup,
};

BT_HANDLE BT_CreateMux(BT_u32 ulFlags, BT_ERROR *pError) {
	BT_ERROR Error = BT_ERR_NONE;

	BT_HANDLE hMux = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), &Error);
	if(!hMux) {
		goto err_out;
	}

	BT_LIST_INIT_HEAD(&hMux->handles);
	hMux->mode_flags = ulFlags;

	return hMux;

err_out:
	if(pError) {
		*pError = Error;
	}

	return NULL;
}
BT_EXPORT_SYMBOL(BT_CreateMux);

static BT_ERROR mux_attach(BT_HANDLE hMux, BT_HANDLE h, BT_u32 flags) {
	struct handle_item *item = BT_kMalloc(sizeof(*item));
	if(!item) {
		return BT_ERR_NO_MEMORY;
	}

	item->hHandle = h;
	item->flags = flags;

	bt_list_add(&item->item, &hMux->handles);

	return BT_ERR_NONE;
}

BT_ERROR BT_MuxOpen(BT_HANDLE hMux, const BT_i8 *path) {

	BT_ERROR Error = BT_ERR_NONE;

	BT_HANDLE h = BT_Open(path, 0, &Error);
	if(!h) {
		return BT_ERR_GENERIC;
	}

	return mux_attach(hMux, h, 0);
}
BT_EXPORT_SYMBOL(BT_MuxOpen);

BT_ERROR BT_MuxAttach(BT_HANDLE hMux, BT_HANDLE h) {
	return mux_attach(hMux, h, FLAG_ATTACHED);
}
BT_EXPORT_SYMBOL(BT_MuxAttach);
