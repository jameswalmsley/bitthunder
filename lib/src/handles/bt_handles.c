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

BT_HANDLE BT_CreateHandle(const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError) {
	BT_HANDLE hHandle = BT_Calloc(ulHandleMemory);
	hHandle->h.pIf = pIf;
	hHandle->h.ulClaimedMemory = ulHandleMemory;
	return hHandle;
}

BT_ERROR BT_DestroyHandle(BT_HANDLE h) {
	BT_kFree(h);
	return BT_ERR_NONE;
}

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
	BT_ERROR Error = hHandle->h.pIf->pfnCleanup(hHandle);
	if(!Error) {
		BT_DestroyHandle(hHandle);
	}

	if(Error == BT_HANDLE_DO_NOT_FREE) {
		Error = BT_ERR_NONE;
	}

	return Error;
}
