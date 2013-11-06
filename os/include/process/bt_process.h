#ifndef _BT_PROCESS_H_
#define _BT_PROCESS_H_

#include <collections/bt_list.h>

struct bt_task {
	BT_i8				name[BT_CONFIG_MAX_PROCESS_NAME+1];
	struct bt_task 	   *parent;
	struct bt_vm_map   *map;
	BT_u32				flags;
	struct bt_list_head threads;
	struct bt_list_head handles;
	BT_u64 				ullRunTimeCounter;
	BT_HANDLE			fds[8];
};

struct bt_process_time {
	BT_u64 		ullRunTimeCounter;
	BT_u32 		ulRunTimePercent;
	BT_HANDLE 	hProcess;
	char 	   *name;
};

#define curtask 	curthread->task

BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);

BT_HANDLE BT_GetProcessHandle(void);
struct bt_task *BT_GetProcessTask(BT_HANDLE hProces);
BT_LIST *BT_GetProcessThreadList(BT_HANDLE hProcess);

BT_ERROR BT_GetProcessTime(struct bt_process_time *time, BT_u32 i);
BT_u32 BT_GetTotalProcesses();

BT_ERROR BT_SetFileDescriptor(BT_u32 i, BT_HANDLE h);
BT_HANDLE BT_GetFileDescriptor(BT_u32 i, BT_ERROR *pError);

#endif
