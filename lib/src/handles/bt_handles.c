/**
 *	Handle Creation and Management API.
 *
 **/

#include <bitthunder.h>


struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
};

BT_HANDLE BT_CreateHandle(const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError) {
	BT_HANDLE hHandle = BT_Calloc(ulHandleMemory);
	return hHandle;
}
