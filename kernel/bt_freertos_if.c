#include <bitthunder.h>
#include <FreeRTOS.h>
#include <task.h>
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

