/**
 *	Process abstraction API for BitThunder.
 *
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
	BT_LIST				oThreads;
	BT_u16				usPID;			///< ProcessID of this process.
	BT_THREAD_CONFIG	oConfig;
	struct bt_vm_map   *mmap;			///< Processes' private memory map.

	BT_BOOL				bIsStarted;		///< Flag process started, prevent dual starting!
	BT_BOOL				bAutoRestart;	///< Flag allow auto-restarting of process.
	BT_i8		   	szProcessName[BT_CONFIG_MAX_PROCESS_NAME+1];
};

static BT_LIST oProcessHandles;
static BT_u16 usLastPID = 0;

//static BT_HANDLE g_hKernelProcess 	= NULL;
//static BT_HANDLE g_hProcessHandles 	= NULL;

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
	strncpy(hProcess->szProcessName, szpName, BT_CONFIG_MAX_PROCESS_NAME);

	BT_ListInit(&hProcess->oThreads);

	hProcess->mmap = bt_vm_create();

	BT_CreateProcessThread(hProcess, pfnStartRoutine, &hProcess->oConfig, pError);

	BT_ListAddItem(&oProcessHandles, &hProcess->h.oItem);

	return hProcess;
}

BT_HANDLE BT_GetProcessHandle(void) {
	BT_HANDLE hThread = BT_GetThreadHandle();
	if(hThread) {
		return BT_GetThreadProcessHandle(hThread);
	}

	return NULL;
}

BT_LIST *BT_GetProcessThreadList(BT_HANDLE hProcess) {
	return &hProcess->oThreads;
}

static BT_ERROR bt_process_cleanup(BT_HANDLE hProcess) {

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_PROCESS,
	.pfnCleanup = bt_process_cleanup,
};

static BT_ERROR bt_process_manager_init() {

	// Create the kernel process handle!

	return BT_ListInit(&oProcessHandles);
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_process_manager_init,
};
