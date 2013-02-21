#ifndef _BT_HANDLES_H_
#define _BT_HANDLES_H_

#include "bt_types.h"
#include "interfaces/bt_interfaces.h"
#include "collections/bt_linked_list.h"

/**
 *	This is the primary object of BitThunder.
 *	Every HANDLE must be derived/based on this structure.
 **/
typedef struct _BT_HANDLE_HEADER {
	const BT_IF_HANDLE	   *pIf;				///< Pointer to the handle interface.
	BT_u32					ulClaimedMemory;	///< Amount of memory claimed by this handle.
	BT_LIST_ITEM			oItem;				///< Allows a handle to be added to any list!
} BT_HANDLE_HEADER;

BT_HANDLE 	BT_CreateHandle	(const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError);
BT_ERROR 	BT_CloseHandle	(BT_HANDLE hHandle);

#endif
