/**
 *	Process abstraction API for BitThunder.
 *
 **/

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
	const BT_i8		   *szpProcessName;
};


BT_ERROR BT_StartScheduler() {

}

BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {


}

