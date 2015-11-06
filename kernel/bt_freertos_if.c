#include <bitthunder.h>
#include <FreeRTOS.h>
#include <task.h>
#include <queue.h>
#include <semphr.h>
#include <bt_kernel.h>

BT_ERROR BT_kStartScheduler() {
	vTaskStartScheduler();
	return BT_ERR_GENERIC;
}

void BT_kStopScheduler() {
	vTaskEndScheduler();
}

void *BT_kTaskCreate(BT_FN_TASK_ENTRY pfnStartRoutine, const BT_i8 *szpName, BT_THREAD_CONFIG *pConfig, BT_ERROR *pError) {
	xTaskHandle taskHandle = NULL;

	portBASE_TYPE retval;
	retval = xTaskCreate(pfnStartRoutine,
						 (const signed char * const)  szpName,
						 pConfig->ulStackDepth,
						 pConfig->pParam,
						 pConfig->ulPriority,
						 &taskHandle);

	if(retval != pdTRUE) {
		*pError = BT_ERR_NO_MEMORY;
	} else {
		*pError = BT_ERR_NONE;
	}

	return taskHandle;
}

void BT_kTaskDelete(void *pTaskHandle) {
	vTaskDelete(pTaskHandle);
}

BT_TICK BT_kTickCount() {
	return (BT_TICK) xTaskGetTickCount();
}

void BT_kTaskDelay(BT_TICK ulTicks) {
	vTaskDelay(ulTicks);
}

void BT_kTaskDelayUntil(BT_TICK *pulPreviousWakeTime, BT_TICK ulTimeIncrement) {
	vTaskDelayUntil(pulPreviousWakeTime, ulTimeIncrement);
}

void BT_kTaskYield() {
	if (xTaskGetSchedulerState() != taskSCHEDULER_NOT_STARTED) {
		taskYIELD();
	}
}

BT_BOOL BT_kTaskNotify(struct bt_thread *thread, BT_u32 ulValue, BT_NOTIFY_ACTION eNotifyAction) {
	return (BT_BOOL) xTaskNotify(thread->pKThreadID, ulValue, eNotifyAction);
}

BT_u32 BT_kTaskNotifyTake(BT_BOOL bClearCountOnExit, BT_TICK oTimeoutTicks) {
	return (BT_u32) ulTaskNotifyTake(bClearCountOnExit, oTimeoutTicks);
}

BT_EVGROUP_T BT_kEventGroupCreate(void) {
	return (BT_EVGROUP_T) xEventGroupCreate();
}

void BT_kEventGroupDelete(const BT_EVGROUP_T oEventGroup) {
	vEventGroupDelete(oEventGroup);
}

BT_u32 BT_kEventGroupWaitBits(const BT_EVGROUP_T oEventGroup, const BT_u32 ulBitsToWaitFor, const BT_BOOL bClearCountOnExit, const BT_BOOL bWaitForAllBits, BT_TICK oTimeoutTicks) {
	return xEventGroupWaitBits(oEventGroup, ulBitsToWaitFor, bClearCountOnExit, bWaitForAllBits, oTimeoutTicks);
}

BT_u32 BT_kEventGroupSetBits(const BT_EVGROUP_T oEventGroup, const BT_u32 ulBitsToSet) {
	return xEventGroupSetBits(oEventGroup, ulBitsToSet);
}

void *BT_kGetThreadTag(void *pThreadID) {
	return xTaskGetApplicationTaskTag((xTaskHandle) pThreadID);
}

void BT_kSetThreadTag(void *pThreadID, void *pTagData) {
	vTaskSetApplicationTaskTag((xTaskHandle) pThreadID, pTagData);
}

void *BT_kMutexCreate() {
	xSemaphoreHandle m;
	vSemaphoreCreateBinary(m);
	return m;
}

void *BT_kRecursiveMutexCreate() {
	return xSemaphoreCreateRecursiveMutex();
}

