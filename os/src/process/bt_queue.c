/**
 *	Process Queue Implementation.
 *
 **/

#include <bitthunder.h>

BT_DEF_MODULE_NAME			("BT Queue Manager")
BT_DEF_MODULE_DESCRIPTION	("OS Thread/Process Queue")
BT_DEF_MODULE_AUTHOR		("Robert Steinbauer")
BT_DEF_MODULE_EMAIL			("rsteinbauer@riegl.co.at")

struct _BT_OPAQUE_HANDLE {
	BT_HANDLE_HEADER h;
	void *pQueue;
};


static BT_ERROR queue_cleanup(BT_HANDLE hQueue) {
	BT_kQueueDestroy(hQueue->pQueue);
	return BT_ERR_NONE;
}

static const BT_IF_HANDLE oHandleInterface;

BT_HANDLE BT_CreateQueue(BT_u32 ulElements, BT_u32 ulElementWidth, BT_ERROR *pError) {
	BT_HANDLE hQueue = BT_CreateHandle(&oHandleInterface, sizeof(struct _BT_OPAQUE_HANDLE), pError);
	if(!hQueue) {
		return NULL;
	}

	hQueue->pQueue = BT_kQueueCreate(ulElements, ulElementWidth);

	return hQueue;
}

BT_ERROR BT_QueueSend(BT_HANDLE hQueue, const void *pMessage, BT_TICK oTimeoutTicks) {
	return BT_kQueueSend(hQueue->pQueue, pMessage, oTimeoutTicks);
}

BT_ERROR BT_QueueSendFromISR(BT_HANDLE hQueue, const void *pMessage, BT_BOOL *pbHigherPriorityTaskWoken) {
	return BT_kQueueSendFromISR(hQueue->pQueue, pMessage, pbHigherPriorityTaskWoken);
}

BT_ERROR BT_QueueSendToBack(BT_HANDLE hQueue, const void *pMessage, BT_TICK oTimeoutTicks) {
	return BT_kQueueSendToBack(hQueue->pQueue, pMessage, oTimeoutTicks);
}

BT_ERROR BT_QueueSendToFront(BT_HANDLE hQueue, const void *pMessage, BT_TICK oTimeoutTicks) {
	return BT_kQueueSendToFront(hQueue->pQueue, pMessage, oTimeoutTicks);
}


BT_u32 BT_QueueMessagesWaiting(BT_HANDLE hQueue) {
	return BT_kQueueMessagesWaiting(hQueue->pQueue);
}

BT_ERROR BT_QueueReceive(BT_HANDLE hQueue, void *pMessage, BT_TICK oTimeoutTicks) {
	return BT_kQueueReceive(hQueue->pQueue, pMessage, oTimeoutTicks);
}

BT_ERROR BT_QueueReceiveFromISR(BT_HANDLE hQueue, void *pMessage, BT_BOOL *pbHigherPriorityTaskWoken) {
	return BT_kQueueReceiveFromISR(hQueue->pQueue, pMessage, pbHigherPriorityTaskWoken);
}

static const BT_IF_HANDLE oHandleInterface = {
	BT_MODULE_DEF_INFO,
	{NULL},
	BT_HANDLE_T_SYSTEM,
	queue_cleanup,
};

