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
	taskYIELD();
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

void BT_kMutexDestroy(void *pMutex) {
	vSemaphoreDelete(pMutex);
}

BT_BOOL BT_kMutexPend(void *pMutex, BT_TICK oTimeoutTicks) {
	if(xSemaphoreTake(pMutex, oTimeoutTicks) == pdTRUE) {
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

BT_BOOL BT_kMutexReleaseFromISR(void *pMutex, BT_BOOL *pbHigherPriorityTaskWoken) {
	portBASE_TYPE val;

	BT_BOOL bReturn =  xSemaphoreGiveFromISR(pMutex, &val);
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
