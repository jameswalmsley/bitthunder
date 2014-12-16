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
};

static struct bt_list_head process_handles;
static BT_u32 total_processes = 0;
static BT_u16 usLastPID = 0;

struct _BT_OPAQUE_HANDLE kernel_handle;

static const BT_IF_HANDLE oHandleInterface;

BT_ERROR BT_StartScheduler() {
	return BT_kStartScheduler();
}


static BT_s32 task_alloc_fd(struct bt_task *task);

BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {
	BT_HANDLE hProcess = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hProcess) {
		return NULL;
	}

	hProcess->task.pid 		= ++usLastPID;

	memcpy(&hProcess->oConfig, pConfig, sizeof(BT_THREAD_CONFIG));
	strncpy(hProcess->task.name, szpName, BT_CONFIG_MAX_PROCESS_NAME);

	BT_LIST_INIT_HEAD(&hProcess->task.threads);
	BT_LIST_INIT_HEAD(&hProcess->task.handles);

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	hProcess->task.map = bt_vm_create();
#endif

	BT_CreateProcessThread(hProcess, pfnStartRoutine, &hProcess->oConfig, pError);

	bt_list_add(&hProcess->h.item, &process_handles);

	BT_HANDLE hParent = BT_GetProcessHandle();
	hProcess->task.parent = &hParent->task;
#ifdef BT_CONFIG_PROCESS_CWD
	strcpy(hProcess->task.cwd, hParent->task.cwd);	// Iherit the current working directory from parent.
#endif

	total_processes += 1;

	// Inherit the file-descriptors from the parent.
	BT_u32 i;
	for(i = 0; i < (sizeof(hProcess->task.fds)/sizeof(BT_HANDLE)); i++) {
		BT_RefHandle(hProcess->task.parent->fds[i]);
	}

	memcpy(hProcess->task.fds, hProcess->task.parent->fds, sizeof(hProcess->task.fds));
	hProcess->task.free_fd = hProcess->task.parent->free_fd;

	return hProcess;
}
BT_EXPORT_SYMBOL(BT_CreateProcess);

BT_ERROR BT_DestroyProcess(BT_HANDLE hProcess) {

	// Kill the process in the scheduler.

	// All sub-processes should be migrated to the parent.

	// If there is no parent, then the sub-processes must also be killed first.

	// Suspend each thread. (Except calling thread).
	// Close the an open handles.
	struct bt_list_head *pos;
	bt_list_for_each(pos, &hProcess->task.threads) {
		BT_HANDLE h = bt_container_of(pos, struct _BT_OPAQUE_HANDLE, h.item);
		BT_CloseHandle(h);
	}

	// Call the vTaskDelete allowing cleanup to occur in IDLE time.


	// If currently running process then schedule cleanup and yield.
	if(BT_GetProcessHandle() == hProcess) {
		// Schedule for closing in IDLE.
		return BT_ERR_NONE;
	}

	BT_u32 i = 0;
	for(i = 0; i < sizeof(hProcess->task.fds)/sizeof(BT_HANDLE); i++) {
		BT_SetProcessFileDescriptor(hProcess, i, NULL);	// This will cause the handle to be closed or unreferenced.
	}

	// Close all open file-descriptors.

	// Close the an open handles.
	bt_list_for_each(pos, &hProcess->task.handles) {
		BT_HANDLE h = bt_container_of(pos, struct _BT_OPAQUE_HANDLE, h.item);
		BT_CloseHandle(h);
	}

	// Destroy the memory map.

	// Kill


	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_DestroyProcess);

BT_HANDLE BT_GetProcessHandle(void) {
	if(curthread && curtask) {
		struct bt_task *task = curtask;
		return bt_container_of(task, struct _BT_OPAQUE_HANDLE, task);
	}
	return (BT_HANDLE) &kernel_handle;
}
BT_EXPORT_SYMBOL(BT_GetProcessHandle);

BT_HANDLE BT_GetKernelProcessHandle(void) {
	return (BT_HANDLE) &kernel_handle;
}
BT_EXPORT_SYMBOL(BT_GetKernelProcessHandle);

BT_HANDLE BT_GetParentProcessHandle(void) {
	struct bt_task *task = BT_GetProcessTask(NULL);
	BT_HANDLE hParent = (BT_HANDLE) bt_container_of(task->parent, struct _BT_OPAQUE_HANDLE, task);
	return hParent;
}
BT_EXPORT_SYMBOL(BT_GetParentProcessHandle);

struct bt_task *BT_GetProcessTask(BT_HANDLE hProcess) {
	if(hProcess) {
		return &hProcess->task;
	}

	return &BT_GetProcessHandle()->task;
}
BT_EXPORT_SYMBOL(BT_GetProcessTask);

BT_ERROR BT_GetProcessTime(struct bt_process_time *time, BT_u32 pid) {
	struct bt_list_head *pos;
	BT_HANDLE hProcess = NULL;

	bt_list_for_each(pos, &process_handles) {
		if(!pid--) {
			hProcess = (BT_HANDLE) pos;
			break;
		}
	}

	if(!hProcess) {
		return BT_ERR_GENERIC;
	}

	BT_u32 ulThreads = 0;
	bt_list_for_each(pos, &hProcess->task.threads) {
		ulThreads++;
	}

	time->hProcess 			= hProcess;
	time->ullRunTimeCounter = hProcess->task.ullRunTimeCounter;
	time->ulRunTimePercent 	= ((time->ullRunTimeCounter) / (BT_GetGlobalTimer() / 100));
	time->name 				= hProcess->task.name;
	time->ulPID				= hProcess->task.pid;
	time->ulThreads			= ulThreads;

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_GetProcessTime);

