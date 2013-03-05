/**
 *	BT Tasklets implementation.
 *
 *	BT Tasklets run in a SoftIRQ context, i.e. they run from
 *	a normal thread/process context.
 **/

#ifndef _BT_TASKLETS_H_
#define _BT_TASKLETS_H_

typedef void (*BT_TASKLET_HANDLER)(void *pData);

typedef struct _BT_TASKLET {
	struct _BT_TASKLET *pNext;
	BT_u32 				ulState;
	BT_TASKLET_HANDLER	pfnHandler;
	void 			   *pData;
} BT_TASKLET;


/**
 *
 *
 **/
BT_ERROR BT_TaskletSchedule		(BT_TASKLET *pTasklet);
BT_ERROR BT_TaskletHighSchedule	(BT_TASKLET *pTasklet);

#endif
