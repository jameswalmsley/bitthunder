/**
 *	Process abstraction API for BitThunder.
 **/

#include <bitthunder.h>
#include <string.h>
#include <mm/bt_vm.h>

BT_DEF_MODULE_NAME			("Process Manager")
BT_DEF_MODULE_DESCRIPTION	("OS Process abstraction for the BitThunder Kernel")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	struct bt_task		task;
	BT_THREAD_CONFIG	oConfig;
	BT_u32				flags;
	BT_BOOL				bIsStarted;		///< Flag process started, prevent dual starting!
	BT_BOOL				bAutoRestart;	///< Flag allow auto-restarting of process.
	BT_u16				usPID;			///< ProcessID of this process.
};

static struct bt_list_head process_handles;
static BT_u16 usLastPID = 0;

struct _BT_OPAQUE_HANDLE kernel_handle;
struct bt_task *kernel_task = &kernel_handle.task;

static const BT_IF_HANDLE oHandleInterface;

BT_ERROR BT_StartScheduler() {
	return BT_kStartScheduler();
}

BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {
	BT_HANDLE hProcess = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hProcess) {
		return NULL;
	}

	hProcess->usPID 		= ++usLastPID;

	memcpy(&hProcess->oConfig, pConfig, sizeof(BT_THREAD_CONFIG));
	strncpy(hProcess->task.name, szpName, BT_CONFIG_MAX_PROCESS_NAME);

	BT_LIST_INIT_HEAD(&hProcess->task.threads);

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	hProcess->task.map = bt_vm_create();
#endif

	BT_CreateProcessThread(hProcess, pfnStartRoutine, &hProcess->oConfig, pError);

	bt_list_add(&hProcess->h.list, &process_handles);

	return hProcess;
}

BT_ERROR BT_DestroyProcess(BT_HANDLE hProcess) {

	return BT_ERR_NONE;
}

BT_HANDLE BT_GetProcessHandle(void) {
	if(curthread && curtask) {
		struct bt_task *task = curtask;
		return bt_container_of(task, struct _BT_OPAQUE_HANDLE, task, struct bt_task);
	}
	return NULL;
}

struct bt_task *BT_GetProcessTask(BT_HANDLE hProcess) {
	return &hProcess->task;
}

static BT_ERROR bt_process_cleanup(BT_HANDLE hProcess) {

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_PROCESS,
	.pfnCleanup = bt_process_cleanup,
};

extern struct bt_thread idle_thread;

static BT_ERROR bt_process_manager_init() {

	// Create the kernel process handle!
	BT_LIST_INIT_HEAD(&process_handles);
	strncpy(kernel_handle.task.name, "kernel", BT_CONFIG_MAX_PROCESS_NAME);
#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	kernel_handle.task.map = bt_vm_get_kernel_map();
#endif
	idle_thread.task = &kernel_handle.task;

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	.name = BT_MODULE_NAME,
	bt_process_manager_init,
};
