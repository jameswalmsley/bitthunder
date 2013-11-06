#ifndef _BT_HANDLES_H_
#define _BT_HANDLES_H_

#include "bt_types.h"
#include "interfaces/bt_interfaces.h"
#include "collections/bt_list.h"

/**
 *	This is the primary object of BitThunder.
 *	Every HANDLE must be derived/based on this structure.
 **/
typedef struct _BT_HANDLE_HEADER {
	struct bt_list_head		list;
	const BT_IF_HANDLE	   *pIf;				///< Pointer to the handle interface.
	BT_u32					ulClaimedMemory;	///< Amount of memory claimed by this handle.
} BT_HANDLE_HEADER;


BT_ERROR 	BT_AttachHandle(BT_HANDLE hProcess, const BT_IF_HANDLE *pIf, BT_HANDLE h);
BT_HANDLE 	BT_CreateHandle	(const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError);
BT_ERROR 	BT_DestroyHandle(BT_HANDLE h);
BT_ERROR 	BT_CloseHandle	(BT_HANDLE hHandle);

BT_HANDLE 	BT_CreateMux(BT_u32 ulFlags, BT_ERROR *pError);
BT_ERROR 	BT_MuxOpen(BT_HANDLE hMux, const BT_i8 *path);
BT_ERROR 	BT_MuxAttach(BT_HANDLE hMux, BT_HANDLE h);

#endif
