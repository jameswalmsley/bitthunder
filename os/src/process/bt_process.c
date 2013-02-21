/**
 *	Process abstraction API for BitThunder.
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("BitThunder Process Model")
BT_DEF_MODULE_DESCRIPTION	("OS Process abstraction for the BitThunder Kernel")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER 	h;

	BT_HANDLE			hThreads;		///< HANDLE collection of associated threads.
	BT_u32				ulPID;			///< ProcessID of this process.
	BT_BOOL				bIsStarted;		///< Flag process started, prevent dual starting!
	BT_BOOL				bAutoRestart;	///< Flag allow auto-restarting of process.
	const BT_i8		   	szProcessName[BT_CONFIG_MAX_PROCESS_NAME];
};

static BT_LIST oProcessHandles;

//static BT_HANDLE g_hKernelProcess 	= NULL;
//static BT_HANDLE g_hProcessHandles 	= NULL;

static const BT_IF_HANDLE oHandleInterface;

BT_ERROR BT_StartScheduler() {
	return BT_kStartScheduler();
}

BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {
	BT_HANDLE hProcess = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hProcess) {
		return NULL;
	}



	BT_ListAddItem(&oProcessHandles, &hProcess->h.oItem);

	return hProcess;
}


static BT_ERROR bt_process_cleanup(BT_HANDLE hProcess) {

	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_NAME,
	BT_MODULE_DESCRIPTION,
	BT_MODULE_AUTHOR,
	BT_MODULE_EMAIL,
	{NULL},
	BT_HANDLE_T_PROCESS,
	bt_process_cleanup,
};

static BT_ERROR bt_process_manager_init() {

	// Create the kernel process handle!

	return BT_ListInit(&oProcessHandles);
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_process_manager_init,
};
