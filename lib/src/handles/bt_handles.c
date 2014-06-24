/**
 *	Handle Creation and Management API.
 *
 **/

#include <bitthunder.h>

#ifdef BT_CONFIG_OS
#include <mm/bt_heap.h>
#endif

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};


BT_ERROR BT_AttachHandle(BT_HANDLE hProcess, const BT_IF_HANDLE *pIf, BT_HANDLE h) {
	h->h.pIf = pIf;
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_AttachHandle);

BT_ERROR BT_DetachHandle(BT_HANDLE hProcess, BT_HANDLE h) {

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DetachHandle);

BT_HANDLE BT_CreateHandle(const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError) {
	BT_HANDLE hHandle = BT_Calloc(ulHandleMemory);
	BT_AttachHandle(NULL, pIf, hHandle);
	return hHandle;
}
BT_EXPORT_SYMBOL(BT_CreateHandle);

BT_ERROR BT_DestroyHandle(BT_HANDLE h) {
	BT_kFree(h);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DestroyHandle);

static BT_BOOL isHandleValid(BT_HANDLE h) {
	if(h) {
		return BT_TRUE;
	}

	return BT_FALSE;
}

BT_ERROR BT_CloseHandle(BT_HANDLE hHandle) {
	if(!isHandleValid(hHandle)) {
		return BT_ERR_INVALID_HANDLE;
	}

	BT_ERROR Error = BT_ERR_NONE;
	if(hHandle->h.pIf->pfnCleanup) {
		hHandle->h.pIf->pfnCleanup(hHandle);
	}

	if(!(hHandle->h.pIf->ulFlags & BT_HANDLE_FLAGS_NO_DESTROY)) {
		BT_DestroyHandle(hHandle);
	}

	return Error;
}
BT_EXPORT_SYMBOL(BT_CloseHandle);
