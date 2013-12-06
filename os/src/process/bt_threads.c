/**
 *	Thread Control API.
 *
 *
 **/

#include <bitthunder.h>
#include <string.h>

BT_DEF_MODULE_NAME			("Threading Model")
BT_DEF_MODULE_DESCRIPTION	("Process threads for the BitThunder Kernel")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	struct bt_thread	thread;
	BT_HANDLE			hProcess;
	BT_THREAD_CONFIG 	oConfig;
	BT_FN_THREAD_ENTRY 	pfnStartRoutine;
	void 			   *pThreadParam;
	void 			   *pKThreadID;
	BT_ERROR			lastThreadError;
};

struct bt_thread idle_thread;
struct bt_thread *curthread = &idle_thread;
BT_u64 ullTaskSwitchedInTime = 0;

static void threadStartup(void *pParam) {

	BT_HANDLE hThread = (BT_HANDLE) pParam;

	if(hThread->pfnStartRoutine) {
		BT_kSetThreadTag(hThread->pKThreadID, &hThread->thread);	// Tag the task structure.
		curthread = &hThread->thread;
#ifdef BT_CONFIG_USE_VIRTUAL_ADDRESSING
		if(curtask && curtask->map) {
			bt_mmu_switch(curtask->map->pgd);
		}
#endif
		hThread->lastThreadError = hThread->pfnStartRoutine(hThread, hThread->pThreadParam);
	}

	void *pKThreadID = hThread->pKThreadID;
	if(!(hThread->oConfig.ulFlags & BT_THREAD_FLAGS_NO_CLEANUP)) {
		BT_CloseHandle(hThread);
	}

	BT_kTaskDelete(pKThreadID);
}

static const BT_IF_HANDLE oHandleInterface;

BT_HANDLE BT_CreateProcessThread(BT_HANDLE hProcess, BT_FN_THREAD_ENTRY pfnStartRoutine, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {

	BT_ERROR Error;
	BT_HANDLE hThread = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hThread) {
		return NULL;
	}

	hThread->hProcess = hProcess;
	hThread->thread.task = BT_GetProcessTask(hProcess);

	memcpy(&hThread->oConfig, pConfig, sizeof(BT_THREAD_CONFIG));

	hThread->pThreadParam = pConfig->pParam;
	hThread->oConfig.pParam = hThread;
	hThread->pfnStartRoutine = pfnStartRoutine;

	hThread->pKThreadID = BT_kTaskCreate(threadStartup, NULL, &hThread->oConfig, &Error);
	if(!hThread->pKThreadID) {
		BT_DestroyHandle(hThread);
		return NULL;
	}

	struct bt_task *task = BT_GetProcessTask(hProcess);
	bt_list_add(&hThread->h.list, &task->threads);

	return hThread;
}

BT_HANDLE BT_CreateThread(BT_FN_THREAD_ENTRY pfnStartRoutine, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {

	BT_HANDLE hProcess = BT_GetProcessHandle();
	if(!hProcess) {
		// Error -- bad context to create a thread!
		return NULL;
	}

	return BT_CreateProcessThread(hProcess, pfnStartRoutine, pConfig, pError);
}

BT_HANDLE BT_GetThreadHandle() {
	BT_HANDLE hThread = bt_container_of(curthread, struct _BT_OPAQUE_HANDLE, thread);
	return hThread;
}

BT_HANDLE BT_GetThreadProcessHandle(BT_HANDLE hThread) {
	return hThread->hProcess;
}

BT_ERROR BT_ThreadSleepUntil(BT_TICK *pulPreviousWakeTime, BT_u32 ulTimeMs) {
	BT_kTaskDelayUntil(pulPreviousWakeTime, ulTimeMs);
	return BT_ERR_NONE;
}

BT_ERROR BT_ThreadSleep(BT_u32 ulTimeMs) {
	BT_kTaskDelay(ulTimeMs);
	return BT_ERR_NONE;
}

BT_ERROR BT_ThreadYield() {
	BT_kTaskYield();
	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
};
