#ifndef _BT_THREADS_H_
#define _BT_THREADS_H_

#include <bt_types.h>

typedef struct _BT_THREAD_CONFIG {
	BT_u32		ulStackDepth;
	BT_u32		ulPriority;
	BT_BOOL		bStartSuspended;
	BT_BOOL		bAutoRestart;
	void 	   *pParam;
} BT_THREAD_CONFIG;





#endif