void BT_kMutexDestroy(void *pMutex) {
	vSemaphoreDelete(pMutex);
}

BT_BOOL BT_kMutexPend(void *pMutex, BT_TICK oTimeoutTicks) {

	if(xSemaphoreTake(pMutex, oTimeoutTicks) == pdPASS) {
		return BT_TRUE;
	}

	return BT_FALSE;
}

BT_BOOL  BT_kMutexRelease(void *pMutex) {
	if(xSemaphoreGive(pMutex) == pdTRUE) {
		return BT_TRUE;
	}
	return BT_FALSE;
}

BT_BOOL BT_kMutexPendRecursive(void *pMutex, BT_TICK oTimeoutTicks) {

	if(!oTimeoutTicks) {
		oTimeoutTicks = portMAX_DELAY;
	}

	if(xSemaphoreTakeRecursive(pMutex, oTimeoutTicks) == pdPASS) {
		return BT_TRUE;
	}
	return BT_FALSE;
}

BT_BOOL  BT_kMutexReleaseRecursive(void *pMutex) {
	if(xSemaphoreGiveRecursive(pMutex) == pdTRUE) {
		return BT_TRUE;
	}
	return BT_FALSE;
}

BT_BOOL BT_kMutexReleaseFromISR(void *pMutex, BT_BOOL *pbHigherPriorityTaskWoken) {
	portBASE_TYPE val;

	BT_BOOL bReturn =  xSemaphoreGiveFromISR(pMutex, &val);
	if(pbHigherPriorityTaskWoken) {
		*pbHigherPriorityTaskWoken = (BT_BOOL) val;
	}

	return bReturn;
}

void *BT_kQueueCreate(BT_u32 ulElements, BT_u32 ulElementWidth) {
	return xQueueCreate(ulElements, ulElementWidth);
}

void BT_kQueueDestroy(void *pQueue) {
	vQueueDelete(pQueue);
}

BT_ERROR BT_kQueueSend(void *pQueue, const void* pMessage, BT_TICK oTimeoutTicks) {
	return xQueueSend(pQueue, pMessage, oTimeoutTicks );
}

BT_ERROR BT_kQueueSendFromISR(void *pQueue, const void* pMessage, BT_BOOL *pbHigherPriorityTaskWoken) {
	portBASE_TYPE val;

	BT_BOOL bReturn = xQueueSendFromISR(pQueue, pMessage, &val );

	if(pbHigherPriorityTaskWoken) {
		*pbHigherPriorityTaskWoken = (BT_BOOL) val;
	}

	return bReturn;
}

BT_u32 BT_kQueueMessagesWaiting(void *pQueue) {
	return uxQueueMessagesWaiting(pQueue);
}

BT_ERROR BT_kQueueSendToFront(void *pQueue, const void* pMessage, BT_TICK oTimeoutTicks) {
	return xQueueSendToFront(pQueue, pMessage, oTimeoutTicks );
}

BT_ERROR BT_kQueueSendToBack(void *pQueue, const void* pMessage, BT_TICK oTimeoutTicks) {
	return xQueueSendToBack(pQueue, pMessage, oTimeoutTicks );
}

BT_ERROR BT_kQueueReceive(void *pQueue, void* pMessage, BT_TICK oTimeoutTicks) {
	return xQueueReceive(pQueue, pMessage, oTimeoutTicks);
}
BT_ERROR BT_kQueueReceiveFromISR(void *pQueue, void* pMessage, BT_BOOL *pbHigherPriorityTaskWoken) {
	portBASE_TYPE val;

	BT_BOOL bReturn = xQueueReceiveFromISR(pQueue, pMessage, &val);

	if(pbHigherPriorityTaskWoken) {
		*pbHigherPriorityTaskWoken = (BT_BOOL) val;
	}

	return bReturn;

}



void BT_kEnterCritical() {
	taskENTER_CRITICAL();
}

void BT_kExitCritical() {
	taskEXIT_CRITICAL();
}
