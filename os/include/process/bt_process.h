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
};

#define curtask 	curthread->task

BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);

BT_HANDLE BT_GetProcessHandle(void);
struct bt_task *BT_GetProcessTask(BT_HANDLE hProces);
BT_LIST *BT_GetProcessThreadList(BT_HANDLE hProcess);

#endif
