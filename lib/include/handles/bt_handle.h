#ifndef _BT_HANDLES_H_
#define _BT_HANDLES_H_

#include "bt_types.h"
#include "interfaces/bt_interfaces.h"



typedef struct _BT_HANDLE_HEADER {
	const BT_IF_HANDLE	   *pIf;				///< Pointer to the handle interface.
	BT_u32					ulClaimedMemory;	///< Amount of memory claimed by this handle.
	BT_HANDLE				hNext;				///< Pointer to the next handle in the list.
} BT_HANDLE_HEADER;





BT_HANDLE BT_CreateHandle(const BT_IF_HANDLE *pIf, BT_u32 ulHandleMemory, BT_ERROR *pError);
BT_ERROR 	BT_CloseHandle(BT_HANDLE hHandle);






#endif
