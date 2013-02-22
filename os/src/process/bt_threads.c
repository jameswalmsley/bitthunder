/**
 *	Thread Control API.
 *
 *
 **/

#include <bitthunder.h>
#include <string.h>

BT_DEF_MODULE_NAME			("BitThunder Threading Model")
BT_DEF_MODULE_DESCRIPTION	("Process threads for the BitThunder Kernel")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;
	BT_HANDLE			hProcess;
	BT_LIST				oHandles;
	BT_THREAD_CONFIG 	oConfig;
	BT_FN_THREAD_ENTRY 	pfnStartRoutine;
	void 			   *pThreadParam;
	void 			   *pKThreadID;
	BT_ERROR			lastThreadError;
};


static void threadStartup(void *pParam) {

	BT_HANDLE hThread = (BT_HANDLE) pParam;

	// Set the task tag!

	if(hThread->pfnStartRoutine) {
		BT_kSetThreadTag(hThread->pKThreadID, hThread);
		hThread->lastThreadError = hThread->pfnStartRoutine(hThread, hThread->pThreadParam);
	}

	BT_kTaskDelete(hThread->pKThreadID);
}

static const BT_IF_HANDLE oHandleInterface;

BT_HANDLE BT_CreateProcessThread(BT_HANDLE hProcess, BT_FN_THREAD_ENTRY pfnStartRoutine, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {

	BT_HANDLE hThread = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hThread) {
		return NULL;
	}

	hThread->hProcess = hProcess;

	memcpy(&hThread->oConfig, pConfig, sizeof(BT_THREAD_CONFIG));

	hThread->pThreadParam = pConfig->pParam;
	hThread->oConfig.pParam = hThread;

	hThread->pKThreadID = BT_kTaskCreate(threadStartup, NULL, &hThread->oConfig, pError);
	if(!hThread->pKThreadID) {
		BT_kFree(hThread);
		return NULL;
	}

	BT_LIST *pThreadList = BT_GetProcessThreadList(hProcess);
	if(pThreadList) {
		BT_ListAddItem(pThreadList, &hThread->h.oItem);
	}

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
	BT_HANDLE hThread = (BT_HANDLE) BT_kGetThreadTag(NULL);
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
