#ifndef _BT_MUTEX_H_
#define _BT_MUTEX_H_

BT_HANDLE 	BT_CreateMutex			(BT_ERROR *pError);
BT_ERROR 	BT_PendMutex			(BT_HANDLE hMutex, BT_TICK oTimeoutTicks);
BT_ERROR 	BT_ReleaseMutex			(BT_HANDLE hMutex);
BT_ERROR 	BT_PendMutexRecursive	(BT_HANDLE hMutex, BT_TICK oTimeoutTicks);
BT_ERROR 	BT_ReleaseMutexRecursive(BT_HANDLE hMutex);
BT_ERROR 	BT_ReleaseMutexFromISR	(BT_HANDLE hMutex, BT_BOOL *pbHigherPriorityThreadWoken);



#endif
