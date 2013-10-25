#ifndef _BT_DEV_IF_GTIMER_H_
#define _BT_DEV_IF_GTIMER_H_

#include "bt_types.h"

typedef struct _BT_DEV_IF_GTIMER {
	BT_u32	(*pfnGetClockRate)		(BT_HANDLE hTimer, BT_ERROR *pError);
	BT_u64	(*pfnGetValue)			(BT_HANDLE hTimer, BT_ERROR *pError);
} BT_DEV_IF_GTIMER;

#endif
