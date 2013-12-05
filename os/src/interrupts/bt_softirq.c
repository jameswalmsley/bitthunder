/**
 *	BitThunder SoftIRQ implementation.
 *
 *	The API is modelled after the linux softirq implementation, to allow
 *	kernel developer skills to be relatively easily transferred.
 *
 **/

#include <bitthunder.h>
#include <interrupts/bt_softirq.h>

BT_DEF_MODULE_NAME	("SoftIRQ")

static BT_SOFTIRQ 	g_SoftIRQ[BT_CONFIG_INTERRUPTS_SOFTIRQ_MAX];
static BT_u32 		g_ulPending;
static BT_HANDLE	g_hMutex;

BT_ERROR BT_OpenSoftIRQ(BT_u32 ulSoftIRQ, BT_SOFTIRQ_HANDLER pfnHandler, void *pData) {
	g_SoftIRQ[ulSoftIRQ].pfnHandler = pfnHandler;
	g_SoftIRQ[ulSoftIRQ].pData		= pData;
	return BT_ERR_NONE;
}

BT_ERROR BT_RaiseSoftIRQ(BT_u32 ulSoftIRQ) {

	BT_ERROR Error;

	// Enter Critical Section
	{
		Error = BT_RaiseSoftIRQFromISR(ulSoftIRQ);
	}
	// Exit Critical Section
	return Error;
}

BT_ERROR BT_RaiseSoftIRQFromISR(BT_u32 ulSoftIRQ) {
	if(ulSoftIRQ < BT_CONFIG_SOFTIRQ_MAX) {
		g_ulPending |= (1 << ulSoftIRQ);
		BT_ReleaseMutexFromISR(g_hMutex, NULL);
		return BT_ERR_NONE;
	}

	return BT_ERR_GENERIC;
}


static BT_ERROR softirq_dispatcher(BT_HANDLE hThread, void *pParam) {

	BT_u32 ulPending;
	BT_SOFTIRQ *p;

	BT_ThreadSleep(100);
	BT_kPrint("Started the softIRQ dispatcher");

	while(1) {
		BT_PendMutex(g_hMutex, 0);

		ulPending = g_ulPending;
		if(ulPending) {
			g_ulPending = 0;
			p = g_SoftIRQ;
			do {
				if(ulPending & 1) {
					p->pfnHandler(p->pData);
				}

				p++;

				ulPending >>= 1;
			} while(ulPending);
		}
	}

	return BT_ERR_NONE;
}

static BT_ERROR bt_softirq_init() {
	BT_ERROR Error;
	BT_THREAD_CONFIG oConfig;

	oConfig.ulStackDepth 	= 256;
	oConfig.ulPriority 		= BT_CONFIG_INTERRUPTS_SOFTIRQ_PRIORITY;

	g_hMutex = BT_CreateMutex(&Error);
	if(!g_hMutex) {
		return BT_ERR_GENERIC;
	}

	BT_PendMutex(g_hMutex, 0);

	BT_CreateThread(softirq_dispatcher, &oConfig, &Error);

	return BT_ERR_NONE;
}

BT_MODULE_INIT_0_DEF oModuleEntry = {
	BT_MODULE_NAME,
	bt_softirq_init,
};
