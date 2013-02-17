#ifndef _BT_KERNEL_H_
#define _BT_KERNEL_H_

#include <bt_types.h>
#include <process/bt_threads.h>

typedef void (*BT_FN_TASK_ENTRY)(void *pParam);

BT_ERROR 	BT_kStartScheduler	();
void 	   *BT_kTaskCreate		(BT_FN_TASK_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError);
void		BT_kTaskDelete 		(void *pTaskHandle);

BT_TICK		BT_kTickCount		();
void		BT_kTaskDelay		(BT_TICK ulTicks);
void		BT_kTaskDelayUntil 	(BT_TICK *pulPreviousWakeTime, BT_TICK ulTimeIncrement);
void 		BT_kTaskYield();


#endif
