/**
 *	BT Tasklets Implementation.
 *
 *
 **/

#include <bitthunder.h>
#include <interrupts/bt_tasklets.h>
#include <interrupts/bt_softirq.h>

static BT_TASKLET *g_Tasklets = NULL;
static BT_TASKLET *g_HighTasklets = NULL;

BT_ERROR BT_TaskletSchedule(BT_TASKLET *pTasklet) {

	BT_kEnterCritical();
	{
		if(!g_Tasklets) {
			g_Tasklets = pTasklet;
		} else {
			pTasklet->pNext = g_Tasklets;
			g_Tasklets = pTasklet;
		}
	}
	BT_kExitCritical();

	BT_RaiseSoftIRQ(BT_SOFTIRQ_TASKLET);

	return BT_ERR_NONE;
}


BT_ERROR BT_TaskletHighSchedule(BT_TASKLET *pTasklet) {

	BT_kEnterCritical();
	{
		if(!g_HighTasklets) {
			g_HighTasklets = pTasklet;
		} else {
			pTasklet->pNext = g_HighTasklets;
			g_HighTasklets = pTasklet;
		}
	}
	BT_kExitCritical();

	BT_RaiseSoftIRQ(BT_SOFTIRQ_HI);

	return BT_ERR_NONE;
}

