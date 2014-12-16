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
	struct bt_task *task = BT_GetProcessTask(hProcess);
	switch(pIf->eType) {
	case BT_HANDLE_T_THREAD:
		bt_list_add(&h->h.item, &task->threads);
		break;

	default:
		bt_list_add(&h->h.item, &task->handles);
		break;
	}

	h->h.ulReferenceCount = 1;

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_AttachHandle);

BT_ERROR BT_DetachHandle(BT_HANDLE hProcess, BT_HANDLE h) {
	bt_list_del(&h->h.item);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DetachHandle);

BT_HANDLE BT_CreateHandleAttached(BT_HANDLE hProcess, const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError) {
	BT_HANDLE h = BT_Calloc(ulHandleMemory);
	BT_AttachHandle(hProcess, pIf, h);
	return h;
}
BT_EXPORT_SYMBOL(BT_CreateHandleAttached);

BT_ERROR BT_DestroyHandle(BT_HANDLE h) {
	BT_kFree(h);
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DestroyHandle);

BT_ERROR BT_RefHandle(BT_HANDLE h) {
	if(!h) {
		return BT_ERR_NULL_POINTER;
	}

	h->h.ulReferenceCount += 1;
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_RefHandle);

BT_ERROR BT_UnrefHandle(BT_HANDLE h) {
	if(!h) {
		return BT_ERR_NULL_POINTER;
	}

	h->h.ulReferenceCount -= 1;
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_UnrefHandle);

static BT_BOOL isHandleValid(BT_HANDLE h) {
	if(h) {
		return BT_TRUE;
	}

	return BT_FALSE;
}

BT_ERROR BT_CloseHandle(BT_HANDLE h) {
	if(!isHandleValid(h)) {
		return BT_ERR_INVALID_HANDLE;
	}

	BT_UnrefHandle(h);
	if(h->h.ulReferenceCount) {
		return BT_ERR_NONE;
	}

	BT_ERROR Error = BT_ERR_NONE;
	if(h->h.pIf->pfnCleanup) {
		h->h.pIf->pfnCleanup(h);
	}

	BT_DetachHandle(NULL, h);	// Detach the handle from the current process.

	if(!(h->h.pIf->ulFlags & BT_HANDLE_FLAGS_NO_DESTROY)) {
		BT_DestroyHandle(h);
	}

	return Error;
}
BT_EXPORT_SYMBOL(BT_CloseHandle);
