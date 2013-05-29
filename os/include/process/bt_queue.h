#ifndef _BT_QUEUE_H_
#define _BT_QUEUE_H_

BT_HANDLE 	BT_CreateQueue			(BT_u32 ulElements, BT_u32 ulElementWidth, BT_ERROR *pError);
BT_ERROR 	BT_QueueSend			(BT_HANDLE hQueue, const void *pMessage, BT_TICK oTimeoutTicks);
BT_ERROR 	BT_QueueSendFromISR		(BT_HANDLE hQueue, const void *pMessage, BT_BOOL *pbHigherPriorityTaskWoken);
BT_ERROR 	BT_QueueSendToBack		(BT_HANDLE hQueue, const void *pMessage, BT_TICK oTimeoutTicks);
BT_ERROR 	BT_QueueSendToFront		(BT_HANDLE hQueue, const void *pMessage, BT_TICK oTimeoutTicks);
BT_ERROR 	BT_QueueReceive			(BT_HANDLE hQueue, void *pMessage, BT_TICK oTimeoutTicks);
BT_ERROR 	BT_QueueReceiveFromISR	(BT_HANDLE hQueue, void *pMessage, BT_BOOL *pbHigherPriorityTaskWoken);
BT_u32		BT_QueueMessagesWaiting	(BT_HANDLE hQueue);



#endif
