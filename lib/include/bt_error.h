#ifndef _BT_ERROR_H_
#define _BT_ERROR_H_

#include "bt_types.h"

typedef BT_u32	BT_ERROR;






#define BT_ERR_DEF_GLOBAL(x)		(x)

#define BT_ERR_NONE					0

#define BT_ERR_GENERIC				BT_ERR_DEF_GLOBAL(1)
#define BT_ERR_NULL_POINTER			BT_ERR_DEF_GLOBAL(2)
#define BT_ERR_NO_MEMORY 			BT_ERR_DEF_GLOBAL(3)
#define BT_ERR_UNIMPLEMENTED		BT_ERR_DEF_GLOBAL(4)
#define BT_ERR_INVALID_HANDLE		BT_ERR_DEF_GLOBAL(5)

#endif
