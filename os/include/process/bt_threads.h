#ifndef _BT_THREADS_H_
#define _BT_THREADS_H_

#include <bt_types.h>

typedef struct _BT_THREAD_CONFIG {
	BT_u32		ulStackDepth;
	BT_u32		ulPriority;
	BT_u32		ulFlags;
#define BT_THREAD_FLAGS_START_SUSPENDED	0x00000001
#define BT_THREAD_FLAGS_AUTO_RESTART	0x00000002
	void 	   *pParam;
} BT_THREAD_CONFIG;


typedef BT_ERROR (*BT_FN_THREAD_ENTRY)	(BT_HANDLE hThread, void *pParam);



BT_HANDLE BT_CreateThread(BT_FN_THREAD_ENTRY pfnStartRoutine, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);
BT_HANDLE BT_CreateProcessThread(BT_HANDLE hProcess, BT_FN_THREAD_ENTRY pfnStartRoutine, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);

BT_HANDLE BT_GetThreadHandle();
BT_HANDLE BT_GetThreadProcessHandle(BT_HANDLE hThread);

BT_ERROR BT_ThreadSleepUntil(BT_TICK *pulPreviousWakeTime, BT_u32 ulTimeMs);
BT_ERROR BT_ThreadSleep(BT_u32 ulTimeMs);
BT_ERROR BT_ThreadYield();

#endif