BT_ERROR BT_GetProcessThreadTime(struct bt_thread_time *time, BT_u32 pid, BT_u32 tid) {
	struct bt_list_head *pos;
	BT_HANDLE hProcess = NULL;

	bt_list_for_each(pos, &process_handles) {
		if(!pid--) {
			hProcess = (BT_HANDLE) pos;
			break;
		}
	}

	if(!hProcess) {
		return BT_ERR_GENERIC;
	}

	BT_HANDLE hThread = NULL;
	bt_list_for_each(pos, &hProcess->task.threads) {
		if(!tid--) {
			hThread = (BT_HANDLE) pos;
			break;
		}
	}

	if(!hThread) {
		return BT_ERR_GENERIC;
	}

	BT_GetThreadTime(hThread, time);

	return BT_ERR_NONE;
}

BT_u32 BT_GetTotalProcesses() {
	return total_processes;
}
BT_EXPORT_SYMBOL(BT_GetTotalProcesses);

BT_ERROR BT_SetProcessFileDescriptor(BT_HANDLE hProcess, BT_u32 i, BT_HANDLE h) {
	struct bt_task *task = BT_GetProcessTask(hProcess);
	BT_CloseHandle(task->fds[i]);	// Unreference or close handle used.
	BT_RefHandle(h);				// Ensure we reference the handle when creating an FD.
	task->fds[i] = h;
	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_SetProcessFileDescriptor);

BT_HANDLE BT_GetProcessFileDescriptor(BT_HANDLE hProcess, BT_u32 i, BT_ERROR *pError) {
	struct bt_task *task = BT_GetProcessTask(hProcess);
	return task->fds[i];
}
BT_EXPORT_SYMBOL(BT_GetProcessFileDescriptor);

BT_ERROR BT_SetFileDescriptor(BT_u32 i, BT_HANDLE h) {
	BT_HANDLE hProcess = BT_GetProcessHandle();
	return BT_SetProcessFileDescriptor(hProcess, i, h);
}
BT_EXPORT_SYMBOL(BT_SetFileDescriptor);

BT_HANDLE BT_GetFileDescriptor(BT_u32 i, BT_ERROR *pError) {
	BT_HANDLE hProcess = BT_GetProcessHandle();
	return BT_GetProcessFileDescriptor(hProcess, i, pError);
}
BT_EXPORT_SYMBOL(BT_GetFileDescriptor);

static BT_s32 task_alloc_fd(struct bt_task *task) {
	BT_s32 i;
	BT_u32 mask = 0x80000000 >> task->free_fd;

	for(i = task->free_fd; i < 8; i++) {
		if(!(mask & task->flags)) {
			task->flags |= mask;
			task->free_fd = i;
			return i;
		}

		mask >>= 1;
	}

	return BT_ERR_GENERIC;
}

BT_s32 BT_AllocFileDescriptor() {
	struct bt_task *task = BT_GetProcessTask(NULL);
	return task_alloc_fd(task);
}
BT_EXPORT_SYMBOL(BT_AllocFileDescriptor);

static BT_ERROR task_free_fd(struct bt_task *task, BT_s32 fd) {
	if(fd < 0) {
		return BT_ERR_GENERIC;
	}

	if(fd < 8) {
		BT_u32 mask = 0x80000000 >> fd;
		task->flags &= ~mask;
		if(fd < task->free_fd) {
			task->free_fd = fd;
		}
		return BT_ERR_NONE;
	}

	return BT_ERR_GENERIC;
}

BT_ERROR BT_FreeFileDescriptor(BT_s32 fd) {
	struct bt_task *task = BT_GetProcessTask(NULL);
	return task_free_fd(task, fd);
}
BT_EXPORT_SYMBOL(BT_FreeFileDescriptor);

static BT_ERROR bt_process_cleanup(BT_HANDLE hProcess) {

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_PROCESS,
	.pfnCleanup = bt_process_cleanup,
};

extern struct bt_thread idle_thread;

BT_ERROR bt_process_init() {

	// Create the kernel process handle!
	BT_LIST_INIT_HEAD(&process_handles);
	memset(&kernel_handle, 0, sizeof(struct _BT_OPAQUE_HANDLE));
	strncpy(kernel_handle.task.name, "kernel", BT_CONFIG_MAX_PROCESS_NAME);
#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	kernel_handle.task.map = bt_vm_get_kernel_map();
#endif
	bt_list_add(&kernel_handle.h.item, &process_handles);
	idle_thread.task = &kernel_handle.task;

	BT_LIST_INIT_HEAD(&kernel_handle.task.threads);
	BT_LIST_INIT_HEAD(&kernel_handle.task.handles);

#ifdef BT_CONFIG_PROCESS_CWD
	strcpy(kernel_handle.task.cwd, "/");
#endif
	total_processes = 1;

	BT_AllocFileDescriptor();	// stdin
	BT_AllocFileDescriptor();	// stdout
	BT_AllocFileDescriptor();	// stderr

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(bt_process_init);


/**
 *	This is a low-level process cleanup handler.
 *	It
 *
 **/
void bt_process_thread_cleanup(struct bt_task *task) {

	BT_u32 ulThreads = 0;
	struct bt_list_head *pos;
	bt_list_for_each(pos, &task->threads) {
		ulThreads++;
	}

	if(ulThreads) {
		return;
	}

#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
	bt_vm_destroy(task->map);
#endif

}
