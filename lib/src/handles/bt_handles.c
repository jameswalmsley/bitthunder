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


BT_ERROR BT_CloseHandle(BT_HANDLE hHandle) {
	BT_ERROR Error = hHandle->h.pIf->pfnCleanup(hHandle);
	if(!Error) {
		BT_kFree(hHandle);
	}
	return Error;
}
