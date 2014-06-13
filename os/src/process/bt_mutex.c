/**
 *	Process Mutex Implementation.
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("BT Mutex Manager")
BT_DEF_MODULE_DESCRIPTION	("OS Thread/Process Mutex")
BT_DEF_MODULE_AUTHOR		("James Walmsley")
BT_DEF_MODULE_EMAIL			("james@fullfat-fs.co.uk")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	void *pMutex;
};


static BT_ERROR mutex_cleanup(BT_HANDLE hMutex) {
	BT_kMutexDestroy(hMutex->pMutex);
	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface;

BT_HANDLE BT_CreateMutex(BT_ERROR *pError) {
	BT_HANDLE hMutex = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hMutex) {
		return NULL;
	}

	hMutex->pMutex = BT_kMutexCreate();

	return hMutex;
}
BT_EXPORT_SYMBOL(BT_CreateMutex);

BT_ERROR BT_PendMutex(BT_HANDLE hMutex, BT_TICK oTimeoutTicks) {
	return BT_kMutexPend(hMutex->pMutex, oTimeoutTicks);
}
BT_EXPORT_SYMBOL(BT_PendMutex);

BT_ERROR BT_ReleaseMutex(BT_HANDLE hMutex) {
	return BT_kMutexRelease(hMutex->pMutex);
}
BT_EXPORT_SYMBOL(BT_ReleaseMutex);

BT_ERROR BT_PendMutexRecursive(BT_HANDLE hMutex, BT_TICK oTimeoutTicks) {
	return BT_kMutexPendRecursive(hMutex->pMutex, oTimeoutTicks);
}
BT_EXPORT_SYMBOL(BT_PendMutexRecursive);

BT_ERROR BT_ReleaseMutexRecursive(BT_HANDLE hMutex) {
	return BT_kMutexReleaseRecursive(hMutex->pMutex);
}
BT_EXPORT_SYMBOL(BT_ReleaseMutexRecursive);

BT_ERROR BT_ReleaseMutexFromISR(BT_HANDLE hMutex, BT_BOOL *pbHigherPriorityThreadWoken) {
	return BT_kMutexReleaseFromISR(hMutex->pMutex, pbHigherPriorityThreadWoken);
}
BT_EXPORT_SYMBOL(BT_ReleaseMutexFromISR);

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	.eType = BT_HANDLE_T_SYSTEM,
	.pfnCleanup = mutex_cleanup,
};
