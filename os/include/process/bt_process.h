#ifndef _BT_PROCESS_H_
#define _BT_PROCESS_H_

BT_HANDLE BT_CreateProcess(BT_FN_THREAD_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);

BT_HANDLE BT_GetProcessHandle(void);



BT_LIST *BT_GetProcessThreadList(BT_HANDLE hProcess);








#endif
