/**
 *	BT Tasklets Implementation.
 *
 *
 **/

#include <bitthunder.h>
#include <interrupts/bt_tasklets.h>
#include <interrupts/bt_softirq.h>

BT_DEF_MODULE_NAME	("Tasklets")

static BT_TASKLET *g_Tasklets = NULL;
static BT_TASKLET *g_HighTasklets = NULL;

BT_ERROR BT_TaskletSchedule(BT_TASKLET *pTasklet) {

	BT_kEnterCritical();
	{
		if(pTasklet->eState == BT_TASKLET_IDLE) {
			pTasklet->eState = BT_TASKLET_SCHEDULED;

			if(!g_Tasklets) {
				g_Tasklets = pTasklet;
			} else {
				pTasklet->pNext = g_Tasklets;
				g_Tasklets = pTasklet;
			}
		}
	}
	BT_kExitCritical();

	BT_RaiseSoftIRQ(BT_SOFTIRQ_TASKLET);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_TaskletSchedule);

BT_ERROR BT_TaskletHighSchedule(BT_TASKLET *pTasklet) {

	BT_kEnterCritical();
	{
		if(pTasklet->eState == BT_TASKLET_IDLE) {
			pTasklet->eState = BT_TASKLET_SCHEDULED;

			if(!g_HighTasklets) {
				g_HighTasklets = pTasklet;
			} else {
				pTasklet->pNext = g_HighTasklets;
				g_HighTasklets = pTasklet;
			}
		}
	}
	BT_kExitCritical();

	BT_RaiseSoftIRQ(BT_SOFTIRQ_HI);

	return BT_ERR_NONE;
}
BT_EXPORT_SYMBOL(BT_TaskletHighSchedule);

static void tasklet_action(struct _BT_SOFTIRQ *pSoftIRQ) {

	BT_TASKLET *pTasklets;

	BT_kEnterCritical();
	{
		pTasklets = g_Tasklets;
		g_Tasklets = NULL;
	}
	BT_kExitCritical();

	while(pTasklets) {
		BT_TASKLET *t = pTasklets;
		pTasklets = pTasklets->pNext;

		t->eState = BT_TASKLET_RUNNING;
		t->pfnHandler(t->pData);
		t->pNext = NULL;
		t->eState = BT_TASKLET_IDLE;
	}
}

static void tasklet_action_hi(struct _BT_SOFTIRQ *pSoftIRQ) {
	BT_TASKLET *pTasklets;

	BT_kEnterCritical();
	{
		pTasklets = g_HighTasklets;
		g_HighTasklets = NULL;
	}
	BT_kExitCritical();

	while(pTasklets) {
		BT_TASKLET *t = pTasklets;
		pTasklets = pTasklets->pNext;

		t->eState = BT_TASKLET_RUNNING;
		t->pfnHandler(t->pData);
		t->pNext = NULL;
		t->eState = BT_TASKLET_IDLE;
	}
}

static BT_ERROR bt_tasklets_init() {

	BT_OpenSoftIRQ(BT_SOFTIRQ_HI, 		tasklet_action_hi, NULL);
	BT_OpenSoftIRQ(BT_SOFTIRQ_TASKLET, 	tasklet_action, NULL);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_tasklets_init,
};
