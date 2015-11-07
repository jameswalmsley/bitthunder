#ifndef _BT_THREADS_H_
#define _BT_THREADS_H_

#include <bt_config.h>
#include <bt_types.h>

struct bt_thread {
	struct bt_task *task;
	void		   *tag;						///< A Tag item used e.g. by newlib for reent structures.
	const BT_i8    *name;
	BT_u64			ullRunTimeCounter;
	void 		   *pKThreadID;					///< FreeRTOS task handle.
};

struct bt_thread_time {
	BT_u64 		ullRunTimeCounter;
	BT_u32 		ulRunTimePercent;
	const BT_i8 	   *name;
};

extern struct bt_thread *curthread;
extern BT_u64 ullTaskSwitchedInTime;

typedef struct _BT_THREAD_CONFIG {
	BT_u32		ulStackDepth;
	BT_u32		ulPriority;
	BT_u32		ulFlags;
#define BT_THREAD_FLAGS_START_SUSPENDED	0x00000001
#define BT_THREAD_FLAGS_AUTO_RESTART	0x00000002
#define BT_THREAD_FLAGS_NO_CLEANUP		0x00000004
	void 	   *pParam;
} BT_THREAD_CONFIG;


typedef BT_ERROR (*BT_FN_THREAD_ENTRY)	(BT_HANDLE hThread, void *pParam);



BT_HANDLE BT_CreateThread(BT_FN_THREAD_ENTRY pfnStartRoutine, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);
BT_HANDLE BT_CreateProcessThread(BT_HANDLE hProcess, BT_FN_THREAD_ENTRY pfnStartRoutine, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);

BT_HANDLE BT_GetThreadHandle(void);
BT_HANDLE BT_GetThreadProcessHandle(BT_HANDLE hThread);
struct bt_thread *BT_GetThreadDescripter(BT_HANDLE hThread);

BT_ERROR BT_ThreadSleepUntil(BT_TICK *pulPreviousWakeTime, BT_u32 ulTimeMs);
BT_ERROR BT_ThreadSleep(BT_u32 ulTimeMs);
BT_ERROR BT_ThreadYield(void);
void *BT_GetThreadTag(void);
void BT_SetThreadTag(void *tag);
BT_ERROR BT_GetThreadTime(BT_HANDLE hThread, struct bt_thread_time *time);

#ifdef BT_CONFIG_KERNEL_NONE
#define BT_ThreadYield(x)
#define BT_ThreadSleep(x)
#define BT_ThreadSleepUntil(x, y)
#endif

#endif
