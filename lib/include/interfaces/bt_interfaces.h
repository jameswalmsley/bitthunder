#ifndef _BT_INTERFACES_H_
#define _BT_INTERFACES_H_

#include "bt_if_device.h"
#include "bt_if_module.h"

typedef BT_ERROR 	(*BT_HANDLE_CLEANUP)		(BT_HANDLE hHandle);

typedef enum _BT_HANDLE_TYPE {
	BT_HANDLE_T_INVALID 	= 0,
	BT_HANDLE_T_MODULE,
	BT_HANDLE_T_DEVICE,
} BT_HANDLE_TYPE;

typedef union _BT_IF_INTERFACES {
	const BT_IF_DEVICE *pDevIF;
} BT_UN_IFS;


typedef struct _BT_IF_HANDLE {
	const BT_s8		   *szpModuleName;
	const BT_s8		   *szpDescription;
	const BT_s8		   *szpAuthor;
	const BT_s8		   *szpEmail;

	const BT_UN_IFS	   *pIfs;

	BT_HANDLE_TYPE 		eType;
	BT_HANDLE_CLEANUP	pfnCleanup;
} BT_IF_HANDLE;


#endif
